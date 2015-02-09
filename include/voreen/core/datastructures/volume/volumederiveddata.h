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

#ifndef VRN_VOLUMEDERIVEDDATA_H
#define VRN_VOLUMEDERIVEDDATA_H

#include "voreen/core/io/serialization/abstractserializable.h"

namespace voreen {

class VolumeHandleBase;

/**
 * Marker interface for data that is derived from a volume dataset,
 * such as histograms or preview images.
 *
 * @note each concrete subclass must provide a default constructor
 * @note each concrete subclass must be handled by a SerializableFactory
 *
 * @see DerivedDataFactory
 * @see VoreenModule::addSerializerFactory
 */
class VRN_CORE_API VolumeDerivedData : public AbstractSerializable {
public:
    VolumeDerivedData();
    virtual ~VolumeDerivedData() {}

    /**
     * Virtual constructor.
     *
     * @param handle the volume handle to derive from
     * @return the constructed derived data object, or null if construction is not possible
     */
    virtual VolumeDerivedData* createFrom(const VolumeHandleBase* handle) const = 0;

    virtual void serialize(XmlSerializer& s) const = 0;
    virtual void deserialize(XmlDeserializer& s) = 0;
};

} // namespace voreen

#endif
