// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <imeas/CHistogramStatisticsProcessorComp.h>


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
#include <imeas/IDataSequence.h>
#include <imeas/IDataSequenceStatistics.h>


namespace imeas
{


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CHistogramStatisticsProcessorComp::DoProcessing(
				const iprm::IParamsSet* /*paramsPtr*/,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	const imeas::IDataSequence* dataSequencePtr = dynamic_cast<const imeas::IDataSequence*>(inputPtr);
	if (dataSequencePtr == NULL){
		SendErrorMessage(0, "Data sequence for statistics calculation not set");

		return TS_INVALID;
	}

	imeas::IDataSequenceStatistics* histogramStatisticsPtr = dynamic_cast<imeas::IDataSequenceStatistics*>(outputPtr);
	if (histogramStatisticsPtr == NULL){
		SendErrorMessage(0, "Statistics data object not set");

		return TS_INVALID;
	}

	return CalculateDataStatistics(*dataSequencePtr, *histogramStatisticsPtr);
}


// reimplemented (imeas::IDataSequenceStatisticsProcessor)

iproc::IProcessor::TaskState CHistogramStatisticsProcessorComp::CalculateDataStatistics(
			const imeas::IDataSequence& dataSequence,
			imeas::IDataSequenceStatistics& dataStatistics,
			istd::IChangeable* /*processingReportPtr*/) const
{
	istd::CChangeNotifier changePtr(&dataStatistics);

	if (dataSequence.IsEmpty()){
		dataStatistics.ResetStatistics();

		return TS_INVALID;
	}

	if (dataSequence.GetChannelsCount() != dataStatistics.GetChannelsCount()){
		dataStatistics.ResetStatistics();
	}

	bool retVal = true;
	int channelsCount = dataSequence.GetChannelsCount();
	for (int channelIndex = 0; channelIndex < channelsCount; channelIndex++){
		imeas::CSimpleDataStatistics channelStatistics;

		retVal = retVal && CalculateChannelStatistics(dataSequence, channelIndex, channelStatistics);

		retVal = retVal && dataStatistics.SetChannelStatistics(channelStatistics, channelIndex);
	}

	return retVal ? TS_OK : TS_INVALID;
}


bool CHistogramStatisticsProcessorComp::CalculateChannelStatistics(
			const imeas::IDataSequence& input,
			int channelIndex,
			imeas::CSimpleDataStatistics& dataStatistics) const
{
	int channleSamplesCount = input.GetSamplesCount();

	double average = 0.0;
	double median = -1.0;
	double standardDeviation = 0.0;
	istd::CRange boundaries;

	for (int sampleIndex = 0; sampleIndex < channleSamplesCount; sampleIndex++){
		double sample = input.GetSample(sampleIndex, channelIndex);

		if (sample != 0){
			boundaries.SetMinValue(sampleIndex / channleSamplesCount);
			break;
		}
	}

	for (int sampleIndex = channleSamplesCount - 1; sampleIndex >= 0; sampleIndex--){
		double sample = input.GetSample(sampleIndex, channelIndex);

		if (sample != 0){
			boundaries.SetMaxValue(sampleIndex / channleSamplesCount);
			break;
		}
	}

	double weights = 0.0;

	for (int sampleIndex = 0; sampleIndex < channleSamplesCount; sampleIndex++){
		double sample = input.GetSample(sampleIndex, channelIndex);

		average += sampleIndex * sample;

		weights += sample;

		if (weights >= 0.5 && median < 0){
			median = sampleIndex;
		}
	}

	for (int sampleIndex = 0; sampleIndex < channleSamplesCount; sampleIndex++){
		double sample = input.GetSample(sampleIndex, channelIndex);

		standardDeviation += sample * (sampleIndex - average) * (sampleIndex - average);
	}

	standardDeviation = qSqrt(standardDeviation);

	dataStatistics.CreateStatistics(average / channleSamplesCount, median / channleSamplesCount, standardDeviation / channleSamplesCount, boundaries);

	return true;
}


} // namespace imeas


