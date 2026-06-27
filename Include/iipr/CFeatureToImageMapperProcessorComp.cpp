// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iipr/CFeatureToImageMapperProcessorComp.h>

#include <imeas/INumericValue.h>


namespace iipr
{


// reimplemented (iipr::IFeatureToFeatureProcessor)

iproc::IProcessor::TaskState CFeatureToImageMapperProcessorComp::DoConvertFeatures(
			const iprm::IParamsSet* paramsPtr,
			const IFeaturesProvider& container,
			IFeaturesConsumer& results,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (!m_featuresMapperCompPtr.IsValid()){
		return TS_INVALID;
	}

	int featuresCount = container.GetFeaturesCount();

	for (int featureIndex = 0; featureIndex < featuresCount; featureIndex++){
		const imeas::INumericValue& feature = container.GetFeature(featureIndex);

		i2d::CVector2d position;
		if (m_featuresMapperCompPtr->GetImagePosition(feature, paramsPtr, position)){
			double weight = 1.0;

			if (feature.IsValueTypeSupported(imeas::INumericValue::VTI_WEIGHT)){
				weight = feature.GetComponentValue(imeas::INumericValue::VTI_WEIGHT).GetElement(0);
			}

			PositionFeature* positionFeaturePtr = new PositionFeature(weight);
			positionFeaturePtr->SetPosition(position);

			bool isFull = false;
			results.AddFeature(positionFeaturePtr, &isFull);

			if (isFull){
				break;
			}
		}
		else if (!*m_ignoreNotMappableAttrPtr){
			return TS_INVALID;
		}
	}

	return TS_OK;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CFeatureToImageMapperProcessorComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (outputPtr == NULL){
		return TS_OK;
	}

	const IFeaturesProvider* containerPtr = dynamic_cast<const IFeaturesProvider*>(inputPtr);
	IFeaturesConsumer* consumerPtr = dynamic_cast<IFeaturesConsumer*>(outputPtr);

	if (		(containerPtr == NULL) ||
				(consumerPtr == NULL)){
		return TS_INVALID;
	}

	return DoConvertFeatures(paramsPtr, *containerPtr, *consumerPtr);
}


} // namespace iipr


