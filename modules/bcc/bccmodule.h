#ifndef VRN_BCCMODULE_H
#define VRN_BCCMODULE_H

#include "voreen/core/voreenmodule.h"

namespace voreen {

class BccModule : public VoreenModule {

public:
    BccModule();

    virtual std::string getDescription() const {
        return "Provides rendering of volumes in Body Centered Cubic and Face Centered Cubic format.";
    }

protected:
    virtual void initialize()
        throw (VoreenException);
    virtual void deinitialize()
        throw (VoreenException);

    static const std::string loggerCat_;
};

} // namespace

#endif // VRN_BCCMODULE_H
