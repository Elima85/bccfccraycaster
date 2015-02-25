#include "volumeinterleave.h"
#include "voreen/core/datastructures/volume/volume.h"
#include "voreen/core/datastructures/volume/volumeatomic.h"
#include "voreen/core/datastructures/volume/volumehandle.h"
#include "voreen/core/datastructures/volume/volumeoperator.h"
//test
namespace voreen {

const std::string VolumeInterleave::loggerCat_("voreen.VolumeInterleave");

VolumeInterleave::VolumeInterleave()
    : CachingVolumeProcessor()
    , inport1_(Port::INPORT, "volumehandle.input1")
    , inport2_(Port::INPORT, "volumehandle.input2")
    , outport_(Port::OUTPORT, "volumehandle.output", 0)
    , forceUpdate_(true)
{
    addPort(inport1_);
    addPort(inport2_);
    addPort(outport_);

    setExpensiveComputationStatus(COMPUTATION_STATUS_PROGRESSBAR);
}

VolumeInterleave::~VolumeInterleave() {}

Processor* VolumeInterleave::create() const {
    return new VolumeInterleave();
}

void VolumeInterleave::process() {
	if (forceUpdate_ || inport1_.hasChanged() || inport2_.hasChanged()) {
        applyOperator();
    }
}

void VolumeInterleave::forceUpdate() {
    forceUpdate_ = true;
}

void VolumeInterleave::applyOperator() {
    const VolumeHandleBase* inputHandle1 = inport1_.getData();
    const VolumeHandleBase* inputHandle2 = inport2_.getData();
    const Volume* inputVolume1 = inputHandle1->getRepresentation<Volume>();
    const Volume* inputVolume2 = inputHandle2->getRepresentation<Volume>();
    VolumeHandle* outputVolume = 0;

    forceUpdate_ = false;

    if (inputVolume1->getNumChannels() != 3) {
		LERROR("Input volume 1 does not have 3, but " << inputVolume1->getNumChannels() << " channels.");
		return;
	}

	if (inputVolume2->getNumChannels() != 1) {
		LERROR("Input volume 2 does not have 1, but " << inputVolume1->getNumChannels() << " channels.");
		return;
	}

	if (inputVolume1->getDimensions() != inputVolume2->getDimensions()) {
		LERROR("Input volume 1 and 2 have different size.");
		return;
	}

    bool bit16 = inputVolume1->getBitsAllocated() > 8;

    if (bit16) {

		const VolumeAtomic<tgt::Vector3<uint16_t> >* inputGradient = static_cast<const VolumeAtomic<tgt::Vector3<uint16_t> > *>(inputVolume1);
		const VolumeAtomic<uint16_t> *inputIntensity = static_cast<const VolumeAtomic<uint16_t> *>(inputVolume2);

		VolumeAtomic<tgt::Vector4<uint16_t> >* output = new VolumeAtomic<tgt::Vector4<uint16_t> >(inputIntensity->getDimensions());

		tgt::ivec3 pos;
	    tgt::ivec3 dim = inputIntensity->getDimensions();

       for (pos.z = 0; pos.z < dim.z; ++pos.z) {
			if (progressBar_)
	            progressBar_->setProgress(static_cast<float>(pos.z) / static_cast<float>(dim.z));

            for (pos.y = 0; pos.y < dim.y; ++pos.y) {
                for (pos.x = 0; pos.x < dim.x; ++pos.x) {
					tgt::Vector3<uint16_t> gradient  = inputGradient->voxel(pos);
					uint16_t               intensity = inputIntensity->voxel(pos);

					output->voxel(pos) = tgt::Vector4<uint16_t>(gradient.x, gradient.y, gradient.z, intensity);
                }
            }
        }

		if (progressBar_)
			progressBar_->setProgress(1.0f);

		outputVolume = new VolumeHandle(output, inputHandle1);
		
	}
    else {
        LERROR("Unknown technique");
		return;
    }

    outport_.setData(outputVolume);
}

}
