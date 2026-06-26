// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef imeas_CHistogramStatisticsProcessorComp_included
#define imeas_CHistogramStatisticsProcessorComp_included


// ACF includes
#include <iproc/TSyncProcessorCompBase.h>

// ACF-Solutions includes
#include <imeas/IDataSequenceStatisticsProcessor.h>
#include <imeas/CSimpleDataStatistics.h>


namespace imeas
{


class IDataSequence;
class IDataSequenceStatistics;


/**	
	Implementation of a processor for the image histogram calculation.
*/
class CHistogramStatisticsProcessorComp: public iproc::TSyncProcessorCompBase<IDataSequenceStatisticsProcessor>
{
public:
	typedef iproc::TSyncProcessorCompBase<IDataSequenceStatisticsProcessor> BaseClass;
	
	I_BEGIN_COMPONENT(CHistogramStatisticsProcessorComp);
		I_REGISTER_INTERFACE(IDataSequenceStatisticsProcessor);
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (imeas::IDataSequenceStatisticsProcessor)
	virtual iproc::IProcessor::TaskState CalculateDataStatistics(
				const imeas::IDataSequence& dataSequence,
				imeas::IDataSequenceStatistics& dataStatistics) const override;

private:
	bool CalculateChannelStatistics(const imeas::IDataSequence& input, int inputIndex, imeas::CSimpleDataStatistics& dataStatistics) const;
};


} // namespace imeas


#endif // !imeas_CHistogramStatisticsProcessorComp_included

