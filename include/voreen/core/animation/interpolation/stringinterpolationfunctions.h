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

#ifndef VRN_VOLUMESTRINGINTERPOLATIONFUNCTIONS_H
#define VRN_VOLUMESTRINGINTERPOLATIONFUNCTIONS_H

#include "voreen/core/animation/interpolationfunction.h"

namespace voreen {

#ifdef DLL_TEMPLATE_INST
template class VRN_CORE_API InterpolationFunction<std::string>;
#endif

/**
 * This class VRN_CORE_API offers an interpolation function for strings. Interpolation: focus on startvalue.
 */
class VRN_CORE_API StringStartInterpolationFunction : public InterpolationFunction<std::string> {
public:
    StringStartInterpolationFunction();
    InterpolationFunction<std::string>* clone() const;
    std::string interpolate(std::string startvalue, std::string endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for strings. Interpolation: focus on endvalue.
 */
class VRN_CORE_API StringEndInterpolationFunction : public InterpolationFunction<std::string> {
public:
    StringEndInterpolationFunction();
    InterpolationFunction<std::string>* clone() const;
    std::string interpolate(std::string startvalue, std::string endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

/**
 * This class VRN_CORE_API offers an interpolation function for strings. Interpolation: bisection.
 */
class VRN_CORE_API StringStartEndInterpolationFunction : public InterpolationFunction<std::string> {
public:
    StringStartEndInterpolationFunction();
    InterpolationFunction<std::string>* clone() const;
    std::string interpolate(std::string startvalue, std::string endvalue, float time) const;

    std::string getMode() const;
    std::string getIdentifier() const;
};

} // namespace voreen
#endif
