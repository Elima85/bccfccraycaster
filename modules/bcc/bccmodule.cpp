#include "modules/bcc/bccmodule.h"
#include "modules/bcc/bccvolumeraycaster.h"
#include "modules/bcc/fccvolumeraycaster.h"
#include "modules/bcc/volumeinterleave.h"

namespace voreen {

const std::string BccModule::loggerCat_("voreen.BccModule");

BccModule::BccModule()
    : VoreenModule()
{
    setName("BCC");
    setXMLFileName("bcc/bccmodule.xml");

    addProcessor(new BccVolumeRaycaster());
    addProcessor(new FccVolumeRaycaster());
	addProcessor(new VolumeInterleave());

	addShaderPath(getModulesPath("bcc/glsl"));
}

void BccModule::initialize() throw (VoreenException) {
    VoreenModule::initialize();
}

void BccModule::deinitialize() throw (VoreenException) {
    VoreenModule::deinitialize();
}

} // namespace
