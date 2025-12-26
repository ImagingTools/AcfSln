#include <iipr/CSearchFeature.h>


// ACF includes
#include <istd/TDelPtr.h>
#include <iser/IArchive.h>
#include <iser/CArchiveTag.h>


namespace iipr
{


// public methods

CSearchFeature::CSearchFeature()
	:m_isNegativeModelEnabled(false)
{
}


CSearchFeature::CSearchFeature(
			double weight,
			const i2d::CVector2d& position,
			double angle,
			const i2d::CVector2d& scale,
			bool negativeModel,
			const QByteArray& objectId)
	:BaseClass(weight, position, angle, scale),
	m_isNegativeModelEnabled(negativeModel)
{
	BaseClass::SetObjectId(objectId);
}


bool CSearchFeature::IsNegativeModelEnabled() const
{
	return m_isNegativeModelEnabled;
}


void CSearchFeature::SetNegativeModelEnabled(bool enable)
{
	if (m_isNegativeModelEnabled != enable){
		istd::CChangeNotifier notifier(this);
		
		m_isNegativeModelEnabled = enable;
	}
}


// reimplemented (iser::ISerializable)

bool CSearchFeature::Serialize(iser::IArchive& archive)
{
	bool retVal = true;

	retVal = retVal && BaseClass::Serialize(archive);

	static iser::CArchiveTag negativeModelTag("NegativeModelEnabled", "Is negative model enabled", iser::CArchiveTag::TT_LEAF);
	retVal = retVal && archive.BeginTag(negativeModelTag);
	retVal = retVal && archive.Process(m_isNegativeModelEnabled);
	retVal = retVal && archive.EndTag(negativeModelTag);

	return retVal;
}


// reimplemented (istd::IChangeable)

bool CSearchFeature::CopyFrom(const IChangeable& object, CompatibilityMode mode)
{
	const CSearchFeature* objectPtr = dynamic_cast<const CSearchFeature*>(&object);
	if (objectPtr != nullptr){
		istd::CChangeNotifier notifier(this);

		m_isNegativeModelEnabled = objectPtr->IsNegativeModelEnabled();

		BaseClass::CopyFrom(object, mode);

		return true;
	}

	return false;
}

istd::IChangeableUniquePtr CSearchFeature::CloneMe(CompatibilityMode mode) const
{
	istd::TDelPtr<CSearchFeature> retVal(new CSearchFeature);

	if (retVal->CopyFrom(*this, mode)){
		return retVal.PopPtr();
	}

	return nullptr;
}


} // namespace iipr


