// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CRectDerivativeProcessor.h>


// Qt includes
#include <QtCore/QObject>

// ACF includes
#include <istd/CChangeNotifier.h>
#include <istd/TSmartPtr.h>
#include <iprm/TParamsPtr.h>

// ACF-Solutions includes
#include <imeas/IDataSequence.h>
#include <imeas/CGeneralDataSequence.h>
#include <imeas/INumericValue.h>
#include <imeas/CSamplesInfo.h>


namespace iipr
{


// public

CRectDerivativeProcessor::CRectDerivativeProcessor()
:	m_doublePrecision(false)
{
}




bool CRectDerivativeProcessor::DoDerivativeProcessing(const imeas::IDataSequence& source, double filterLength, imeas::IDataSequence& results, bool doublePrecision)
{
	int samplesCount = source.GetSamplesCount();
	int channelsCount = source.GetChannelsCount();
	if ((samplesCount < 2) || (channelsCount < 1)){
		return false;
	}

	istd::CChangeNotifier notifier(&results);

	istd::CRange sourceProportionRange(0, 1);

	const imeas::CSamplesInfo* sourceInfoPtr = dynamic_cast<const imeas::CSamplesInfo*>(source.GetSequenceInfo());
	if (sourceInfoPtr != NULL){
		const istd::CRange& logicalRange = sourceInfoPtr->GetLogicalSamplesRange();
		if (logicalRange.IsValid()){
			sourceProportionRange = logicalRange;
		}
	}

	int projectionWidth = samplesCount - 1;

	double proportionXAlpha = 0.5 / samplesCount;
	istd::CRange resultProportionRange(
				sourceProportionRange.GetValueFromAlpha(proportionXAlpha),
				sourceProportionRange.GetValueFromAlpha(1.0 - proportionXAlpha));
	if (!results.CreateSequenceWithInfo(
				istd::TSmartPtr<const imeas::IDataSequenceInfo>(new imeas::CSamplesInfo(resultProportionRange)),
				projectionWidth,
				channelsCount)){
		return false;
	}

	if (projectionWidth > 0){
		// special case for imeas::CGeneralDataSequence gives up to 25% performance gain
		const imeas::CGeneralDataSequence* inputSequencePtr = dynamic_cast<const imeas::CGeneralDataSequence*>(&source);
		imeas::CGeneralDataSequence* resultsSequencePtr = dynamic_cast<imeas::CGeneralDataSequence*>(&results);

		if ((channelsCount == 1) && (inputSequencePtr != NULL) && (resultsSequencePtr != NULL)){
			const double* channelData = inputSequencePtr->GetSamplesBuffer();
			double* resultsData = resultsSequencePtr->GetSamplesBuffer();

			if (doublePrecision){
				return CalculateDerivative<double, double, double>(channelData, samplesCount, filterLength, resultsData);
			}
			else{
				return CalculateDerivative<double, double, float>(channelData, samplesCount, filterLength, resultsData);
			}
		}

		// Default samples processing
		if (doublePrecision){
			return CalculateDerivative<double>(source, filterLength, results);
		}
		else{
			return CalculateDerivative<float>(source, filterLength, results);
		}
	}

	return true;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CRectDerivativeProcessor::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	const imeas::IDataSequence* inputProjectionPtr = dynamic_cast<const imeas::IDataSequence*>(inputPtr);
	imeas::IDataSequence* outputProjectionPtr = dynamic_cast<imeas::IDataSequence*>(outputPtr);

	double filterLength = 5.0;
	if (paramsPtr != NULL){
		iprm::TParamsPtr<imeas::INumericValue> filterParamsPtr(paramsPtr, m_filterParamsId);
		if (filterParamsPtr.IsValid()){
			imath::CVarVector filterLengths = filterParamsPtr->GetValues();
			if (filterLengths.GetElementsCount() > 0){
				filterLength = filterLengths[0];
			}
		}
	}

	return DoDerivativeProcessing(*inputProjectionPtr, filterLength, *outputProjectionPtr, m_doublePrecision)?
				TS_OK:
				TS_INVALID;
}


} // namespace iipr


