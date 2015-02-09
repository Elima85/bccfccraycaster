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

#include "pythonmoduleqt.h"

namespace voreen {

const std::string PythonModuleQt::loggerCat_("voreen.python.PythonModuleQt");

PythonModuleQt::PythonModuleQt() 
    : VoreenModuleQt() 
    , pythonQt_(0)
{
    setName("Python (Qt)");
}

void PythonModuleQt::initialize() throw (tgt::Exception) {
    VoreenModuleQt::initialize();

    if (PythonModule::getInstance() && PythonModule::getInstance()->isInitialized())
        pythonQt_ = new PyVoreenQt();
    else
        LERROR("Skipped instantiation of VoreenPythonQt: Python module not initialized");
}

void PythonModuleQt::deinitialize() throw (tgt::Exception) {
    delete pythonQt_;
    pythonQt_ = 0;

    VoreenModuleQt::deinitialize();
}

} // namespace
