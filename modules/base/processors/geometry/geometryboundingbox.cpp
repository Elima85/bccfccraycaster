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

#include "modules/base/processors/geometry/geometryboundingbox.h"

#include "voreen/core/datastructures/geometry/meshgeometry.h"
#include "voreen/core/datastructures/geometry/meshlistgeometry.h"

using std::min;
using std::max;
using tgt::vec3;

namespace voreen {

GeometryBoundingBox::GeometryBoundingBox()
    : Processor()
    , inport_(Port::INPORT, "geometry.input")
    , outport_(Port::OUTPORT, "geometry.output", true)
{
    addPort(inport_);
    addPort(outport_);
}

void GeometryBoundingBox::process() {
    vec3 llf(FLT_MAX);
    vec3 urb(-FLT_MAX);

    const Geometry* geom = inport_.getData();
    const MeshListGeometry* meshGeom = dynamic_cast<const MeshListGeometry*>(geom);

    if (meshGeom) {
        meshGeom->getBoundingBox(llf, urb);
        const MeshGeometry& mesh = MeshGeometry::createCube(llf, urb);

        outport_.setData(new MeshGeometry(mesh));
    }
    else {
        LERRORC("GeometryBoundingBox", "Only MeshListGeometries are supported in this processor");
    }

}

} // namespace
