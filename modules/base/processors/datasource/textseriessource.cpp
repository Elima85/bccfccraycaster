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

#include "textseriessource.h"

#include "voreen/core/voreenapplication.h"

namespace voreen {

TextSeriesSource::TextSeriesSource()
    : Processor()
    , filename_("seriesfile", "Text Series File", "Select Text Series File",
                VoreenApplication::app()->getVolumePath(), "Text File (*.txt)")
    , step_("step", "Time Step", 0, 0, 1000)
    , outport_(Port::OUTPORT, "text.outport", true)
{
    filename_.onChange(CallMemberAction<TextSeriesSource>(this, &TextSeriesSource::openTextFile));
    addProperty(filename_);

    step_.setStepping(1);
    step_.setTracking(false);
    addProperty(step_);

    addPort(outport_);
}

void TextSeriesSource::process() {
    int step = step_.get();
   
    if (step >= static_cast<int>(texts_.size()))
        return;

    std::string text = texts_[step];
    outport_.setData(text);
}

Processor* TextSeriesSource::create() const {
    return new TextSeriesSource;
}

void TextSeriesSource::openTextFile() {
    std::string filename = filename_.get();
    std::ifstream f(filename.c_str(), std::ios_base::in);
    if (!f) {
        LERRORC("voreen.TextSeriesSource", "Could not open file: " << filename);
        return;
    }
    texts_.clear();
    std::string line;
    while (f.good()) {
        getline(f, line);
        texts_.push_back(line);
    }

    step_.setMaxValue(static_cast<int>(texts_.size()));
}

} // namespace
