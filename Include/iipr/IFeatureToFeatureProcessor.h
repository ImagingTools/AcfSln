// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_IFeatureToFeatureProcessor_included
#define iipr_IFeatureToFeatureProcessor_included


#include <iproc/IProcessor.h>

#include <iipr/IFeaturesConsumer.h>
#include <iipr/IFeaturesProvider.h>


namespace iipr
{


/**
	Allow to extract some features from another one.
*/
class IFeatureToFeatureProcessor: virtual public iproc::IProcessor
{
public:
	/**
		Specialization of processing interface for extraction of features from image.
		\return				state of this processor \sa iproc::IProcessor::TaskState.
	*/
	virtual iproc::IProcessor::TaskState DoConvertFeatures(
				const iprm::IParamsSet* paramsPtr,
				const IFeaturesProvider& container,
				IFeaturesConsumer& results,
				istd::IChangeable* processingReportPtr = NULL) = 0;
};


} // namespace iipr


#endif // !iipr_IFeatureToFeatureProcessor_included


