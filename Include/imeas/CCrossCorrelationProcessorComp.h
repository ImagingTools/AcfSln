// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef imeas_CCrossCorrelationProcessorComp_included
#define imeas_CCrossCorrelationProcessorComp_included


// ACF includes
#include <iproc/TSyncProcessorCompBase.h>

// ACF-Solutions includes
#include <imeas/IDataSequenceCorrelationProcessor.h>


namespace imeas
{


/**	
	Implementation of a processor for calculation of cross correlation between two data sequence objects.
*/
class CCrossCorrelationProcessorComp: public iproc::TSyncProcessorCompBase<IDataSequenceCorrelationProcessor>
{
public:
	typedef iproc::TSyncProcessorCompBase<IDataSequenceCorrelationProcessor> BaseClass;
	
	I_BEGIN_COMPONENT(CCrossCorrelationProcessorComp);
		I_REGISTER_INTERFACE(IDataSequenceCorrelationProcessor);
	I_END_COMPONENT;

	/**
		Calculate cross correlation between two data sequences.
	*/
	virtual bool CalculateCorrelation(
				const imeas::IDataSequence& input1,
				const imeas::IDataSequence& input2,
				imeas::IDataSequence& correlation,
				istd::IChangeable* processingReportPtr = NULL) const;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

protected:
	/**
		Calculate mean sample value for a given channel.
	*/
	static double GetSampleAverage(const imeas::IDataSequence& input, int channelIndex);
};


} // namespace imeas


#endif // !imeas_CCrossCorrelationProcessorComp_included

