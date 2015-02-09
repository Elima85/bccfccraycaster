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

#include "voreen/core/datastructures/volume/volumefactory.h"

using tgt::vec3;
using tgt::ivec3;
using tgt::svec3;

namespace voreen {

const std::string VolumeFactory::loggerCat_("voreen.VolumeFactory");

VolumeFactory::VolumeFactory() {
    generators_.push_back(new VolumeGeneratorUInt8());
    generators_.push_back(new VolumeGeneratorInt8());

    generators_.push_back(new VolumeGeneratorUInt16());
    generators_.push_back(new VolumeGeneratorInt16());

    generators_.push_back(new VolumeGeneratorUInt32());
    generators_.push_back(new VolumeGeneratorInt32());

    generators_.push_back(new VolumeGeneratorFloat());
    generators_.push_back(new VolumeGeneratorDouble());

    //Vector:
    generators_.push_back(new VolumeGenerator2xFloat()); 
    generators_.push_back(new VolumeGenerator2xDouble());
    generators_.push_back(new VolumeGenerator2xUInt8()); 
    generators_.push_back(new VolumeGenerator2xInt8()); 
    generators_.push_back(new VolumeGenerator2xUInt16());
    generators_.push_back(new VolumeGenerator2xInt16()); 
    generators_.push_back(new VolumeGenerator3xFloat()); 
    generators_.push_back(new VolumeGenerator3xDouble());
    generators_.push_back(new VolumeGenerator3xUInt8()); 
    generators_.push_back(new VolumeGenerator3xInt8()); 
    generators_.push_back(new VolumeGenerator3xUInt16());
    generators_.push_back(new VolumeGenerator3xInt16()); 
    generators_.push_back(new VolumeGenerator4xFloat()); 
    generators_.push_back(new VolumeGenerator4xDouble());
    generators_.push_back(new VolumeGenerator4xUInt8()); 
    generators_.push_back(new VolumeGenerator4xInt8()); 
    generators_.push_back(new VolumeGenerator4xUInt16());
    generators_.push_back(new VolumeGenerator4xInt16()); 

    //Matrix:
    generators_.push_back(new VolumeGeneratorMat3Float());
    generators_.push_back(new VolumeGeneratorMat3Double());
    generators_.push_back(new VolumeGeneratorMat4Float());
    generators_.push_back(new VolumeGeneratorMat4Double());

    //Tensor:
    generators_.push_back(new VolumeGeneratorTensor2Float());
}

VolumeFactory::~VolumeFactory() {
    while(!generators_.empty()) {
        delete generators_.back();
        generators_.pop_back();
    }
}

} // namespace voreen
