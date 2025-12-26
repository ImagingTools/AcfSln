#include <iipr/CExtremumCaliperProcessorComp.h>


// ACF includes
#include <iprm/TParamsPtr.h>


// ACF-Solutions includes
#include <imeas/IDataSequence.h>
#include <imeas/CSamplesInfo.h>

#include <iipr/ICaliperParams.h>
#include <iipr/IFeaturesConsumer.h>
#include <iipr/CCaliperFeature.h>


namespace iipr
{


// reimplemented (iipr::ICaliperProcessor)

bool CExtremumCaliperProcessorComp::DoCaliper(
			const imeas::IDataSequence& derivative,
			const ICaliperParams* paramsPtr,
			IFeaturesConsumer& results)
{
	if (paramsPtr == nullptr){
		return false;
	}

	int samplesCount = derivative.GetSamplesCount();
	if ((samplesCount <= 0) || (derivative.GetChannelsCount() <= 0)){
		return false;
	}

	double weightThreshold = paramsPtr->GetWeightThreshold();
	Q_ASSERT(weightThreshold >= 0);

	istd::CRange proportionRange(0, 1);

	const imeas::CSamplesInfo* derivativeInfoPtr = dynamic_cast<const imeas::CSamplesInfo*>(derivative.GetSequenceInfo());
	if (derivativeInfoPtr != nullptr){
		const istd::CRange& logicalRange = derivativeInfoPtr->GetLogicalSamplesRange();
		if (logicalRange.IsValid()){
			proportionRange = logicalRange;
		}
	}

	int polarityMode = paramsPtr->GetPolarityMode();
	bool isRisedEdgeSupported = (polarityMode != ICaliperParams::PM_DROPPED);
	bool isDropppedEdgeSupported = (polarityMode != ICaliperParams::PM_RISED);

	results.ResetFeatures();

	bool isBackward = (paramsPtr->GetDirectionMode() == ICaliperParams::DM_BACKWARD);
	int index = -1;
	int nextIndex = isBackward? samplesCount - 1: 0;
	int indexDiff = isBackward? -1: 1;
	int endIndex = isBackward? -1: samplesCount;

	int risedCount = -1;
	int droppedCount = -1;

	double prevValue = 0;
	double value = 0;

	for (; nextIndex != endIndex; nextIndex += indexDiff){
		double nextValue = derivative.GetSample(nextIndex);
		if (nextValue < value){
			if (isRisedEdgeSupported && (risedCount > 0)){
				if (value > weightThreshold){
					double prevDelta = value - prevValue;
					Q_ASSERT(prevDelta >= 0);

					double nextDelta = value - nextValue;
					Q_ASSERT(nextDelta >= 0);
					Q_ASSERT(nextDelta + prevDelta > 0);

					double deltaSum = (nextDelta + prevDelta);
					Q_ASSERT(deltaSum >= 0);

					double shift = prevDelta / deltaSum;

					double position = proportionRange.GetValueFromAlpha((index + shift) / samplesCount);
					Q_ASSERT(position >= 0 - I_EPSILON);
					Q_ASSERT(position <= 1 + I_EPSILON);

					bool isReady = false;
					if (		!results.AddFeature(new CCaliperFeature(value, position, CCaliperFeature::EM_RISING), &isReady) ||
								isReady){
						return isReady;
					}
				}
			}

			risedCount = 0;
			droppedCount++;
		}
		else{
			if (isDropppedEdgeSupported && (droppedCount > 0)){
				if (value < -weightThreshold){
					double prevDelta = prevValue - value;
					Q_ASSERT(prevDelta >= 0);

					double nextDelta = nextValue - value;
					Q_ASSERT(nextDelta >= 0);
					Q_ASSERT(nextDelta + prevDelta > 0);

					double deltaSum = (nextDelta + prevDelta);
					Q_ASSERT(deltaSum >= 0);

					double shift = prevDelta / deltaSum;

					double position = proportionRange.GetValueFromAlpha((index + shift) / samplesCount);
					Q_ASSERT(position >= 0 - I_EPSILON);
					Q_ASSERT(position <= 1 + I_EPSILON);

					bool isReady = false;
					if (		!results.AddFeature(new CCaliperFeature(-value, position, CCaliperFeature::EM_FALLING), &isReady) ||
								isReady){
						return isReady;
					}
				}
			}

			risedCount++;
			droppedCount = 0;
		}

		prevValue = value;
		value = nextValue;

		index = nextIndex;
	}

	return true;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CExtremumCaliperProcessorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/)
{
	if (outputPtr == nullptr){
		return TS_OK;
	}

	const imeas::IDataSequence* derivativePtr = dynamic_cast<const imeas::IDataSequence*>(inputPtr);
	IFeaturesConsumer* consumerPtr = dynamic_cast<IFeaturesConsumer*>(outputPtr);

	if (		(derivativePtr == nullptr) ||
				(consumerPtr == nullptr) ||
				(paramsPtr == nullptr)){
		return TS_INVALID;
	}

	iprm::TParamsPtr<ICaliperParams> caliperParamsPtr(paramsPtr, *m_caliperParamsIdAttrPtr);

	return DoCaliper(*derivativePtr, caliperParamsPtr.GetPtr(), *consumerPtr)? TS_OK: TS_INVALID;
}


// reimplemented (iipr::IFeatureToProjectionMapper)

bool CExtremumCaliperProcessorComp::GetProjectionPosition(
			const imeas::INumericValue& feature,
			const iprm::IParamsSet* /*paramsPtr*/,
			double& result) const
{
	imath::CVarVector values = feature.GetComponentValue(imeas::INumericValue::VTI_ALPHA);
	if (values.GetElementsCount() > 0){
		result = values.GetElement(0);

		return true;
	}

	return false;
}


} // namespace iipr

