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

#ifndef VRN_TONEMAPPING_H
#define VRN_TONEMAPPING_H

#include "voreen/core/processors/imageprocessor.h"

#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/floatproperty.h"

namespace voreen {

/**
 * Performs different tone mapping upon input texture.
 */
class ToneMapping : public ImageProcessor {
public:
    ToneMapping();
    virtual Processor* create() const { return new ToneMapping(); }

    virtual std::string getCategory() const { return "Image Processing"; }
    virtual std::string getClassName() const { return "ToneMapping"; }
    virtual CodeState getCodeState() const { return CODE_STATE_EXPERIMENTAL; }

protected:
    void process();
    virtual std::string generateHeader(const tgt::GpuCapabilities::GlVersion* version = 0);
    virtual void compile();

    void toneMappingMethodChanged();
    void rahmanUpdate();

    virtual void initialize() throw (tgt::Exception);

    StringOptionProperty toneMappingMethod_;

    FloatProperty scurveSigma_;
    FloatProperty scurvePower_;
    FloatProperty rahmanFrequency_;
    FloatProperty rahmanSubtractionFactor_;
    FloatProperty rahmanIterations_;
    FloatProperty rahmanMaxLevel_;

    RenderPort inport_;
    RenderPort outport_;

private:
    bool rahmanTotalUpdate_;
    float wTotal_;
};


} // namespace voreen

#endif //VRN_TONEMAPPING_H
