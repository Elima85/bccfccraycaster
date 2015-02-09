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

#include "voreen/core/processors/volumeraycaster.h"
#include "voreen/core/utils/voreenpainter.h"

#include "tgt/vector.h"

using tgt::mat4;

namespace voreen {

const std::string VolumeRaycaster::loggerCat_("voreen.VolumeRaycaster");

/*
    constructor and destructor
*/

VolumeRaycaster::VolumeRaycaster()
    : VolumeRenderer()
    //, raycastPrg_(0)
    , samplingRate_("samplingRate", "Sampling Rate", 2.f, 0.01f, 20.f)
    , isoValue_("isoValue", "Iso Value", 0.5f, 0.0f, 1.0f)
    , maskingMode_("masking", "Masking", Processor::INVALID_PROGRAM)
    , gradientMode_("gradient", "Gradient Calculation", Processor::INVALID_PROGRAM)
    , classificationMode_("classification", "Classification", Processor::INVALID_PROGRAM)
    , shadeMode_("shading", "Shading", Processor::INVALID_PROGRAM)
    , compositingMode_("compositing", "Compositing", Processor::INVALID_PROGRAM)
    , interactionCoarseness_("interactionCoarseness","Interaction Coarseness", 4, 1, 16, Processor::VALID)
    , interactionQuality_("interactionQuality","Interaction Quality", 1.0f, 0.01f, 1.0f, Processor::VALID)
    , useInterpolationCoarseness_("interpolation.coarseness","Use Interpolation Coarseness", false, Processor::INVALID_PROGRAM)
    , size_(128, 128)
    , switchToInteractionMode_(false)
{
    initProperties();
}

/*
    further methods
*/
std::string VolumeRaycaster::generateHeader(const tgt::GpuCapabilities::GlVersion* version) {
    std::string headerSource = VolumeRenderer::generateHeader(version);

    // configure gradient calculation
    headerSource += "#define RC_CALC_GRADIENTS(voxel, samplePos, volumeStruct, t, rayDirection, entryPoints, entryParameters) ";
    if (gradientMode_.isSelected("none"))
        headerSource += "(voxel.xyz-vec3(0.5))*2.0;\n";
    else if (gradientMode_.isSelected("forward-differences"))
        headerSource += "calcGradientAFD(volumeStruct, samplePos, t, rayDirection, entryPoints, entryParameters);\n";
    else if (gradientMode_.isSelected("central-differences"))
        headerSource += "calcGradientA(volumeStruct, samplePos, t, rayDirection, entryPoints, entryParameters);\n";
    else if (gradientMode_.isSelected("filtered"))
        headerSource += "calcGradientFiltered(volumeStruct, samplePos, entryPoints, entryParameters);\n";

    // configure classification
    headerSource += "#define RC_APPLY_CLASSIFICATION(transferFunc, voxel) ";
    if (classificationMode_.isSelected("none"))
        headerSource += "voxel;\n";
    else if (classificationMode_.isSelected("transfer-function"))
        headerSource += "applyTF(transferFunc, voxel);\n";

    // configure shading mode
    headerSource += "#define RC_APPLY_SHADING(gradient, samplePos, volumeStruct, ka, kd, ks) ";
    if (shadeMode_.isSelected("none"))
        headerSource += "ka;\n";
    else if (shadeMode_.isSelected("phong-diffuse"))
        headerSource += "phongShadingD(gradient, samplePos, volumeStruct, kd);\n";
    else if (shadeMode_.isSelected("phong-specular"))
        headerSource += "phongShadingS(gradient, samplePos, volumeStruct, ks);\n";
    else if (shadeMode_.isSelected("phong-diffuse-ambient"))
        headerSource += "phongShadingDA(gradient, samplePos, volumeStruct, kd, ka);\n";
    else if (shadeMode_.isSelected("phong-diffuse-specular"))
        headerSource += "phongShadingDS(gradient, samplePos, volumeStruct, kd, ks);\n";
    else if (shadeMode_.isSelected("phong"))
        headerSource += "phongShading(gradient, samplePos, volumeStruct, ka, kd, ks);\n";
    else if (shadeMode_.isSelected("toon"))
        headerSource += "toonShading(gradient, samplePos, volumeStruct, kd, 3);\n";

    // configure compositing mode
    headerSource += "#define RC_APPLY_COMPOSITING(result, color, samplePos, gradient, t, tDepth) ";
    if (compositingMode_.isSelected("dvr"))
        headerSource += "compositeDVR(result, color, t, tDepth);\n";
    else if (compositingMode_.isSelected("mip"))
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode_.isSelected("mida"))
        headerSource += "compositeMIDA(result, voxel, color, f_max_i, t, tDepth, gammaValue_);\n";
    else if (compositingMode_.isSelected("iso"))
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode_.isSelected("fhp"))
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode_.isSelected("fhn"))
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    if (applyLightAttenuation_.get())
        headerSource += "#define PHONG_APPLY_ATTENUATION\n";

    return headerSource;
}

