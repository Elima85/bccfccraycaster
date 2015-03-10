#ifndef VRN_VOLUMEINTERLEAVE_H
#define VRN_VOLUMEINTERLEAVE_H

#include <string>
#include "voreen/core/processors/volumeprocessor.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/properties/boolproperty.h"

namespace voreen {

class VolumeHandle;

class VolumeInterleave : public CachingVolumeProcessor {
public:
    VolumeInterleave();
    virtual ~VolumeInterleave();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "VolumeInterleave"; }
    virtual std::string getCategory() const  { return "Volume Processing"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }

protected:
    virtual void process();

private:
	void applyOperator();
    void forceUpdate();

    VolumePort inport1_;
    VolumePort inport2_;
    VolumePort outport_;

    static const std::string loggerCat_; ///< category used in logging

    bool forceUpdate_;
};

}   //namespace

#endif
