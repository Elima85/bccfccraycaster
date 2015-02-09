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

#ifndef VRN_POSITIONMETADATA_H
#define VRN_POSITIONMETADATA_H

#include "voreen/core/io/serialization/serialization.h"

namespace voreen {

/**
 * The @c PositionMetaData class stores position information of GUI widgets.
 *
 * @see MetaDataBase
 */
class VRN_CORE_API PositionMetaData : public MetaDataBase {
public:
    /**
     * Creates a @c PositionMetaData object storing the given position.
     *
     * @param x the x position
     * @param y the y position
     */
    PositionMetaData(const int& x = 0, const int& y = 0);
    virtual ~PositionMetaData();

    virtual std::string getClassName() const { return "PositionMetaData"; }
    virtual Serializable* create() const;
    virtual MetaDataBase* clone() const;
    virtual std::string toString() const;

    /**
     * @see Serializable::serialize
     */
    virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Serializable::deserialize
     */
    virtual void deserialize(XmlDeserializer& s);

    /**
     * Sets the x position.
     *
     * @param value the new position
     */
    void setX(const int& value);

    /**
     * Returns the x position.
     *
     * @return the position
     */
    int getX() const;

    /**
     * Sets the y position.
     *
     * @param value the new position
     */
    void setY(const int& value);

    /**
     * Returns the y position.
     *
     * @return the position
     */
    int getY() const;

private:
    /**
     * X position.
     */
    int x_;

    /**
     * Y position.
     */
    int y_;

};

} // namespace

#endif // VRN_POSITIONMETADATA_H
