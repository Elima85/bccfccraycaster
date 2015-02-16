#ifndef VRN_FCCGRADIENTVOLUMERAYCASTER_H
#define VRN_FCCGRADIENTVOLUMERAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/floatproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class FccVolumeRaycaster : public VolumeRaycaster {
public:
    FccVolumeRaycaster();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "FCCVolumeRaycaster"; }
    virtual std::string getCategory() const     { return "Raycasting"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_EXPERIMENTAL; }

    virtual bool isReady() const;

protected:
    virtual void beforeProcess();
    virtual void process();

    virtual void initialize() throw (VoreenException);

    virtual void deinitialize() throw (VoreenException);

    virtual std::string generateHeader();
    virtual void compile();

private:
    void adjustPropertyVisibilities();
	void adjustPropertyReconstruction();

    VolumePort volumeInport1_;
    VolumePort volumeInport2_;
    VolumePort volumeInport3_;
    VolumePort volumeInport4_;
    RenderPort entryPort_;
    RenderPort exitPort_;

    RenderPort outport_;
    RenderPort outport1_;
    RenderPort outport2_;
    PortGroup portGroup_;

    tgt::Shader* raycastPrg_;               ///< shader program used by this raycaster (rc_multivolume.frag)

    TransFuncProperty transferFunc_;       ///< transfer function to apply to volume

	StringOptionProperty reconstruction_;

    CameraProperty camera_;                 ///< the camera used for lighting calculations

    StringOptionProperty compositingMode1_;   ///< What compositing mode should be applied for second outport
    StringOptionProperty compositingMode2_;   ///< What compositing mode should be applied for third outport

    static const std::string loggerCat_; ///< category used in logging
};


} // namespace voreen

#endif // VRN_FCCGRADIENTVOLUMERAYCASTER_H
