// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef imeas_IDataSequenceStatisticsProcessor_included
#define imeas_IDataSequenceStatisticsProcessor_included


// ACF includes
#include <iproc/IProcessor.h>


namespace imeas
{


class IDataSequence;
class IDataSequenceStatistics;


/**
	Processor interface for statistics calculation of a data sequence.
*/
class IDataSequenceStatisticsProcessor: virtual public iproc::IProcessor
{
public:
	/**
		Calculate statistics of a data sequence.
	*/
	virtual iproc::IProcessor::TaskState CalculateDataStatistics(
				const imeas::IDataSequence& dataSequence,
				imeas::IDataSequenceStatistics& dataStatistics,
				istd::IChangeable* processingReportPtr = NULL) const = 0;
};


} // namespace imeas


#endif // !imeas_IDataSequenceStatisticsProcessor_included


