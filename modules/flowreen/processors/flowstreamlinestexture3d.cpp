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

#include "flowstreamlinestexture3d.h"
#include "modules/flowreen/datastructures/streamlinetexture.h"
#include "modules/flowreen/datastructures/volumeflow3d.h"

#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

FlowStreamlinesTexture3D::FlowStreamlinesTexture3D()
    : Processor(),
    processedVolumeHandle_(0),
    voxelSamplingProp_("voxelSampling", "voxel sampling: ", 10, 1, 100000),
    volInport_(Port::INPORT, "volumehandle.input"),
    volOutport_(Port::OUTPORT, "volumehandle.output", processedVolumeHandle_)
{

    CallMemberAction<FlowStreamlinesTexture3D> cma(this, &FlowStreamlinesTexture3D::calculateStreamlines);
    maxStreamlineLengthProp_.onChange(cma);
    thresholdProp_.onChange(cma);
    voxelSamplingProp_.onChange(cma);

    addProperty(maxStreamlineLengthProp_);
    addProperty(thresholdProp_);
    addProperty(voxelSamplingProp_);

    addPort(volInport_);
    addPort(volOutport_);
}

FlowStreamlinesTexture3D::~FlowStreamlinesTexture3D() {
    if ((processedVolumeHandle_ != 0) && (processedVolumeHandle_ != currentVolumeHandle_))
        delete processedVolumeHandle_;
}

void FlowStreamlinesTexture3D::process() {
    if (volInport_.isReady() && volInport_.hasChanged())
        calculateStreamlines();
}

// private methods
//

void FlowStreamlinesTexture3D::calculateStreamlines() {
    currentVolumeHandle_ = volInport_.getData();
    if (currentVolumeHandle_ == 0)
        return;

    const VolumeFlow3D* input = dynamic_cast<const VolumeFlow3D*>(currentVolumeHandle_->getRepresentation<Volume>());
    if (input == 0) {
        LERROR("process(): supplied VolumeHandle seems to contain no flow data! Cannot proceed.");
        return;
    }

    const size_t voxelSampling = static_cast<size_t>(voxelSamplingProp_.get());
    const Flow3D& flow = input->getFlow3D();
    tgt::vec2 thresholds(flow.maxMagnitude_ * thresholdProp_.get() / 100.0f);

    const int textureScaling = 1;
    unsigned char* streamlineTexture =
        StreamlineTexture<unsigned char>::integrateDraw(flow, textureScaling, voxelSampling, thresholds);

    Volume* output = new VolumeUInt8(streamlineTexture, flow.dimensions_ * textureScaling);
    if ((processedVolumeHandle_ != 0)
        && (processedVolumeHandle_ != currentVolumeHandle_)) {
            delete processedVolumeHandle_;
    }

    if (output != 0)
        processedVolumeHandle_ = new VolumeHandle(output, currentVolumeHandle_);
    else
        processedVolumeHandle_ = 0;

    volOutport_.setData(processedVolumeHandle_);
}

}   // namespace

