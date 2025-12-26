#ifndef iipr_IImageToFeatureProcessor_included
#define iipr_IImageToFeatureProcessor_included


// ACF includes
#include <iimg/IBitmap.h>
#include <iprm/IParamsSet.h>
#include <iproc/IProcessor.h>

#include <iipr/IFeaturesConsumer.h>


namespace iipr
{


/**
	Allow to extract some features from the image.
*/
class IImageToFeatureProcessor: virtual public iproc::IProcessor
{
public:
	/**
		Specialization of processing interface for extraction of features from image.
		\return				state of this processor task \sa iproc::IProcessor::TaskState.
	*/
	virtual iproc::IProcessor::TaskState DoExtractFeatures(
				const iprm::IParamsSet* paramsPtr,
				const iimg::IBitmap& image,
				IFeaturesConsumer& results,
				ibase::IProgressManager* progressManagerPtr = nullptr) = 0;
};


} // namespace iipr


#endif // !iipr_IImageToFeatureProcessor_included


