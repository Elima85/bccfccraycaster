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

#include "flowreenadapter.h"
#include "modules/flowreen/datastructures/volumeflow3d.h"
#include "modules/flowreen/datastructures/volumeoperatorflowmagnitude.h"

#include "voreen/core/datastructures/volume/volumehandle.h"

namespace voreen {

FlowreenAdapter::FlowreenAdapter()
    : Processor(),
    processedVolumeHandle_(0),
    volInport_(Port::INPORT, "volumehandle.input"),
    volOutport_(Port::OUTPORT, "volumehandle.output", true)
{
    addPort(volInport_);
    addPort(volOutport_);
}

FlowreenAdapter::~FlowreenAdapter() {
    if ((processedVolumeHandle_ != 0) && (processedVolumeHandle_ != currentVolumeHandle_))
        delete processedVolumeHandle_;
}

void FlowreenAdapter::process() {
    currentVolumeHandle_ = volInport_.getData();
    if (dynamic_cast<const Volume3xFloat*>(currentVolumeHandle_->getRepresentation<Volume>()) != 0)
        calculateMagnitudes();
    else
        LERROR("supplied VolumeHandle seems to contain no flow data! Cannot proceed.");
}

void FlowreenAdapter::initialize() throw (tgt::Exception) {
    FlowreenProcessor::init();
    Processor::initialize();
}

// private methods
//

void FlowreenAdapter::calculateMagnitudes() {
    const Volume3xFloat* input = dynamic_cast<const Volume3xFloat*>(currentVolumeHandle_->getRepresentation<Volume>());
    Volume3xFloat* output = 0;
    if(input) {
        tgt::ivec3 dimensions = input->getDimensions();
        const size_t numVoxels = dimensions.x * dimensions.y * dimensions.z;

        tgt::vec3* voxels = 0;
        try {
            voxels = new tgt::vec3[numVoxels];
        } catch (std::bad_alloc) {
            throw;
        }

        const tgt::ivec3 xyz(0, 1, 2);  // target voxel order is XYZ

        tgt::ivec3 pos(0, 0, 0);
        float maxValue = std::numeric_limits<float>::min();
        float minValue = std::numeric_limits<float>::max();
        float maxMagnitude = 0.0f;

        for (size_t i = 0; i < numVoxels; ++i) {
            voxels[i] = input->voxel(i);

            // calculate max and min of current voxel and dataset's min and max.
            float voxelMax = tgt::max(voxels[i]);
            float voxelMin = tgt::min(voxels[i]);
            float magnitude = tgt::length(voxels[i]);

            if (voxelMax > maxValue)
                maxValue = voxelMax;
            if (voxelMin < minValue)
                minValue = voxelMin;
            if (magnitude > maxMagnitude)
                maxMagnitude = magnitude;
        }

        LINFO("min. among all components of all voxels: " << minValue);
        LINFO("max. among all components of all voxels: " << maxValue);
        LINFO("max. magnitude among all  voxels: " << maxMagnitude);

        output = new VolumeFlow3D(voxels, dimensions, minValue, maxValue, maxMagnitude);
    } 

    if ((processedVolumeHandle_ != 0)
        && processedVolumeHandle_ != currentVolumeHandle_)
    {
            delete processedVolumeHandle_;
    }

    if (output != 0)
        processedVolumeHandle_ = new VolumeHandle(output, currentVolumeHandle_);
    else
        processedVolumeHandle_ = 0;

    volOutport_.setData(processedVolumeHandle_);
}

}   // namespace
