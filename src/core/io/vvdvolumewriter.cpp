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

#include "voreen/core/io/vvdvolumewriter.h"
#include "voreen/core/io/vvdformat.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumehandle.h"

#include "tgt/filesystem.h"
#include "tgt/matrix.h"

namespace voreen {

const std::string VvdVolumeWriter::loggerCat_("voreen.io.VvdVolumeWriter");

VvdVolumeWriter::VvdVolumeWriter() {
    extensions_.push_back("vvd");
}

void VvdVolumeWriter::write(const std::string& filename, const VolumeHandleBase* volumeHandle)
    throw (tgt::IOException)
{
    tgtAssert(volumeHandle, "No volume handle");
    const Volume* volume = volumeHandle->getRepresentation<Volume>();
    if (!volume) {
        LWARNING("No volume");
        return;
    }

    std::string vvdname = filename;
    std::string rawname = getFileNameWithoutExtension(filename) + ".raw";
    LINFO("saving " << vvdname << " and " << rawname);

    // VVD: ---------------------------

    XmlSerializer s(vvdname);
    s.setUseAttributes(true);

    VvdObject o = VvdObject(volumeHandle, tgt::FileSystem::fileName(rawname));
    std::vector<VvdObject> vec;
    vec.push_back(o);

    s.serialize("Volumes", vec, "Volume");
    
    //errorList_ = s.getErrors();

    // write serialization data to temporary string stream
    std::ostringstream textStream;
    try {
        s.write(textStream);
        if (textStream.fail())
            throw SerializationException("Failed to write serialization data to string stream.");
    }
    catch (std::exception& e) {
        throw SerializationException("Failed to write serialization data to string stream: " + std::string(e.what()));
    }
    catch (...) {
        throw SerializationException("Failed to write serialization data to string stream (unknown exception).");
    }

    // Now we have a valid StringStream containing the serialization data.
    // => Open output file and write it to the file.
    std::fstream fileStream(vvdname.c_str(), std::ios_base::out);
    if (fileStream.fail())
        throw SerializationException("Failed to open file '" + vvdname + "' for writing.");

    try {
        fileStream << textStream.str();
    }
    catch (std::exception& e) {
        throw SerializationException("Failed to write serialization data stream to file '"
                                     + vvdname + "': " + std::string(e.what()));
    }
    catch (...) {
        throw SerializationException("Failed to write serialization data stream to file '"
                                     + vvdname + "' (unknown exception).");
    }
    fileStream.close();

    std::fstream rawout(rawname.c_str(), std::ios::out | std::ios::binary);

    if (!rawout.is_open() || rawout.bad())
        throw tgt::IOException();

    // RAW: ---------------------------
    const char* data = static_cast<const char*>(volume->getData());
    size_t numbytes = volume->getNumVoxels() * volume->getBytesPerVoxel();

    rawout.write(data, numbytes);
    if (rawout.bad())
        throw tgt::IOException();

    rawout.close();
}

VolumeWriter* VvdVolumeWriter::create(ProgressBar* /*progress*/) const {
    return new VvdVolumeWriter();
}

} // namespace voreen
