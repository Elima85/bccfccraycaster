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

#include "compositor.h"

#include "tgt/textureunit.h"

using tgt::TextureUnit;

namespace voreen {

Compositor::Compositor()
    : ImageProcessor("image/compositor")
    , compositingMode_("blendMode", "Blend mode", Processor::INVALID_PROGRAM)
    , weightingFactor_("weightingFactor", "Weighting factor", 0.5f, 0.0f, 1.0f)    
    , weightFirst_("weightFirst", "Weight (First Image)", 1.0f, 0.f, 1.0f)
    , weightSecond_("weightSecond", "Weight (Second Image)", 1.0f, 0.f, 1.0f)
    , addDepth_("addDepth", "Add Depth", false)
    , inport0_(Port::INPORT, "image.inport0")
    , inport1_(Port::INPORT, "image.inport1")
    , outport_(Port::OUTPORT, "image.outport")
{

    compositingMode_.addOption("depth-test",                "Depth Test",                   "MODE_DEPTH_TEST");
    compositingMode_.addOption("alpha-compositing",         "Alpha Compositing",            "MODE_ALPHA_COMPOSITING");
    compositingMode_.addOption("alpha-blending",            "Alpha Blending",               "MODE_ALPHA_BLENDING");
    compositingMode_.addOption("alpha-blending-b-over-a",   "Alpha Blending (B over A)",    "MODE_ALPHA_BLENDING_B_OVER_A");
    compositingMode_.addOption("weighted-average",          "Weighted Average",             "MODE_WEIGHTED_AVERAGE");
    compositingMode_.addOption("take-first",                "Take First",                   "MODE_TAKE_FIRST");
    compositingMode_.addOption("take-second",               "Take Second",                  "MODE_TAKE_SECOND");
    compositingMode_.addOption("take-second-if-ready",      "Take Second if ready",         "MODE_TAKE_SECOND_IF_READY");
    compositingMode_.addOption("first-has-priority",        "First Has Priority",           "MODE_FIRST_HAS_PRIORITY");
    compositingMode_.addOption("second-has-priority",       "Second Has Priority",          "MODE_SECOND_HAS_PRIORITY");
    compositingMode_.addOption("maximum-alpha",             "Maximum Alpha",                "MODE_MAXIMUM_ALPHA");
    compositingMode_.addOption("difference",                "Difference",                   "MODE_DIFFERENCE");
    compositingMode_.addOption("add",                       "Add",                          "MODE_ADD");
    compositingMode_.set("alpha-compositing");
    compositingMode_.onChange(CallMemberAction<Compositor>(this, &Compositor::compositingModeChanged));
    addProperty(compositingMode_);

    addProperty(weightingFactor_);
    addProperty(weightFirst_);
    addProperty(weightSecond_);
    addProperty(addDepth_);

    addPort(inport0_);
    addPort(inport1_);
    addPort(outport_);
}

Compositor::~Compositor() {}

void Compositor::initialize() throw (tgt::Exception){
    //call parent's initialize method
    ImageProcessor::initialize();
    //Hide properties that should be invisible at beginning
    compositingModeChanged();
}

bool Compositor::isReady() const {
    // in takeFirst/takeSecond mode, it is sufficient when the respective port is ready
    bool ready = false;
    ready |= (inport0_.isReady() && inport1_.isReady());
    ready |= (inport0_.isReady() && compositingMode_.isSelected("take-first"));
    ready |= (inport1_.isReady() && compositingMode_.isSelected("take-second"));
    ready |= ((inport0_.isReady() || inport1_.isReady()) && compositingMode_.isSelected("take-second-if-ready"));

    return ready;
}

void Compositor::process() {

    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();

    outport_.activateTarget();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_ALWAYS);

    TextureUnit colorUnit, depthUnit;
    TextureUnit colorUnit1, depthUnit1;
    inport0_.bindTextures(colorUnit.getEnum(), depthUnit.getEnum());
    inport1_.bindTextures(colorUnit1.getEnum(), depthUnit1.getEnum());

    // initialize shader
    program_->activate();
    setGlobalShaderParameters(program_);
    if (compositingMode_.get() != "take-second") {
        program_->setUniform("colorTex0_", colorUnit.getUnitNumber());
        program_->setUniform("depthTex0_", depthUnit.getUnitNumber());
        inport0_.setTextureParameters(program_, "textureParameters0_");
    }
    if (compositingMode_.get() != "take-first") {
        program_->setUniform("colorTex1_", colorUnit1.getUnitNumber());
        program_->setUniform("depthTex1_", depthUnit1.getUnitNumber());
        inport1_.setTextureParameters(program_, "textureParameters1_");
    }
    if (compositingMode_.get() == "take-second-if-ready") {
        if(inport1_.isReady()) {
            program_->setUniform("colorTex0_", colorUnit1.getUnitNumber());
            program_->setUniform("depthTex0_", depthUnit1.getUnitNumber());
            inport1_.setTextureParameters(program_, "textureParameters0_");
        }
        else if(inport0_.isReady()){
            program_->setUniform("colorTex0_", colorUnit.getUnitNumber());
            program_->setUniform("depthTex0_", depthUnit.getUnitNumber());
            inport0_.setTextureParameters(program_, "textureParameters0_");
        }
    }
    if (compositingMode_.get() == "weighted-average")
        program_->setUniform("weightingFactor_", weightingFactor_.get());

    if (compositingMode_.get() == "add") {
            program_->setUniform("weightFirst_", weightFirst_.get());
            program_->setUniform("weightSecond_", weightSecond_.get());
            program_->setUniform("addDepth_", addDepth_.get());
        }

    renderQuad();

    glDepthFunc(GL_LESS);

    program_->deactivate();
    outport_.deactivateTarget();
    TextureUnit::setZeroUnit();
    LGL_ERROR;
}

std::string Compositor::generateHeader(const tgt::GpuCapabilities::GlVersion* version) {
    std::string header = ImageProcessor::generateHeader(version);
    header += "#define " + compositingMode_.getValue() + "\n";
    return header;
}

void Compositor::compile() {
    if (program_)
        program_->setHeaders(generateHeader());
    ImageProcessor::compile();
}

void Compositor::compositingModeChanged() {
    weightingFactor_.setVisible(compositingMode_.get() == "weighted-average");
    weightFirst_.setVisible(compositingMode_.get() == "add");
    weightSecond_.setVisible(compositingMode_.get() == "add");
    addDepth_.setVisible(compositingMode_.get() == "add");
}

} // voreen namespace
