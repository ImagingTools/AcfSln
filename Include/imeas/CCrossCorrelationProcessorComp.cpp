// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <imeas/CCrossCorrelationProcessorComp.h>


// Qt includes
#include <QtCore/QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtCore/QtMath>
#else
#include <QtCore/qmath.h>
#endif

 // ACF includes
#include <istd/CChangeNotifier.h>

// ACF-Solutions includes
#include <imeas/IMultiDataSequenceProvider.h>
#include <imeas/IDataSequence.h>


namespace imeas
{


// reimplmented (IDataSequenceCorrelationProcessor)

bool CCrossCorrelationProcessorComp::CalculateCorrelation(
			const imeas::IDataSequence& input1,
			const imeas::IDataSequence& input2,
			imeas::IDataSequence& correlationSequence) const
{
	int workingChannelsCount = qMin(input1.GetChannelsCount(), input2.GetChannelsCount());
	int samplesCountPerChannel1 = input1.GetSamplesCount() / input1.GetChannelsCount();
	int samplesCountPerChannel2 = input2.GetSamplesCount() / input2.GetChannelsCount();

	int outputSamplesPerChannelsCount = qMin(samplesCountPerChannel1, samplesCountPerChannel2);
	if (outputSamplesPerChannelsCount <= 0){
		return false;
	}

	if (!correlationSequence.CreateSequence(outputSamplesPerChannelsCount, workingChannelsCount)){
		return false;
	}

	for (int channelIndex = 0; channelIndex < workingChannelsCount; channelIndex++){
		double average1 = GetSampleAverage(input1, channelIndex);
		double average2 = GetSampleAverage(input2, channelIndex);

		double denom1 = 0.0;
		double denom2 = 0.0;

		for (int sampleIndex = 0; sampleIndex < samplesCountPerChannel1; sampleIndex++){
			double sample = input1.GetSample(sampleIndex, channelIndex);

			denom1 += (sample - average1) * (sample - average1);
		}

		for (int sampleIndex = 0; sampleIndex < samplesCountPerChannel2; sampleIndex++){
			double sample = input2.GetSample(sampleIndex, channelIndex);

			denom2 += (sample - average2) * (sample - average2);
		}

		double denominator = qSqrt(denom1 * denom2);

		// Calculate the correlation sequence
		int maxDelay = -outputSamplesPerChannelsCount / 2;
		for (int offset = -maxDelay; offset < maxDelay; offset++){
			double integral = 0.0;

			for (int i = 0;i < outputSamplesPerChannelsCount; i++){
				int j = i + offset;
				if (j < 0 || j >= outputSamplesPerChannelsCount){
					continue;
				}
				else{
					integral += (input1.GetSample(i, channelIndex) - average1) * (input2.GetSample(j, channelIndex) - average2);
				}
			}

			double correlationCoefficient = integral / denominator;

			correlationSequence.SetSample(offset + maxDelay, channelIndex, correlationCoefficient);
		}
	}

	return true;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CCrossCorrelationProcessorComp::DoProcessing(
				const iprm::IParamsSet* /*paramsPtr*/,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (outputPtr == NULL){
		return TS_OK;
	}

	const imeas::IMultiDataSequenceProvider* multiDataSequenceProviderPtr = dynamic_cast<const imeas::IMultiDataSequenceProvider*>(inputPtr);
	if (multiDataSequenceProviderPtr == NULL){
		SendErrorMessage(0, "Provider of the input data sequences not set");

		return TS_INVALID;
	}

	imeas::IDataSequence* correlationSequencePtr = dynamic_cast<imeas::IDataSequence*>(outputPtr);
	if (correlationSequencePtr == NULL){
		SendErrorMessage(0, "Provider of the input data sequences not set");

		return TS_INVALID;
	}
	
	int sequenceCount = multiDataSequenceProviderPtr->GetSequencesCount();
	if (sequenceCount < 2){
		SendErrorMessage(0, "At least two sequences must be provded to calculate the cross correlation");

		return TS_INVALID;	
	}

	const imeas::IDataSequence* sequence1Ptr = multiDataSequenceProviderPtr->GetDataSequence(0);
	const imeas::IDataSequence* sequence2Ptr = multiDataSequenceProviderPtr->GetDataSequence(1);
	if (sequence1Ptr == NULL || sequence2Ptr == NULL){
		SendErrorMessage(0, "At least one sequence object is invalid");

		return TS_INVALID;	
	}

	istd::CChangeNotifier changePtr(correlationSequencePtr);

	return CalculateCorrelation(*sequence1Ptr, *sequence2Ptr, *correlationSequencePtr) ? TS_OK : TS_INVALID;
}


// protected static methods

double CCrossCorrelationProcessorComp::GetSampleAverage(const imeas::IDataSequence& input, int channelIndex)
{
	int samplesCountPerChannel = input.GetSamplesCount() / input.GetChannelsCount();
	if (samplesCountPerChannel == 0){
		return 0;
	}

	double average = 0.0;

	for (int sampleIndex = 0; sampleIndex < samplesCountPerChannel; sampleIndex++){
		double sample = input.GetSample(sampleIndex, channelIndex);

		average += sample;
	}

	return average / double(samplesCountPerChannel);
}


} // namespace imeas


