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

#include "voreen/core/ports/geometryport.h"

#include "tgt/filesystem.h"
#include "voreen/core/utils/hashing.h"
#include <sstream>

namespace voreen {

GeometryPort::GeometryPort(PortDirection direction, const std::string& name,
      bool allowMultipleConnections, Processor::InvalidationLevel invalidationLevel)
    : GenericPort<Geometry>(direction, name, allowMultipleConnections, invalidationLevel)
{}

bool GeometryPort::supportsCaching() const {
    return true;
}

std::string GeometryPort::getHash() const {
    if (!hasData())
        return "";

    std::string hashes;
    std::vector<const Geometry*> geometries = getAllData();
    if (geometries.size() == 1)
        return geometries.front()->getHash();
    else {
        for (size_t i=0; i<geometries.size(); i++)
            hashes += geometries.at(i)->getHash();
        return VoreenHash::getHash(hashes);
    }

}

void GeometryPort::saveData(const std::string& path) const throw (VoreenException) {
    if (!hasData())
        throw VoreenException("Port is empty");
    tgtAssert(!path.empty(), "empty path");

    // append .xml if no extension specified
    std::string filename = path;
    if (tgt::FileSystem::fileExtension(filename).empty())
        filename += ".xml";

    // serialize workspace
    XmlSerializer s(filename);
    s.setUseAttributes(true);

    s.serialize("Geometry", getData());

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
    std::fstream fileStream(filename.c_str(), std::ios_base::out);
    if (fileStream.fail())
        throw SerializationException("Failed to open file '" + filename + "' for writing.");

    try {
        fileStream << textStream.str();
    }
    catch (std::exception& e) {
        throw SerializationException("Failed to write serialization data stream to file '"
            + filename + "': " + std::string(e.what()));
    }
    catch (...) {
        throw SerializationException("Failed to write serialization data stream to file '"
            + filename + "' (unknown exception).");
    }
    fileStream.close();
}

void GeometryPort::loadData(const std::string& path) throw (VoreenException) {
    tgtAssert(!path.empty(), "empty path");

    // append .xml if no extension specified
    std::string filename = path;
    if (tgt::FileSystem::fileExtension(filename).empty())
        filename += ".xml";

    // open file for reading
    std::fstream fileStream(filename.c_str(), std::ios_base::in);
    if (fileStream.fail()) {
        throw SerializationException("Failed to open file '" + tgt::FileSystem::absolutePath(filename) + "' for reading.");
    }

    // read data stream into deserializer
    XmlDeserializer d(filename);
    d.setUseAttributes(true);
    try {
        d.read(fileStream);
    }
    catch (SerializationException& e) {
        throw SerializationException("Failed to read serialization data stream from file '"
            + filename + "': " + e.what());
    }
    catch (...) {
        throw SerializationException("Failed to read serialization data stream from file '"
            + filename + "' (unknown exception).");
    }

    // deserialize workspace from data stream
    try {
        Geometry* geometry = 0;
        d.deserialize("Geometry", geometry);
        setData(geometry);
    }
    catch (std::exception& e) {
        throw SerializationException("Deserialization from file '" + filename + "' failed: " + e.what());
    }
    catch (...) {
        throw SerializationException("Deserialization from file '" + filename + "' failed (unknown exception).");
    }
}

} // namespace
