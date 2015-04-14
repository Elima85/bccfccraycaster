#include "unbiasedvolumeraycaster.h"

#include "tgt/textureunit.h"
#include "voreen/core/ports/conditions/portconditionvolumetype.h"

#include <sstream>
#include <ctime>

using tgt::vec3;
using tgt::TextureUnit;

namespace voreen {

const std::string UnbiasedVolumeRaycaster::loggerCat_("voreen.UnbiasedVolumeRaycaster");

UnbiasedVolumeRaycaster::UnbiasedVolumeRaycaster()
    : VolumeRaycaster()
    , volumeInport1_(Port::INPORT, "volume1", false, Processor::INVALID_PROGRAM)
    , volumeInport2_(Port::INPORT, "volume2", false, Processor::INVALID_PROGRAM)
    , volumeInport3_(Port::INPORT, "volume3", false, Processor::INVALID_PROGRAM)
    , volumeInport4_(Port::INPORT, "volume4", false, Processor::INVALID_PROGRAM)
    , entryPort_(Port::INPORT, "image.entrypoints")
    , exitPort_(Port::INPORT, "image.exitpoints")
    , outport_(Port::OUTPORT, "image.output", true, Processor::INVALID_PROGRAM, GL_RGBA16F_ARB)
    , outport1_(Port::OUTPORT, "image.output1", true, Processor::INVALID_PROGRAM, GL_RGBA16F_ARB)
    , outport2_(Port::OUTPORT, "image.output2", true, Processor::INVALID_PROGRAM, GL_RGBA16F_ARB)
    , raycastPrg_(0)
    , transferFunc_("transferFunction", "Transfer Function")
    , volumeType_("volumeType_", "Volume Type")
	, shouldRender_(false)
	, renderButton_("renderButton", "Render frame", Processor::INVALID_PROGRAM)
	, aValue_("aValue_", "a", 1, 1, 30, Processor::INVALID_PROGRAM)
	, nValue_("nValue_", "n", 1, 1, 30, Processor::INVALID_PROGRAM)
    , camera_("camera", "Camera", tgt::Camera(vec3(0.f, 0.f, 3.5f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f)))
    , compositingMode1_("compositing1", "Compositing (OP2)", Processor::INVALID_PROGRAM)
    , compositingMode2_("compositing2", "Compositing (OP3)", Processor::INVALID_PROGRAM)
{
    // ports
    volumeInport1_.addCondition(new PortConditionVolumeTypeGL());
    volumeInport2_.addCondition(new PortConditionVolumeTypeGL());
    volumeInport3_.addCondition(new PortConditionVolumeTypeGL());
    volumeInport4_.addCondition(new PortConditionVolumeTypeGL());
    addPort(volumeInport1_);
    addPort(volumeInport2_);
    addPort(volumeInport3_);
    addPort(volumeInport4_);
    addPort(entryPort_);
    addPort(exitPort_);
    addPort(outport_);
    addPort(outport1_);
    addPort(outport2_);

    // tf properties
    addProperty(transferFunc_);
    addProperty(classificationMode_);

	// camera position property
    addProperty(camera_);

    addProperty(renderButton_);

	// reconstruction properties
	volumeType_.addOption("cc", "CC");
	volumeType_.addOption("bcc", "BCC");
	volumeType_.addOption("fcc", "FCC");
	volumeType_.selectByKey("cc");
	addProperty(volumeType_);

	addProperty(aValue_);
	addProperty(nValue_);

	// shading modes
    addProperty(shadeMode_);

    // compositing modes
    addProperty(compositingMode_);
    compositingMode1_.addOption("dvr", "DVR");
    compositingMode1_.addOption("mip", "MIP");
    compositingMode1_.addOption("iso", "ISO");
    compositingMode1_.addOption("fhp", "W-FHP");
    //compositingMode1_.addOption("fhn", "FHN");
    addProperty(compositingMode1_);
    compositingMode2_.addOption("dvr", "DVR");
    compositingMode2_.addOption("mip", "MIP");
    compositingMode2_.addOption("iso", "ISO");
    compositingMode2_.addOption("fhp", "W-FHP");
    //compositingMode2_.addOption("fhn", "FHN");
    addProperty(compositingMode2_);
    addProperty(isoValue_);

    // lighting properties
    addProperty(lightPosition_);
    addProperty(lightAmbient_);
    addProperty(lightDiffuse_);
    addProperty(lightSpecular_);
    addProperty(materialShininess_);
    addProperty(applyLightAttenuation_);
    addProperty(lightAttenuation_);

    // assign lighting properties to property group
    lightPosition_.setGroupID("lighting");
    lightAmbient_.setGroupID("lighting");
    lightDiffuse_.setGroupID("lighting");
    lightSpecular_.setGroupID("lighting");
    materialShininess_.setGroupID("lighting");
    applyLightAttenuation_.setGroupID("lighting");
    lightAttenuation_.setGroupID("lighting");
    setPropertyGroupGuiName("lighting", "Lighting Parameters");

    // listen to changes of properties that influence the GUI state (i.e. visibility of other props)
	volumeType_.onChange(CallMemberAction<UnbiasedVolumeRaycaster>(this, &UnbiasedVolumeRaycaster::adjustPropertyReconstruction));
    classificationMode_.onChange(CallMemberAction<UnbiasedVolumeRaycaster>(this, &UnbiasedVolumeRaycaster::adjustPropertyVisibilities));
    shadeMode_.onChange(CallMemberAction<UnbiasedVolumeRaycaster>(this, &UnbiasedVolumeRaycaster::adjustPropertyVisibilities));
    compositingMode_.onChange(CallMemberAction<UnbiasedVolumeRaycaster>(this, &UnbiasedVolumeRaycaster::adjustPropertyVisibilities));
    compositingMode1_.onChange(CallMemberAction<UnbiasedVolumeRaycaster>(this, &UnbiasedVolumeRaycaster::adjustPropertyVisibilities));
    compositingMode2_.onChange(CallMemberAction<UnbiasedVolumeRaycaster>(this, &UnbiasedVolumeRaycaster::adjustPropertyVisibilities));
    applyLightAttenuation_.onChange(CallMemberAction<UnbiasedVolumeRaycaster>(this, &UnbiasedVolumeRaycaster::adjustPropertyVisibilities));
	renderButton_.onChange(CallMemberAction<UnbiasedVolumeRaycaster>(this, &UnbiasedVolumeRaycaster::setRender));
}

Processor* UnbiasedVolumeRaycaster::create() const {
    return new UnbiasedVolumeRaycaster();
}

void UnbiasedVolumeRaycaster::initialize() throw (VoreenException) {
    VolumeRaycaster::initialize();

    raycastPrg_ = ShdrMgr.loadSeparate("passthrough.vert", "rc_unbiased.frag",
        generateHeader(), false);

    if (!raycastPrg_)
        throw VoreenException("Failed to load shaders: passthrough.vert, rc_unbiased.frag");

    portGroup_.initialize();
    portGroup_.addPort(outport_);
    portGroup_.addPort(outport1_);
    portGroup_.addPort(outport2_);

    adjustPropertyVisibilities();

    if (transferFunc_.get()) {
        transferFunc_.get()->getTexture();
        transferFunc_.get()->invalidateTexture();
    }
}

void UnbiasedVolumeRaycaster::deinitialize() throw (VoreenException) {
    portGroup_.deinitialize();

    ShdrMgr.dispose(raycastPrg_);
    raycastPrg_ = 0;
    LGL_ERROR;

    VolumeRaycaster::deinitialize();
}

void UnbiasedVolumeRaycaster::compile() {
    raycastPrg_->setHeaders(generateHeader());
    raycastPrg_->rebuild();
}

bool UnbiasedVolumeRaycaster::isReady() const {
    //check if all inports are connected:

	if(!shouldRender_)
		return false;

    if (!entryPort_.isReady() || !exitPort_.isReady() || !volumeInport1_.isReady())
        return false;

	if (volumeType_.isSelected("bcc") || volumeType_.isSelected("fcc"))
		if (!volumeInport2_.isReady())
			return false;

	if (volumeType_.isSelected("fcc"))
		if (!volumeInport3_.isReady() && !volumeInport4_.isReady())
			return false;

    //check if at least one outport is connected:
    if (!outport_.isReady() && !outport1_.isReady() && !outport2_.isReady())
        return false;

    return true;
}

void UnbiasedVolumeRaycaster::beforeProcess() {
    VolumeRaycaster::beforeProcess();

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM) {
        PROFILING_BLOCK("compile");
        compile();
    }
    LGL_ERROR;

