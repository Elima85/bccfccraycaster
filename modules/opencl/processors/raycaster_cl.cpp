/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Created between 2005 and 2012 by The Voreen Team                   *
 * as listed in CREDITS.TXT <http://www.voreen.org>                   *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include <cmath>
#include <ctime>
#include <queue>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "modules/opencl/processors/raycaster_cl.h"
#include "voreen/core/voreenapplication.h"
#include "voreen/core/datastructures/volume/gradient.h"
#include "voreen/core/datastructures/transfunc/transfunc.h"
#include "voreen/core/properties/buttonproperty.h"

#include "tgt/glmath.h"
#include "tgt/gpucapabilities.h"

#include "modules/opencl/openclmodule.h"

using tgt::vec3;
using tgt::mat4;

namespace voreen {

using namespace cl;

const std::string RaycasterCL::loggerCat_("voreen.RaycasterCL");

RaycasterCL::RaycasterCL()
    : VolumeRenderer(),
      volumePort_(Port::INPORT, "volumehandle.volumehandle", false, VolumeRenderer::INVALID_PROGRAM),
      entryPort_(Port::INPORT, "image.entrypoints"),
      exitPort_(Port::INPORT, "image.exitpoints"),
      // TODO: depth formats like GL_DEPTH_COMPONENT24 are not supported by OpenCL, investigate. Depth values are not calculated for now. FL
      outport_(Port::OUTPORT, "image.output", true, Processor::INVALID_PROGRAM),
      samplingRate_("samplingRate", "Sampling Rate", 4.f, 0.01f, 20.0f),
      transferFunc_("transferFunction", "Transfer Function"),
      openclProp_("openclprog", "OpenCL Program", VoreenApplication::app()->getBasePath() + "/modules/opencl/cl/raycaster.cl"),
      opencl_(0),
      context_(0),
      queue_(0),
      volumeTex_(0),
      tfData_(0),
      entryTexCol_(0),
      exitTexCol_(0),
      outCol_(0)
{
    addPort(volumePort_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);

    addProperty(samplingRate_);
    addProperty(transferFunc_);
    addProperty(openclProp_);
}

RaycasterCL::~RaycasterCL() {
}

Processor* RaycasterCL::create() const {
    return new RaycasterCL();
}

void RaycasterCL::initialize() throw (tgt::Exception) {
    OpenCLModule::getInstance()->initCL();

    RenderProcessor::initialize();

    if (!OpenCLModule::getInstance()->getCLContext())
        throw VoreenException("No OpenCL context created");

    opencl_ = OpenCLModule::getInstance()->getOpenCL();
    context_ = OpenCLModule::getInstance()->getCLContext();
    queue_ = OpenCLModule::getInstance()->getCLCommandQueue();

    initialized_ = true;
}

void RaycasterCL::deinitialize() throw (tgt::Exception) {
    delete volumeTex_;
    volumeTex_ = 0;
    delete tfData_;
    tfData_ = 0;
    delete entryTexCol_;
    delete exitTexCol_;
    delete outCol_;
    //delete entryTexDepth_;
    //delete exitTexDepth_;
    //delete outDepth_;
    entryTexCol_ = 0;
    exitTexCol_  = 0;
    outCol_      = 0;
}

void RaycasterCL::beforeProcess() {
    VolumeRenderer::beforeProcess();

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM) {
        std::ostringstream clDefines;
        clDefines << " -cl-fast-relaxed-math -cl-mad-enable";

        // set include path for modules
        // TODO: Include path in OpenCL with / does not work on windows. It should be replaced by two backslashes
        // Use shortPath in order to get a path without spaces (nvidia compiler cannot handle paths with spaces)  
        clDefines << " -I " << tgt::FileSystem::shortPath(VoreenApplication::app()->getBasePath()) + "/modules/opencl/cl/";

        // Support for 12 bit data.
        RealWorldMapping rwm = volumePort_.getData()->getRealWorldMapping();
        if( volumePort_.getData()->getBitsStored() == 12 ) {
            clDefines << " -D VOLUME_DATA_SCALE=" << rwm.getScale()*16.f << " ";
        }
        else {
            clDefines << " -D VOLUME_DATA_SCALE=" << rwm.getScale() << " ";
        }
        clDefines << " -D VOLUME_DATA_OFFSET=" << rwm.getOffset() << " ";

        openclProp_.setDefines(clDefines.str());
        openclProp_.rebuild();

        delete volumeTex_;
        volumeTex_ = new ImageObject3D(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, volumePort_.getData()->getRepresentation<Volume>());
    }
    LGL_ERROR;

