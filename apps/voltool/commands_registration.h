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

#ifndef VRN_COMMANDS_REGISTRATION_H
#define VRN_COMMANDS_REGISTRATION_H

#include "voreen/core/utils/cmdparser/command.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

#include "tgt/tgt_math.h"

namespace voreen {

class CommandRegistration : public Command {

public:
    CommandRegistration(const std::string& name = "", const std::string& shortName = "", const std::string& info = "", const std::string& parameterList = "", const int argumentNum = 1);
    bool execute(const std::vector<std::string>& parameters) = 0;

protected:
    tgt::vec3 transformFromVoxelToWorldCoords(tgt::vec3 point, const VolumeHandleBase* vol);
};

class CommandRegistrationUniformScaling : public CommandRegistration {
public:
    CommandRegistrationUniformScaling();
    bool checkParameters(const std::vector<std::string>& parameters);
    bool execute(const std::vector<std::string>& parameters);

};

class CommandRegistrationAffine : public CommandRegistration {
public:
    CommandRegistrationAffine();
    bool checkParameters(const std::vector<std::string>& parameters);
    bool execute(const std::vector<std::string>& parameters);

};

}   //namespace voreen

#endif //VRN_COMMANDS_REGISTRATION_H