    transferFunc_.setVolumeHandle(volumeInport1_.getData());
}

void UnbiasedVolumeRaycaster::process() {

    // compile program if needed
    if (getInvalidationLevel() >= Processor::INVALID_PROGRAM)
        compile();
    LGL_ERROR;

    // bind transfer function
    TextureUnit transferUnit;
    transferUnit.activate();
    if (transferFunc_.get())
        transferFunc_.get()->bind();

    portGroup_.activateTargets();
    portGroup_.clearTargets();
    LGL_ERROR;

    transferFunc_.setVolumeHandle(volumeInport1_.getData());

     // bind entry params
    tgt::TextureUnit entryUnit, entryDepthUnit, exitUnit, exitDepthUnit;
    entryPort_.bindTextures(entryUnit, entryDepthUnit);
    LGL_ERROR;

    // bind exit params
    exitPort_.bindTextures(exitUnit, exitDepthUnit);
    LGL_ERROR;

    // vector containing the volumes to bind; is passed to bindVolumes()
    std::vector<VolumeStruct> volumeTextures;

	// set appropriate filter mode for selected reconstruction method
	GLint filterMode = GL_NEAREST;

    // bind volumes
    TextureUnit volUnit1, volUnit2, volUnit3, volUnit4;
    if (volumeInport1_.isReady()) {
        volumeTextures.push_back(VolumeStruct(
                    volumeInport1_.getData(),
                    &volUnit1,
                    "volumeStruct1_",
                    GL_CLAMP_TO_BORDER,
                    tgt::vec4(0.0),
                    filterMode)
                );
    }
	if (volumeType_.isSelected("bcc") || volumeType_.isSelected("fcc"))
		if (volumeInport2_.isReady()) {
        volumeTextures.push_back(VolumeStruct(
                    volumeInport2_.getData(),
                    &volUnit2,
                    "volumeStruct2_",
                    GL_CLAMP_TO_BORDER,
                    tgt::vec4(0.0),
                    filterMode)
                );
		}
	if (volumeType_.isSelected("fcc"))
		if (volumeInport3_.isReady() && volumeInport4_.isReady())
		{
			volumeTextures.push_back(VolumeStruct(
                    volumeInport3_.getData(),
                    &volUnit3,
                    "volumeStruct3_",
                    GL_CLAMP_TO_BORDER,
                    tgt::vec4(0.0),
                    filterMode)
                );
			volumeTextures.push_back(VolumeStruct(
                    volumeInport4_.getData(),
                    &volUnit4,
                    "volumeStruct4_",
                    GL_CLAMP_TO_BORDER,
                    tgt::vec4(0.0),
                    filterMode)
                );
		}

    // initialize shader
    raycastPrg_->activate();

    // set common uniforms used by all shaders
    tgt::Camera cam = camera_.get();
    setGlobalShaderParameters(raycastPrg_, &cam);
    // bind the volumes and pass the necessary information to the shader
    bindVolumes(raycastPrg_, volumeTextures, &cam, lightPosition_.get());

    // pass the remaining uniforms to the shader
    raycastPrg_->setUniform("entryPoints_", entryUnit.getUnitNumber());
    raycastPrg_->setUniform("entryPointsDepth_", entryDepthUnit.getUnitNumber());
    entryPort_.setTextureParameters(raycastPrg_, "entryParameters_");
    raycastPrg_->setUniform("exitPoints_", exitUnit.getUnitNumber());
    raycastPrg_->setUniform("exitPointsDepth_", exitDepthUnit.getUnitNumber());
    exitPort_.setTextureParameters(raycastPrg_, "exitParameters_");

    if (compositingMode_.get() ==  "iso" ||
        compositingMode1_.get() == "iso" ||
        compositingMode2_.get() == "iso")
        raycastPrg_->setUniform("isoValue_", isoValue_.get());

	raycastPrg_->setUniform("a_", aValue_.get());
	raycastPrg_->setUniform("n_", nValue_.get());

	if (classificationMode_.get() == "transfer-function")
        transferFunc_.get()->setUniform(raycastPrg_, "transferFunc_", transferUnit.getUnitNumber());

	GLuint timer_query;
	glGenQueries(1, &timer_query);
	glBeginQuery(GL_TIME_ELAPSED, timer_query);

	LGL_ERROR;
	{
		PROFILING_BLOCK("raycasting");
		renderQuad();
	}
	
	glEndQuery(GL_TIME_ELAPSED);
	GLuint64 timer_result;
	glGetQueryObjectui64v(timer_query, GL_QUERY_RESULT, &timer_result);
	LERROR("Time taken: " << double(timer_result) / 1e09 << " seconds");

    raycastPrg_->deactivate();
    portGroup_.deactivateTargets();

    TextureUnit::setZeroUnit();
    LGL_ERROR;

	shouldRender_ = false;
}

