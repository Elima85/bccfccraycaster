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

#ifndef VRN_PLOTDATAPROPERTY_H
#define VRN_PLOTDATAPROPERTY_H

#include "modules/plotting/plottingmoduledefine.h"
#include "voreen/core/properties/templateproperty.h"
#include "voreen/core/properties/condition.h"

namespace voreen {

class PlotData;
#ifdef DLL_TEMPLATE_INST
    template class VRN_CORE_API TemplateProperty<const PlotData*>;
#endif
    
class PlotData;

/**
 * \brief   Property encapsulating a pointer to a PlotData object
 *
 * This property does not take ownership of the given PlotData!
 * It just provides just const access to the given PlotData and is e.g.
 * used for the tabular view of its data.
 */
class VRN_MODULE_PLOTTING_API PlotDataProperty : public TemplateProperty<const PlotData*> {
public:
    /**
     * Constructor.
     *
     * \param value     pointer to PlotData object, will be left as it is
     */
    PlotDataProperty(const std::string& id, const std::string& guiText, const PlotData* value,
               Processor::InvalidationLevel invalidationLevel=Processor::INVALID_RESULT);
    PlotDataProperty();

    virtual Property* create() const;

    virtual std::string getClassName() const       { return "PlotDataProperty"; }
    virtual std::string getTypeDescription() const { return "PlotData"; }

    /**
     * Assigns the passed PlotData object to this property but does not take
     * ownership of it.
     *
     * @note The previously assigned object will not be deleted!
     */
    void set(const PlotData* data);

    /**
     * Executes all member actions that belong to the property. Generally the owner of the
     * property is invalidated.
     */
    void notifyChange();

    /**
     * @see Property::serialize
     */
    //virtual void serialize(XmlSerializer& s) const;

    /**
     * @see Property::deserialize
     */
    //virtual void deserialize(XmlDeserializer& s);
};

} // namespace voreen

#endif // VRN_PLOTDATAPROPERTY_H
