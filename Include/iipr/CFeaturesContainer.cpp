#include <iipr/CFeaturesContainer.h>


// ACF includes
#include <istd/CChangeNotifier.h>


namespace iipr
{


void CFeaturesContainer::RemoveFeature(int featureIndex)
{
	m_featuresList.RemoveAt(featureIndex);
}


imeas::INumericValue& CFeaturesContainer::GetFeatureRef(int index)
{
	Q_ASSERT(index >= 0);
	Q_ASSERT(index < m_featuresList.GetCount());

	imeas::INumericValue* featurePtr = m_featuresList.GetAt(index);
	Q_ASSERT(featurePtr != nullptr);

	return *featurePtr;
}


// reimplemented (iipr::IFeaturesConsumer)

void CFeaturesContainer::ResetFeatures()
{
	istd::CChangeNotifier changePtr(this);

	m_featuresList.Reset();
}


bool CFeaturesContainer::AddFeature(const imeas::INumericValue* featurePtr, bool* /*isFullPtr*/)
{
	if (featurePtr != nullptr){
		istd::CChangeNotifier changePtr(this);

		m_featuresList.PushBack(const_cast<imeas::INumericValue*>(featurePtr));
		
		return true;
	}

	return false;
}



// reimplemented (iipr::IFeaturesProvider)

int CFeaturesContainer::GetFeaturesCount() const
{
	return m_featuresList.GetCount();
}


const imeas::INumericValue& CFeaturesContainer::GetFeature(int index) const
{
	Q_ASSERT(index >= 0);
	Q_ASSERT(index < m_featuresList.GetCount());

	const imeas::INumericValue* featurePtr = m_featuresList.GetAt(index);
	Q_ASSERT(featurePtr != nullptr);

	return *featurePtr;
}


//reimplemented (istd::IChangeable)

bool CFeaturesContainer::ResetData(CompatibilityMode /*mode*/)
{
	ResetFeatures();

	return true;
}


} // namespace iipr