std::string UnbiasedVolumeRaycaster::generateHeader() {
    std::string headerSource = VolumeRaycaster::generateHeader();

    headerSource += transferFunc_.get()->getShaderDefines();

	if (volumeType_.isSelected("cc"))
		headerSource += "#define RECONSTRUCT_CC\n";
    else if (volumeType_.isSelected("bcc"))
		headerSource += "#define RECONSTRUCT_BCC\n";
	else if (volumeType_.isSelected("fcc"))
        headerSource += "#define RECONSTRUCT_FCC\n";

    // configure compositing mode for port 1
    headerSource += "#define RC_APPLY_COMPOSITING_1(result, color, samplePos, gradient, t, tDepth) ";
    if (compositingMode_.isSelected("dvr"))
        headerSource += "compositeDVR(result, color, t, tDepth);\n";
    else if (compositingMode_.isSelected("mip"))
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode_.isSelected("iso"))
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode_.isSelected("fhp"))
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode_.isSelected("fhn"))
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    // configure compositing mode for port 2
    headerSource += "#define RC_APPLY_COMPOSITING_2(result, color, samplePos, gradient, t, tDepth) ";
    if (compositingMode1_.isSelected("dvr"))
        headerSource += "compositeDVR(result, color, t, tDepth);\n";
    else if (compositingMode1_.isSelected("mip"))
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode1_.isSelected("iso"))
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode1_.isSelected("fhp"))
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode1_.isSelected("fhn"))
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    // configure compositing mode for port 3
    headerSource += "#define RC_APPLY_COMPOSITING_3(result, color, samplePos, gradient, t, tDepth) ";
    if (compositingMode2_.isSelected("dvr"))
        headerSource += "compositeDVR(result, color, t, tDepth);\n";
    else if (compositingMode2_.isSelected("mip"))
        headerSource += "compositeMIP(result, color, t, tDepth);\n";
    else if (compositingMode2_.isSelected("iso"))
        headerSource += "compositeISO(result, color, t, tDepth, isoValue_);\n";
    else if (compositingMode2_.isSelected("fhp"))
        headerSource += "compositeFHP(samplePos, result, t, tDepth);\n";
    else if (compositingMode2_.isSelected("fhn"))
        headerSource += "compositeFHN(gradient, result, t, tDepth);\n";

    // configure reconstruction function
    headerSource += "#define RC_APPLY_RECONSTRUCTION(p) ";
    if (volumeType_.isSelected("cc"))
        headerSource += "reconstructCC(p);\n";
    else if (volumeType_.isSelected("bcc"))
		headerSource += "reconstructBCC(p);\n";
	else if (volumeType_.isSelected("fcc"))
        headerSource += "reconstructFCC(p);\n";

    portGroup_.reattachTargets();
    headerSource += portGroup_.generateHeader(raycastPrg_);
    return headerSource;
}

void UnbiasedVolumeRaycaster::adjustPropertyReconstruction() {
    invalidate(Processor::INVALID_PROGRAM);
}

void UnbiasedVolumeRaycaster::adjustPropertyVisibilities() {
    bool useLighting = !shadeMode_.isSelected("none");
    setPropertyGroupVisible("lighting", useLighting);

    bool useIsovalue = (compositingMode_.isSelected("iso")  ||
                        compositingMode1_.isSelected("iso") ||
                        compositingMode2_.isSelected("iso"));
    isoValue_.setVisible(useIsovalue);

    lightAttenuation_.setVisible(applyLightAttenuation_.get());
}

void UnbiasedVolumeRaycaster::setRender() {
	toggleInteractionMode(false, this);
	shouldRender_ = true;
}

} // namespace