void VolumeRaycaster::initProperties() {
    addProperty(samplingRate_);

    // initialization of the rendering properties
    // the properties are added in the respective subclasses
    maskingMode_.addOption("none", "none");
    maskingMode_.addOption("Segmentation", "Segmentation");

    gradientMode_.addOption("none",                 "none"                  );
    gradientMode_.addOption("forward-differences",  "Forward Differences"   );
    gradientMode_.addOption("central-differences",  "Central Differences"   );
    gradientMode_.addOption("filtered",             "Filtered"              );
    gradientMode_.select("central-differences");

    classificationMode_.addOption("none", "none");
    classificationMode_.addOption("transfer-function", "Transfer Function");
    classificationMode_.select("transfer-function");

    shadeMode_.addOption("none",                   "none"                   );
    shadeMode_.addOption("phong-diffuse",          "Phong (Diffuse)"        );
    shadeMode_.addOption("phong-specular",         "Phong (Specular)"       );
    shadeMode_.addOption("phong-diffuse-ambient",  "Phong (Diffuse+Amb.)"   );
    shadeMode_.addOption("phong-diffuse-specular", "Phong (Diffuse+Spec.)"  );
    shadeMode_.addOption("phong",                  "Phong (Full)"           );
    shadeMode_.addOption("toon",                   "Toon"                   );
    shadeMode_.select("phong");

    compositingMode_.addOption("dvr", "DVR");
    compositingMode_.addOption("mip", "MIP");
    compositingMode_.addOption("mida", "MIDA");
    compositingMode_.addOption("iso", "ISO");
    compositingMode_.addOption("fhp", "FHP");
    compositingMode_.addOption("fhn", "FHN");

    addProperty(useInterpolationCoarseness_);
    addProperty(interactionCoarseness_);
    addProperty(interactionQuality_);
}

void VolumeRaycaster::setGlobalShaderParameters(tgt::Shader* shader, tgt::Camera* camera) {
    VolumeRenderer::setGlobalShaderParameters(shader, camera);

    shader->setIgnoreUniformLocationError(true);

    // provide values needed for correct depth value calculation
    if (camera) {
        float n = camera->getNearDist();
        float f = camera->getFarDist();
        shader->setUniform("const_to_z_e_1", 0.5f + 0.5f*((f+n)/(f-n)));
        shader->setUniform("const_to_z_e_2", ((f-n)/(f*n)));
        shader->setUniform("const_to_z_w_1", ((f*n)/(f-n)));
        shader->setUniform("const_to_z_w_2", 0.5f*((f+n)/(f-n))+0.5f);
    }

    shader->setIgnoreUniformLocationError(false);
}

void VolumeRaycaster::portResized(RenderPort* p, tgt::ivec2 newsize) {
    size_ = newsize;

    int scale = 1;
    if (interactionMode())
        scale = interactionCoarseness_.get();

    VolumeRenderer::portResized(p, scale*newsize);
}

void VolumeRaycaster::interactionModeToggled() {
    if (interactionMode()) {
        switchToInteractionMode_ = true;
    }
    else {
        switchToInteractionMode_ = false;

        if (interactionCoarseness_.get() != 1) {
            // propagate to predecessing RenderProcessors
            const std::vector<Port*> inports = getInports();
            for(size_t i=0; i<inports.size(); ++i) {
                RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
                if (rp)
                    rp->resize(size_);
            }

            //distribute to outports:
            const std::vector<Port*> outports = getOutports();
            for(size_t i=0; i<outports.size(); ++i) {
                RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
                if (rp)
                    rp->resize(size_);
            }

            //distribute to private ports:
            const std::vector<RenderPort*> pports = getPrivateRenderPorts();
            for (size_t i=0; i<pports.size(); ++i) {
                RenderPort* rp = pports[i];
                rp->resize(size_);
            }
        }
        if(interactionQuality_.get() != 1.0f)
            invalidate();
    }
    VolumeRenderer::interactionModeToggled();
}

void VolumeRaycaster::invalidate(int inv) {
    if (switchToInteractionMode_) {
        switchToInteractionMode_ = false;

        if (interactionCoarseness_.get() != 1) {
            // propagate to predecessing RenderProcessors
            const std::vector<Port*> inports = getInports();
            for(size_t i=0; i<inports.size(); ++i) {
                RenderPort* rp = dynamic_cast<RenderPort*>(inports[i]);
                if (rp)
                    rp->resize(size_ / interactionCoarseness_.get());
            }

            //distribute to outports:
            const std::vector<Port*> outports = getOutports();
            for(size_t i=0; i<outports.size(); ++i) {
                RenderPort* rp = dynamic_cast<RenderPort*>(outports[i]);
                if (rp)
                    rp->resize(size_ / interactionCoarseness_.get());
            }

            //distribute to private ports:
            const std::vector<RenderPort*> pports = getPrivateRenderPorts();
            for (size_t i=0; i<pports.size(); ++i) {
                RenderPort* rp = pports[i];
                rp->resize(size_ / interactionCoarseness_.get());
            }
        }
    }

    VolumeRenderer::invalidate(inv);
}

void VolumeRaycaster::bindVolumes(tgt::Shader* shader, const std::vector<VolumeStruct> &volumes,
        const tgt::Camera* camera, const tgt::vec4& lightPosition) {

    VolumeRenderer::bindVolumes(shader, volumes, camera, lightPosition);

    shader->setIgnoreUniformLocationError(true);

    //TODO: This uses the first volume to set the step size. Could be changed so that step
    // size is calculated for each volume, but shaders need to be adapted as well to have volume
    // parameters available in ray setup and compositing. joerg
    if (volumes.size() > 0) {
        if (!volumes[0].volume_ || !volumes[0].volume_->getRepresentation<VolumeGL>()->getTexture()) {
            LWARNING("No volume texture");
        }
        else {
            tgt::ivec3 dim = volumes[0].volume_->getOriginalDimensions();

            // use dimension with the highest resolution for calculating the sampling step size
            float samplingRate = samplingRate_.get();
            float samplingStepSize = 1.f / (tgt::max(dim) * samplingRate);

            if (interactionMode())
                samplingStepSize /= interactionQuality_.get();

            shader->setUniform("samplingStepSize_", samplingStepSize);
            shader->setUniform("samplingRate_", samplingRate);
            
            LGL_ERROR;
        }
    }
    shader->setIgnoreUniformLocationError(false);
}

} // namespace voreen
