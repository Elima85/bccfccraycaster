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

#include "volumeselector.h"

#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumecollection.h"
#include "voreen/core/processors/processorwidgetfactory.h"

namespace voreen {

const std::string VolumeSelector::loggerCat_("voreen.VolumeSelector");

VolumeSelector::VolumeSelector()
    : Processor(),
      volumeID_("volumeID", "Selected volume", 0, 0, 100),
      inport_(Port::INPORT, "volumecollection", 0),
      outport_(Port::OUTPORT, "volumehandle.volumehandle", 0)
{
    addPort(inport_);
    addPort(outport_);

    addProperty(volumeID_);
}

Processor* VolumeSelector::create() const {
    return new VolumeSelector();
}

void VolumeSelector::process() {
    // nothing
}

void VolumeSelector::initialize() throw (tgt::Exception) {
    Processor::initialize();

    adjustToVolumeCollection();
}

void VolumeSelector::invalidate(int /*inv = INVALID_RESULT*/) {
    adjustToVolumeCollection();
}

void VolumeSelector::adjustToVolumeCollection() {
    if (!outport_.isInitialized())
        return;

    const VolumeCollection* collection = inport_.getData();
    int max = ((collection != 0) ? static_cast<int>(collection->size()) : 0);

    if (collection && !collection->empty() && (volumeID_.get() < max)) {
        // adjust max id to size of collection
        if (volumeID_.getMaxValue() != max - 1) {
            volumeID_.setMaxValue(max - 1);
            if (volumeID_.get() > volumeID_.getMaxValue())
                volumeID_.set(volumeID_.getMaxValue());
            volumeID_.updateWidgets();
        }

        tgtAssert((volumeID_.get() >= 0) && (volumeID_.get() < max), "Invalid volume index");

        // update output handle
        if (collection->at(volumeID_.get()) != outport_.getData())
            outport_.setData(collection->at(volumeID_.get()), false);
    }
    else {
        if (max > 0)
            max--;

        // If the collection is smaller than the previous one, the maximum value
        // must be adjusted and the new value should be set.
        // The collection is 0 when deserializing the workspace, so that we must
        // not set value in that case, because it is the just deserialized one!
        volumeID_.setMaxValue(max);
        if (collection != 0 && !collection->empty()) {
            volumeID_.set(max);
            if (static_cast<int>(collection->size()) > volumeID_.get()
                && collection->at(volumeID_.get()) != outport_.getData())
            {
                outport_.setData(collection->at(volumeID_.get()), false);
            }
        } else {
            outport_.setData(0);
        }

        volumeID_.updateWidgets();
    }
}

} // namespace
