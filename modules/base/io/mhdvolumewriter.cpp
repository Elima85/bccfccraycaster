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

#include "mhdvolumewriter.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

#include "tgt/filesystem.h"
#include "tgt/matrix.h"
#include <iomanip>

namespace voreen {

const std::string MhdVolumeWriter::loggerCat_("voreen.io.MhdVolumeWriter");

MhdVolumeWriter::MhdVolumeWriter() {
    extensions_.push_back("mhd");
}

void MhdVolumeWriter::write(const std::string& filename, const VolumeHandleBase* volumeHandle)
    throw (tgt::IOException)
{
    tgtAssert(volumeHandle, "No volume handle");
    const Volume* volume = volumeHandle->getRepresentation<Volume>();
    if (!volume) {
        LWARNING("No volume");
        return;
    }

    std::string mhdname = filename;
    std::string rawname = getFileNameWithoutExtension(filename) + ".raw";
    LINFO("saving " << mhdname << " and " << rawname);

    std::fstream mhdout(mhdname.c_str(), std::ios::out);
    std::fstream rawout(rawname.c_str(), std::ios::out | std::ios::binary);

    if (!mhdout.is_open() || !rawout.is_open() || mhdout.bad() || rawout.bad())
        throw tgt::IOException();

    mhdout << getMhdFileString(volumeHandle, rawname);
    if (mhdout.bad())
        throw tgt::IOException();
    mhdout.close();

    // write raw file
    const char* data = static_cast<const char*>(volume->getData());
    size_t numbytes = volume->getNumBytes();
    rawout.write(data, numbytes);
    if (rawout.bad())
        throw tgt::IOException();
    rawout.close();
}

std::string MhdVolumeWriter::getMhdFileString(const VolumeHandleBase* const volumeHandle, const std::string& rawFileName)
{
    std::ostringstream mhdout;
    tgtAssert(volumeHandle, "No volume handle");
    const Volume* volume = volumeHandle->getRepresentation<Volume>();
    if (!volume) {
        LWARNING("No volume casted volume data!");
        return "";
    }

    // write dat file
    std::string elementType;
    int numChannels = 0;

    if (dynamic_cast<const VolumeUInt8*>(volume)) {
        elementType = "MET_UCHAR";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeInt8*>(volume)) {
        elementType = "MET_CHAR";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeUInt16*>(volume)) {
        elementType = "MET_USHORT";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeInt16*>(volume)) {
        elementType = "MET_SHORT";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeUInt32*>(volume)) {
        elementType = "MET_UINT";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeInt32*>(volume)) {
        elementType = "MET_INT";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeUInt64*>(volume)) {
        elementType = "MET_ULONG";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeInt64*>(volume)) {
        elementType = "MET_LONG";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeFloat*>(volume)) {
        elementType = "MET_FLOAT";
        numChannels = 1;
    }
    else if (dynamic_cast<const VolumeDouble*>(volume)) {
        elementType = "MET_DOUBLE";
        numChannels = 1;
    }
    // vec2 types
    else if (dynamic_cast<const Volume2xUInt8*>(volume)) {
        elementType = "MET_UCHAR";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xInt8*>(volume)) {
        elementType = "MET_CHAR";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xUInt16*>(volume)) {
        elementType = "MET_USHORT";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xInt16*>(volume)) {
        elementType = "MET_SHORT";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xUInt32*>(volume)) {
        elementType = "MET_UINT";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xInt32*>(volume)) {
        elementType = "MET_INT";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xUInt64*>(volume)) {
        elementType = "MET_ULONG";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xInt64*>(volume)) {
        elementType = "MET_LONG";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xFloat*>(volume)) {
        elementType = "FLOAT";
        numChannels = 2;
    }
    else if (dynamic_cast<const Volume2xDouble*>(volume)) {
        elementType = "DOUBLE";
        numChannels = 2;
    }
    // vec3 types
    else if (dynamic_cast<const Volume3xUInt8*>(volume)) {
        elementType = "MET_UCHAR";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xInt8*>(volume)) {
        elementType = "MET_CHAR";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xUInt16*>(volume)) {
        elementType = "MET_USHORT";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xInt16*>(volume)) {
        elementType = "MET_SHORT";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xUInt32*>(volume)) {
        elementType = "MET_UINT";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xInt32*>(volume)) {
        elementType = "MET_INT";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xUInt64*>(volume)) {
        elementType = "MET_ULONG";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xInt64*>(volume)) {
        elementType = "MET_LONG";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xFloat*>(volume)) {
        elementType = "FLOAT";
        numChannels = 3;
    }
    else if (dynamic_cast<const Volume3xDouble*>(volume)) {
        elementType = "DOUBLE";
        numChannels = 3;
    }
    // vec4 types
    else if (dynamic_cast<const Volume4xUInt8*>(volume)) {
        elementType = "MET_UCHAR";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xInt8*>(volume)) {
        elementType = "MET_CHAR";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xUInt16*>(volume)) {
        elementType = "MET_USHORT";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xInt16*>(volume)) {
        elementType = "MET_SHORT";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xUInt32*>(volume)) {
        elementType = "MET_UINT";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xInt32*>(volume)) {
        elementType = "MET_INT";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xUInt64*>(volume)) {
        elementType = "MET_ULONG";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xInt64*>(volume)) {
        elementType = "MET_LONG";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xFloat*>(volume)) {
        elementType = "MET_FLOAT";
        numChannels = 4;
    }
    else if (dynamic_cast<const Volume4xDouble*>(volume)) {
        elementType = "MET_DOUBLE";
        numChannels = 4;
    }
    // special types
    else if (dynamic_cast<const VolumeMat3Float*>(volume)) {
        elementType = "MET_FLOAT";
        numChannels = 9;
    }
    else if (dynamic_cast<const VolumeTensor2Float*>(volume)) {
        elementType = "MET_FLOAT";
        numChannels = 6;
    } 
    else
        LERROR("Format currently not supported");

    mhdout << std::setprecision(12);

    mhdout << "ObjectType = Image" << std::endl;
    mhdout << "NDims = 3" << std::endl;
    tgt::ivec3 dims = volume->getDimensions();
    mhdout << "DimSize = " << dims.x << " " << dims.y << " " << dims.z << std::endl;
    mhdout << "ElementType = " << elementType << std::endl;
    tgt::vec3 os = volumeHandle->getOffset();
    os *= 100.0f;
    mhdout << "Offset = "  << os.x << " " << os.y << " " << os.z << std::endl;
    tgt::vec3 sp = volumeHandle->getSpacing();
    sp * 100.0f;
    mhdout << "ElementSpacing = "  << sp.x << " " << sp.y << " " << sp.z << std::endl;
    mhdout << "ElementByteOrderMSB = False" << std::endl;
    mhdout << "ElementNumberOfChannels = " << numChannels << std::endl;
    mhdout << "HeaderSize = 0" << std::endl;
    //ElementDataFile has to be last:
    mhdout << "ElementDataFile = " << tgt::FileSystem::fileName(rawFileName) << std::endl;

    return mhdout.str();
}

VolumeWriter* MhdVolumeWriter::create(ProgressBar* /*progress*/) const {
    return new MhdVolumeWriter(/*progress*/);
}

} // namespace voreen
