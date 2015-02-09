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

#ifndef VRN_PLOTDATAEXPORT_H
#define VRN_PLOTDATAEXPORT_H

#include "../ports/plotport.h"

#include "plotdataexportbase.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/buttonproperty.h"

namespace voreen {

class PlotData;
class PlotFunction;

class PlotDataExport : public PlotDataExportBase {

public:
    PlotDataExport();
    virtual Processor* create() const;

    virtual std::string getCategory() const  { return "Export"; }
    virtual std::string getClassName() const { return "PlotDataExport"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_TESTING; }

protected:

private:
    void exportFile();

    FileDialogProperty outputFile_;   ///< filename of the file the value information should be saved in
    ButtonProperty rewrite_;

    static const std::string loggerCat_;
};

} //namespace
#endif // VRN_PLOTDATAEXPORT_H

