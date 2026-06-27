// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_ICaliperProcessor_included
#define iipr_ICaliperProcessor_included


// ACF includes
#include <iproc/IProcessor.h>

// ACF-Solutions includes
#include <imeas/IDataSequence.h>

#include <iipr/IFeaturesConsumer.h>


namespace iipr
{


class ICaliperParams;


class ICaliperProcessor: virtual public iproc::IProcessor
{
public:
	enum FeatureTypeId
	{
		/**
			Unique ID of the caliper feature.
		*/
		FTI_CALIPER_FEATURE = 6572
	};

	virtual bool DoCaliper(
				const imeas::IDataSequence& derivative,
				const ICaliperParams* paramsPtr,
				IFeaturesConsumer& results,
				istd::IChangeable* processingReportPtr = NULL) = 0;
};


} // namespace iipr


#endif // !iipr_ICaliperProcessor_included