    if (getInvalidationLevel() >= Processor::INVALID_RESULT) {
        tgt::Texture* tfTex = transferFunc_.get()->getTexture();
        delete tfData_;
        ImageFormat imgf(CL_RGBA, CL_UNORM_INT8);
        tfData_ = new ImageObject2D(context_, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, imgf, tfTex->getWidth(), 1, tfTex->getWidth() * tfTex->getBpp(), transferFunc_.get()->getPixelData());
        
        delete entryTexCol_;
        delete exitTexCol_;
        delete outCol_;
        //delete entryTexDepth_;
        //delete exitTexDepth_;
        //delete outDepth_;
        entryTexCol_ = new SharedTexture(context_, CL_MEM_READ_ONLY, entryPort_.getColorTexture());
        exitTexCol_  = new SharedTexture(context_, CL_MEM_READ_ONLY, exitPort_.getColorTexture());
        outCol_      = new SharedTexture(context_, CL_MEM_WRITE_ONLY, outport_.getColorTexture());

        //entryTexDepth_ = new SharedTexture(context_, CL_MEM_READ_ONLY, entryPort_.getDepthTexture());
        //exitTexDepth_  = new SharedTexture(context_, CL_MEM_READ_ONLY, exitPort_.getDepthTexture());
        //outDepth_      = new SharedTexture(context_, CL_MEM_READ_ONLY, outport_.getDepthTexture());
    }

    transferFunc_.setVolumeHandle(volumePort_.getData());
}

void RaycasterCL::process() {

    LGL_ERROR;

    Kernel* kernel = openclProp_.getProgram()->getKernel("raycast");

    if (kernel) {

        // TODO: Does this really have to be done every time to see the image on the canvas? FL
        outport_.activateTarget();
        outport_.deactivateTarget();

        LGL_ERROR;
        glFinish();

        tgt::svec3 dims = tgt::svec3(entryPort_.getColorTexture()->getDimensions());

        float samplingStepSize = 1.f / (tgt::min(volumePort_.getData()->getRepresentation<Volume>()->getDimensions()) * samplingRate_.get());


        kernel->setArg(0, volumeTex_);
        kernel->setArg(1, tfData_);
        kernel->setArg(2, entryTexCol_);
        kernel->setArg(3, exitTexCol_);
        kernel->setArg(4, outCol_);
        //kernel->setArg(5, entryTexDepth_);
        //kernel->setArg(6, exitTexDepth_);
        //kernel->setArg(7, outDepth_);
        kernel->setArg(5, samplingStepSize);

        queue_->enqueueAcquireGLObject(entryTexCol_);
        queue_->enqueueAcquireGLObject(exitTexCol_);
        queue_->enqueueAcquireGLObject(outCol_);

        //queue_->enqueueAcquireGLObject(&entryTexDepth_);
        //queue_->enqueueAcquireGLObject(&exitTexDepth_);
        //queue_->enqueueAcquireGLObject(&outDepth_);

        queue_->enqueue(kernel, dims.xy());

        queue_->enqueueReleaseGLObject(entryTexCol_);
        queue_->enqueueReleaseGLObject(exitTexCol_);
        queue_->enqueueReleaseGLObject(outCol_);

        //queue_->enqueueReleaseGLObject(&entryTexDepth_);
        //queue_->enqueueReleaseGLObject(&exitTexDepth_);
        //queue_->enqueueReleaseGLObject(&outDepth_);
        queue_->finish();
        //delete entryTexCol_;
        //delete exitTexCol_;
        //delete outCol_;
        ////delete entryTexDepth_;
        ////delete exitTexDepth_;
        ////delete outDepth_;
        //entryTexCol_ = 0;
        //exitTexCol_  = 0;
        //outCol_      = 0;
    }
    else {
        LERROR("Kernel 'raycast' not found");
        return;
    }
}

bool RaycasterCL::isReady() const {
    //check if all inports are connected
    if(!volumePort_.isReady() || !entryPort_.isReady() || !exitPort_.isReady())
        return false;

    if(!outport_.isReady())
        return false;

    return true;
}

void RaycasterCL::portResized(RenderPort* p, tgt::ivec2 newsize) {
    // it is important to delete the CL memory objects before deleting the associated OpenGL textures from the fbo
    delete entryTexCol_; entryTexCol_ = 0;
    delete exitTexCol_;  exitTexCol_  = 0;
    delete outCol_;      outCol_      = 0;
    RenderProcessor::portResized(p, newsize);
}

} // namespace voreen
