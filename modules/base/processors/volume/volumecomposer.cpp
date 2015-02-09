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

#include "volumecomposer.h"

#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/imagesequence.h"
#include "voreen/core/datastructures/volume/volumecollection.h"
#include "tgt/texturemanager.h"
#include "tgt/gpucapabilities.h"

namespace voreen {

const std::string VolumeComposer::loggerCat_("voreen.base.VolumeComposer");

VolumeComposer::VolumeComposer()
    : VolumeProcessor(),
      inportImages_(Port::INPORT, "imagesequence.in"),
      inportVolumes_(Port::INPORT, "volumecollection.in"),
      outport_(Port::OUTPORT, "volume.out"),
      voxelSpacing_("voxelSpacing", "Voxel Spacing", tgt::vec3(1.f), tgt::vec3(0.01f), tgt::vec3(10.f))
{
    setExpensiveComputationStatus(Processor::COMPUTATION_STATUS_PROGRESSBAR);

    addPort(inportImages_);
    addPort(inportVolumes_);
    addPort(outport_);

    addProperty(voxelSpacing_);
}

Processor* VolumeComposer::create() const {
    return new VolumeComposer();
}

void VolumeComposer::initialize() throw (tgt::Exception) {
    VolumeProcessor::initialize();
}

void VolumeComposer::deinitialize() throw (tgt::Exception) {
    VolumeProcessor::deinitialize();
}

bool VolumeComposer::isReady() const {
    bool ready = outport_.isReady();
    ready &= (inportImages_.isReady() || inportVolumes_.isReady()); 
    return ready;
}

void VolumeComposer::process() {
    if (inportImages_.isReady() && inportVolumes_.isReady()) {
        LWARNING("Composing from images AND volumes not supported. Please connect only one of the inports!");
        return;
    }

    if (inportImages_.isReady())
        stackImages();
    else if (inportVolumes_.isReady())
        stackVolumes();
    else {
        tgtAssert(false, "should not get here");
    }
}

void VolumeComposer::stackImages() {

    const ImageSequence* sequence = inportImages_.getData();
    if (!sequence || sequence->size() < 2) {
        outport_.setData(0);
        return;
    }

    tgt::ivec2 texDims = sequence->front()->getDimensions().xy();
    LINFO("Constructing volume from " << sequence->size() << " slices of dimensions " << texDims);

    // check that textures have the same dimension
    bool dimsMatch = true;
    for (size_t i=0; i<sequence->size(); i++) {
        if (texDims != sequence->at(i)->getDimensions().xy())
            dimsMatch = false;
    }
    if (!dimsMatch) {
        LWARNING("Images of input sequence differ in dimensions. Abort.");
        outport_.setData(0);
        return;
    }


    // download texture data to 16 bit uint buffer and concat frames
    int numFramePixels = tgt::hmul(texDims);
    uint16_t* dataBuffer = 0; 
    try {
        dataBuffer = new uint16_t[numFramePixels * sequence->size()];
        for (size_t i=0; i<sequence->size(); i++) {
            setProgress(static_cast<float>(i) / sequence->size());
            uint16_t* frameData = (uint16_t*)sequence->at(i)->downloadTextureToBuffer(GL_LUMINANCE, GL_UNSIGNED_SHORT);
            memcpy(dataBuffer+numFramePixels*i, frameData, numFramePixels*2);
            delete[] frameData;
        }
    }
    catch (std::bad_alloc&) {
        LERROR("Bad allocation during construction of output volume");
        delete[] dataBuffer;
        outport_.setData(0);
        return;
    }

    // construct volume from buffer and write it to outport
    VolumeUInt16* outputVol = new VolumeUInt16(dataBuffer, tgt::ivec3(texDims, static_cast<int>(sequence->size())));
    VolumeHandle* handle = new VolumeHandle(outputVol, voxelSpacing_.get(), tgt::vec3(0.0f)); //TODO: offset prop?
    outport_.setData(handle);

    LGL_ERROR;
}

void VolumeComposer::stackVolumes() {
    const VolumeCollection* inputCollection = inportVolumes_.getData();
    if (!inputCollection || inputCollection->empty()) {
        outport_.setData(0);
        return;
    }

    tgt::svec2 sliceDims = inputCollection->first()->getDimensions().xy();
    size_t numChannels = inputCollection->first()->getNumChannels();
    LINFO("Constructing volume from " << inputCollection->size() << " volumes with xy-slice dimensions " << sliceDims);
    
    // determine total number of z-slices and check that volumes have equal xy-slice dimensions
    bool dimsMatch = true;
    bool channelCountMatch = true;
    size_t numSlices = 0;
    int bpp = 1;
    for (size_t i=0; i<inputCollection->size(); i++) {
        const VolumeHandleBase* handle = inputCollection->at(i);
        if (sliceDims != handle->getDimensions().xy())
            dimsMatch = false;
        if (numChannels != handle->getNumChannels())
            channelCountMatch = false;
        tgtAssert(handle->getRepresentation<Volume>() != 0, "no cpu representation");
        bpp = std::max(bpp, handle->getRepresentation<Volume>()->getBytesPerVoxel());
        numSlices += handle->getDimensions().z;
    }
    if (!dimsMatch) {
        LWARNING("xy-slice dimensions of input volumes do not match. Abort.");
        outport_.setData(0);
        return;
    }
    if (!channelCountMatch) {
        LWARNING("Channel counts input volumes do not match. Abort.");
        outport_.setData(0);
        return;
    }

    // construct output volume
    tgt::svec3 volDims = tgt::svec3(sliceDims.x, sliceDims.y, numSlices);
    Volume* outputVolume = 0;
    if (numChannels == 1) {
        if (bpp == 1) {
            LINFO("Constructing UInt8 output volume with dimensions " << volDims);
            outputVolume = new VolumeUInt8(volDims);
        }
        else if (bpp == 2) {
            LINFO("Constructing UInt16 output volume with dimensions " << volDims);
            outputVolume = new VolumeUInt16(volDims);
        }
        else {
            LINFO("Constructing float output volume (fallback) with dimensions " << volDims);
            outputVolume = new VolumeFloat(volDims);
        }
    }
    else if (numChannels == 2) {
        if (bpp/numChannels == 1) {
            LINFO("Constructing 2xUInt8 output volume with dimensions " << volDims);
            outputVolume = new Volume2xUInt8(volDims);
        }
        else if (bpp == 2) {
            LINFO("Constructing 2xUInt16 output volume with dimensions " << volDims);
            outputVolume = new Volume2xUInt16(volDims);
        }
        else {
            LINFO("Constructing 2xfloat output volume (fallback )with dimensions " << volDims);
            outputVolume = new Volume2xFloat(volDims);
        }
    }
    else if (numChannels == 3) {
        if (bpp/numChannels == 1) {
            LINFO("Constructing 3xUInt8 output volume with dimensions " << volDims);
            outputVolume = new Volume3xUInt8(volDims);
        }
        else if (bpp == 2) {
            LINFO("Constructing 3xUInt16 output volume with dimensions " << volDims);
            outputVolume = new Volume3xUInt16(volDims);
        }
        else {
            LINFO("Constructing 3xfloat output volume (fallback) with dimensions " << volDims);
            outputVolume = new Volume3xFloat(volDims);
        }
    }
    else if (numChannels == 4) {
        if (bpp/numChannels == 1) {
            LINFO("Constructing 4xUInt8 output volume with dimensions " << volDims);
            outputVolume = new Volume4xUInt8(volDims);
        }
        else if (bpp == 2) {
            LINFO("Constructing 4xUInt16 output volume with dimensions " << volDims);
            outputVolume = new Volume4xUInt16(volDims);
        }
        else {
            LINFO("Constructing 4xfloat output volume (fallback) with dimensions " << volDims);
            outputVolume = new Volume4xFloat(volDims);
        }
    }
    else {
        LWARNING("Unsupported number of channels: " << numChannels << ". Abort.");
        outport_.setData(0);
        return;
    }
    tgtAssert(outputVolume, "output volume is null");

    // copy data from input volumes to output volume (stack in z-direction)
    size_t curSlice = 0;
    for (size_t volID = 0; volID<inputCollection->size(); volID++) {
        const Volume* vol = inputCollection->at(volID)->getRepresentation<Volume>();
        for (size_t z=0; z<vol->getDimensions().z; z++) {
            tgtAssert(curSlice < outputVolume->getDimensions().z, "invalid slice id");
            setProgress(static_cast<float>(curSlice) / outputVolume->getDimensions().z);
            for (size_t y=0; y<vol->getDimensions().y; y++) {
                for (size_t x=0; x<vol->getDimensions().x; x++) {
                    for (size_t channel = 0; channel < numChannels; channel++)
                        outputVolume->setVoxelFloat(vol->getVoxelFloat(x, y, z, channel), x, y, curSlice, channel);
                }
            }
            curSlice++;
        }
    }

    // create volume handle (take spacing and transformation from first volume)
    VolumeHandle* handle = new VolumeHandle(outputVolume, inputCollection->first());

    // put out result volume
    outport_.setData(handle);

}

} // voreen namespace
