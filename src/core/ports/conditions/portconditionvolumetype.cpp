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

#include "voreen/core/ports/conditions/portconditionvolumetype.h"

#include "voreen/core/ports/volumeport.h"
#include "voreen/core/utils/stringconversion.h"

namespace voreen {

PortConditionVolumeTypeGL::PortConditionVolumeTypeGL() 
        : PortCondition("OpenGL-compatible volume type expected: uint8/int8, uint16/int16, uint32/int32, float")
{
    // single channel
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeUInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeUInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeUInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeFloat());

    // two channels
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xUInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xUInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xUInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xFloat());

    // three channels
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xUInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xUInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xUInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xFloat());

    // four channels
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xUInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xUInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xUInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xFloat());
}

bool PortConditionVolumeTypeGL::acceptsPortData() const {
    return orCondition_.acceptsPortData();
}

void PortConditionVolumeTypeGL::setCheckedPort(const Port* checkedPort) {
    orCondition_.setCheckedPort(checkedPort);
}

// ----------------------------------------------------------------------------

PortConditionVolumeTypeReal::PortConditionVolumeTypeReal() 
    : PortCondition("Float or double volume expected")
{
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeFloat());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeDouble());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xFloat());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xDouble());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xFloat());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xDouble());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xFloat());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xDouble());
}

bool PortConditionVolumeTypeReal::acceptsPortData() const {
    return orCondition_.acceptsPortData();
}

void PortConditionVolumeTypeReal::setCheckedPort(const Port* checkedPort) {
    orCondition_.setCheckedPort(checkedPort);
}

// ----------------------------------------------------------------------------

PortConditionVolumeTypeInteger::PortConditionVolumeTypeInteger() 
    : PortCondition("Integer volume expected")
{
    // single channel
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeUInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeUInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeUInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeUInt64());
    orCondition_.addLinkedCondition(new PortConditionVolumeTypeInt64());

    // two channels
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xUInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xUInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xUInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xUInt64());
    orCondition_.addLinkedCondition(new PortConditionVolumeType2xInt64());

    // three channels
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xUInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xUInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xUInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xUInt64());
    orCondition_.addLinkedCondition(new PortConditionVolumeType3xInt64());


    // four channels
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xUInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xInt8());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xUInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xInt16());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xUInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xInt32());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xUInt64());
    orCondition_.addLinkedCondition(new PortConditionVolumeType4xInt64());

}

bool PortConditionVolumeTypeInteger::acceptsPortData() const {
    return orCondition_.acceptsPortData();
}

void PortConditionVolumeTypeInteger::setCheckedPort(const Port* checkedPort) {
    orCondition_.setCheckedPort(checkedPort);
}

// ----------------------------------------------------------------------------

PortConditionVolumeChannelCount::PortConditionVolumeChannelCount(size_t numChannels)
    : PortCondition("")
    , numChannels_(numChannels)
{
    tgtAssert(numChannels_ > 0, "invalid number of channels");
}

bool PortConditionVolumeChannelCount::acceptsPortData() const {
    if (!volumePort_ || !volumePort_->hasData())
        return false;

    return (volumePort_->getData()->getNumChannels() == numChannels_);
}

std::string PortConditionVolumeChannelCount::getDescription() const {
    if (numChannels_ == 1)
        return "Single-channel volume expected";
    else
        return "Volume with " + itos(numChannels_) + " channels expected";
}

void PortConditionVolumeChannelCount::setCheckedPort(const Port* checkedPort) {
    if (!dynamic_cast<const VolumePort*>(checkedPort)) {
        LERRORC("voreen.PortConditionVolumeChannelCount", "Assigned port is not a volume port");
    }
    else {
        volumePort_ = static_cast<const VolumePort*>(checkedPort);
    }
}

} // namespace
