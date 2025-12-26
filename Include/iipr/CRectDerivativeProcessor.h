#ifndef iipr_CRectDerivativeProcessor_included
#define iipr_CRectDerivativeProcessor_included


// STL includes
#include <vector>

// ACF includes
#include <iproc/TSyncProcessorWrap.h>

// ACF-Solutions includes
#include <imeas/IDataSequence.h>


namespace iipr
{


/**
	Calculate derivative of projection using rectangular filter kernel.
*/
class CRectDerivativeProcessor: public iproc::CSyncProcessorBase
{
public:
	CRectDerivativeProcessor();

	template<typename InData, typename OutData, typename ValueType>
	static bool CalculateDerivative(const InData* channelData, int samplesCount, double filterLength, OutData* results);

	template<typename ValueType>
	static bool CalculateDerivative(const imeas::IDataSequence& source, double filterLength, imeas::IDataSequence& results);

	/**
		Do extremum features analyze.
	*/
	static bool DoDerivativeProcessing(const imeas::IDataSequence& source, double filterLength, imeas::IDataSequence& results, bool doublePrecision = false);

	/**
		Get parameter ID used to extract caliper parameter object from parameter set.
	*/
	const QByteArray& GetFilterParamsId() const;

	/**
		Set parameter ID used to extract caliper parameter object from parameter set.
		It is only needed while using general processing interface iproc::IProcessor.
	*/
	void SetFilterParamsId(const QByteArray& id);

	/**
		Sets calculation precision to use (true = double, false = float).
	*/
	void UseDoublePrecision(bool on);

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

private:
	QByteArray m_filterParamsId;
	bool m_doublePrecision;
};


// inline methods

inline const QByteArray& CRectDerivativeProcessor::GetFilterParamsId() const
{
	return m_filterParamsId;
}


inline void CRectDerivativeProcessor::SetFilterParamsId(const QByteArray& id)
{
	m_filterParamsId = id;
}


inline void CRectDerivativeProcessor::UseDoublePrecision(bool on)
{
	m_doublePrecision = on;
}


template<typename InData, typename OutData, typename ValueType>
bool CRectDerivativeProcessor::CalculateDerivative(const InData* channelData, int samplesCount, double filterLength, OutData* results)
{
	if (samplesCount < 2){
		return false;
	}

	ValueType halfRealLength = qMax(1.0, filterLength * 0.5);

	int sumOffset = int(halfRealLength);
	ValueType sumLastAlpha = halfRealLength - sumOffset;
	ValueType sumLastAlphaInv = 1 - sumLastAlpha;

	int projectionWidth = samplesCount - 1;

	ValueType leftSum = 0;
	ValueType leftWeight = 0;
	ValueType rightSum = channelData[0] * sumLastAlpha;
	ValueType rightWeight = sumLastAlpha;

	for (int x = -sumOffset; x < projectionWidth; ++x){
		if (x < projectionWidth - sumOffset){
			rightSum +=	channelData[x + sumOffset + 1] * sumLastAlpha +
				channelData[x + sumOffset] * sumLastAlphaInv;

			rightWeight += 1;
		}
		else if (x == projectionWidth - sumOffset){
			rightSum += channelData[x + sumOffset] * sumLastAlphaInv;

			rightWeight += sumLastAlphaInv;
		}

		if (x >= 0){
			ValueType diff = channelData[x];
			leftSum += diff;

			if (x > sumOffset){
				leftSum -= channelData[x - sumOffset] * sumLastAlphaInv;
				leftSum -= channelData[x - sumOffset - 1] * sumLastAlpha;
			}
			else if (x == sumOffset){
				leftSum -= channelData[x - sumOffset] * sumLastAlphaInv;

				leftWeight += sumLastAlpha;
			}
			else{
				leftWeight += 1;
			}

			rightSum -= diff;
			rightWeight -= 1;

			results[x] = rightSum / rightWeight - leftSum / leftWeight;
		}
	}

	return true;
}


template <typename ValueType>
bool CRectDerivativeProcessor::CalculateDerivative(const imeas::IDataSequence& source, double filterLength, imeas::IDataSequence& results)
{
	int samplesCount = source.GetSamplesCount();
	int channelsCount = source.GetChannelsCount();
	if ((samplesCount < 2) || (channelsCount < 1)){
		return false;
	}

	ValueType halfRealLength = qMax(1.0, filterLength * 0.5);

	int sumOffset = int(halfRealLength);
	ValueType sumLastAlpha = halfRealLength - sumOffset;
	ValueType sumLastAlphaInv = 1 - sumLastAlpha;

	int projectionWidth = samplesCount - 1;

	// Buffer to speed up data access (brings ca. 10%):
	std::vector<ValueType> channelData(samplesCount);

	for (int channelIndex = 0; channelIndex < channelsCount; ++channelIndex){
		for (int i = 0; i < samplesCount; ++i){
			channelData[i] = source.GetSample(i, channelIndex);
		}

		ValueType leftSum = 0;
		ValueType leftWeight = 0;
		ValueType rightSum = channelData[0] * sumLastAlpha;
		ValueType rightWeight = sumLastAlpha;

		for (int x = -sumOffset; x < projectionWidth; ++x){
			if (x < projectionWidth - sumOffset){
				rightSum +=	channelData[x + sumOffset + 1] * sumLastAlpha + channelData[x + sumOffset] * sumLastAlphaInv;

				rightWeight += 1;
			}
			else if (x == projectionWidth - sumOffset){
				rightSum += channelData[x + sumOffset] * sumLastAlphaInv;
				rightWeight += sumLastAlphaInv;
			}

			if (x >= 0){
				ValueType diff = channelData[x];
				leftSum += diff;

				if (x > sumOffset){
					leftSum -= channelData[x - sumOffset] * sumLastAlphaInv;
					leftSum -= channelData[x - sumOffset - 1] * sumLastAlpha;
				}
				else if (x == sumOffset){
					leftSum -= channelData[x - sumOffset] * sumLastAlphaInv;
					leftWeight += sumLastAlpha;
				}
				else{
					leftWeight += 1;
				}

				rightSum -= diff;
				rightWeight -= 1;

				results.SetSample(x, channelIndex, rightSum / rightWeight - leftSum / leftWeight);
			}
		}
	}

	return true;
}


} // namespace iipr


#endif // !iipr_CRectDerivativeProcessor_included


