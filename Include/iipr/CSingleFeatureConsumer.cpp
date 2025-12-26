#include <iipr/CSingleFeatureConsumer.h>


// ACF-Solutions includes
#include <imeas/INumericValue.h>


namespace iipr
{


// public methods
	
CSingleFeatureConsumer::CSingleFeatureConsumer(int featurePolicy)
	:m_featurePolicy(featurePolicy)
{
}


int CSingleFeatureConsumer::GetFeaturePolicy() const
{
	return m_featurePolicy;
}


void CSingleFeatureConsumer::SetFeaturePolicy(int featurePolicy)
{
	ResetFeatures();

	m_featurePolicy = featurePolicy;
}


// reimplemented (iipr::IFeaturesConsumer)

void CSingleFeatureConsumer::ResetFeatures()
{
	m_featurePtr.Reset();
}


bool CSingleFeatureConsumer::AddFeature(const imeas::INumericValue* featurePtr, bool* isFullPtr)
{
	Q_ASSERT(featurePtr != nullptr);

	switch (m_featurePolicy){
	case FP_FIRST:
		if (isFullPtr != nullptr){
			*isFullPtr = true;
		}

		if (!m_featurePtr.IsValid()){
			m_featurePtr.SetPtr(featurePtr);

			return true;
		}

		break;

	case FP_HEAVIEST:
		if (!m_featurePtr.IsValid()){
			m_featurePtr.SetPtr(featurePtr);

			return true;
		}
		else if (	featurePtr->IsValueTypeSupported(imeas::INumericValue::VTI_WEIGHT) && 
					m_featurePtr->IsValueTypeSupported(imeas::INumericValue::VTI_WEIGHT)){
				
			double featureWeight = featurePtr->GetComponentValue(imeas::INumericValue::VTI_WEIGHT).GetElement(0);
			double currentWeight = m_featurePtr->GetComponentValue(imeas::INumericValue::VTI_WEIGHT).GetElement(0);
				
			if (featureWeight > currentWeight){
				m_featurePtr.SetPtr(featurePtr);

				return true;
			}
		}
		break;

	case FP_LAST:
		m_featurePtr.SetPtr(featurePtr);

		return true;

	default:
		break;
	}

	delete featurePtr;

	return false;
}


// reimplemented (iipr::IFeaturesProvider)

int CSingleFeatureConsumer::GetFeaturesCount() const
{
	return m_featurePtr.IsValid()? 1: 0;
}


const imeas::INumericValue& CSingleFeatureConsumer::GetFeature(int I_IF_DEBUG(index)) const
{
	Q_ASSERT(m_featurePtr.IsValid());
	I_IF_DEBUG(Q_ASSERT(index == 0));

	return *m_featurePtr.GetPtr();
}


} // namespace iipr


