#ifndef VRN_BCCINTERLEAVEDVOLUMERAYCASTER_H
#define VRN_BCCINTERLEAVEDVOLUMERAYCASTER_H

#include "voreen/core/processors/volumeraycaster.h"

#include "voreen/core/properties/transfuncproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/properties/floatproperty.h"

#include "voreen/core/ports/volumeport.h"

namespace voreen {

class BccInterleavedVolumeRaycaster : public VolumeRaycaster {
public:
    BccInterleavedVolumeRaycaster();
    virtual Processor* create() const;

    virtual std::string getClassName() const    { return "BCCInterleavedVolumeRaycaster"; }
    virtual std::string getCategory() const     { return "Raycasting"; }
    virtual CodeState getCodeState() const      { return CODE_STATE_EXPERIMENTAL; }

    virtual bool isReady() const;

protected:
	virtual void beforeProcess();
    virtual void process();

    virtual void initialize() throw (tgt::Exception);

    virtual void deinitialize() throw (tgt::Exception);

    virtual std::string generateHeader();
    virtual void compile();

private:
    void adjustPropertyVisibilities();
    void adjustPropertyReconstruction();

	VolumeHandle* convertVolume();			///< converts volume to correct format interleaved

    VolumePort volumeInport1_;
	VolumePort volumeInport2_;
    RenderPort entryPort_;
    RenderPort exitPort_;

    RenderPort outport_;
    RenderPort outport1_;
    RenderPort outport2_;
    PortGroup portGroup_;

	VolumeHandle* convertedVolume_;
	
    tgt::Shader* raycastPrg_;               ///< shader program used by this raycaster (rc_multivolume.frag)

    TransFuncProperty transferFunc_;        ///< transfer function to apply to volume

    StringOptionProperty reconstruction_;   ///< reconstruction algorithm to use

    CameraProperty camera_;                 ///< the camera used for lighting calculations

    StringOptionProperty compositingMode1_; ///< What compositing mode should be applied for second outport
    StringOptionProperty compositingMode2_; ///< What compositing mode should be applied for third outport

    static const std::string loggerCat_; ///< category used in logging
};


} // namespace voreen

#endif // VRN_BCCINTERLEAVEDVOLUMERAYCASTER_H
