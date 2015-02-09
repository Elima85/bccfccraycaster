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

#include "gaussian.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

Gaussian::Gaussian()
    : ImageProcessorBypassable("image/gaussian"),
      sigma_("sigma", "Sigma", 2.0f, 0.1f, 10.0f),
      blurRed_("blurRed", "Red channel", true),
      blurGreen_("blurGreen", "Green channel", true),
      blurBlue_("blurBlue", "Blue channel", true),
      blurAlpha_("blurAlpha", "Alpha channel", true),
      blurDepth_("blurDepth", "Depth channel", false),
      inport_(Port::INPORT, "image.inport"),
      outport_(Port::OUTPORT, "image.outport"),
      privatePort_(Port::OUTPORT, "image.privateport", true)
{
    addProperty(enableSwitch_);

    addProperty(sigma_);
    addProperty(blurRed_);
    addProperty(blurGreen_);
    addProperty(blurBlue_);
    addProperty(blurAlpha_);
    addProperty(blurDepth_);

    addPort(inport_);
    addPort(outport_);
    addPrivateRenderPort(&privatePort_);
}

void Gaussian::process() {

    if (!enableSwitch_.get()){
        bypass(&inport_, &outport_);
        return;
    }

    TextureUnit colorUnit, depthUnit;

    //Compute the Gauss kernel and norm
    float gaussKernel[25];
    float sigma = sigma_.get();
    int kernelRadius = static_cast<int>(sigma*2.5f);

    for (int i=0; i<=kernelRadius; i++)
        gaussKernel[i] = exp(-float(i*i)/(2.f*sigma*sigma));

    // compute norm
    float norm = 0.0;
    for (int i=1; i<=kernelRadius; i++)
        norm += gaussKernel[i];

    // so far we have just computed norm for one half
    norm = 2.f * norm + gaussKernel[0];

    privatePort_.activateTarget();
    privatePort_.clearTarget();

    inport_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("colorTex_", colorUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    inport_.setTextureParameters(program_, "textureParameters_");
    program_->setUniform("dir_", tgt::vec2(1.f,0.f));
    program_->setUniform("gaussKernel_", gaussKernel, 25);
    program_->setUniform("norm_", norm);
    program_->setUniform("kernelRadius_", kernelRadius);
    program_->setUniform("channelWeights_",  blurRed_.get() ? 1.f : 0.f,
        blurGreen_.get() ? 1.f : 0.f,
        blurBlue_.get() ? 1.f : 0.f,
        blurAlpha_.get() ? 1.f : 0.f);
    program_->setUniform("blurDepth_", blurDepth_.get());
    renderQuad();
    program_->deactivate();
    privatePort_.deactivateTarget();

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    privatePort_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    program_->setUniform("colorTex_", colorUnit.getUnitNumber());
    program_->setUniform("depthTex_", depthUnit.getUnitNumber());
    inport_.setTextureParameters(program_, "textureParameters_");
    program_->setUniform("dir_", tgt::vec2(0.f,1.f));
    program_->setUniform("gaussKernel_", gaussKernel, 25);
    program_->setUniform("norm_", norm);
    program_->setUniform("kernelRadius_", kernelRadius);
    program_->setUniform("channelWeights_",  blurRed_.get() ? 1.f : 0.f,
        blurGreen_.get() ? 1.f : 0.f,
        blurBlue_.get() ? 1.f : 0.f,
        blurAlpha_.get() ? 1.f : 0.f);
    program_->setUniform("blurDepth_", blurDepth_.get());
    renderQuad();

    program_->deactivate();
    outport_.deactivateTarget();
    LGL_ERROR;
}

} // voreen namespace
