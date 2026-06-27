// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_ILineProjectionProcessor_included
#define iipr_ILineProjectionProcessor_included


// ACF includes
#include <i2d/CLine2d.h>
#include <iimg/IBitmap.h>

// ACF-Solutions includes
#include <iproc/IProcessor.h>
#include <imeas/IDataSequence.h>


namespace iipr
{


class IProjectionParams;


class ILineProjectionProcessor: virtual public iproc::IProcessor
{
public:
	/**
		Do line projection with explicite projection parameters.
	*/
	virtual bool DoProjection(
				const iimg::IBitmap& bitmap,
				const i2d::CLine2d& projectionLine,
				const IProjectionParams* paramsPtr,
				imeas::IDataSequence& results,
				istd::IChangeable* processingReportPtr = NULL) = 0;
};


} // namespace iipr


#endif // !iipr_ILineProjectionProcessor_included


