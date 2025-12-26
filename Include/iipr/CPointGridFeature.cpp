#include <iipr/CPointGridFeature.h>


// ACF includes
#include <istd/TDelPtr.h>
#include <istd/CChangeNotifier.h>
#include <iser/IArchive.h>
#include <iser/CArchiveTag.h>


namespace iipr
{


// public methods

CPointGridFeature::CPointGridFeature()
	:BaseClass()
{
	m_charucoIds.clear();
}


bool CPointGridFeature::AddChaurcoIds(const std::vector<int>& ids)
{
	m_charucoIds = ids;
	return true;
}

const std::vector<int>& CPointGridFeature::GetCharucoIds() const
{
	return m_charucoIds;
}


// reimplemented (imeas::INumericValue)

bool CPointGridFeature::IsValueTypeSupported(CPointGridFeature::ValueTypeId valueTypeId) const
{
	switch (valueTypeId){
	case VTI_AUTO:
		return true;

	default:
		return false;
	}
}


imath::CVarVector CPointGridFeature::GetComponentValue(CPointGridFeature::ValueTypeId valueTypeId) const
{
	imath::CVarVector result;

	switch (valueTypeId){
	case VTI_AUTO:
		return BaseClass::GetComponentValue(VTI_AUTO);

	default:
		break;
	}

	return result;
}

// reimplemented (istd::IChangeable)

istd::IChangeableUniquePtr CPointGridFeature::CloneMe(CompatibilityMode mode) const
{
	istd::TDelPtr<CPointGridFeature> retVal(new CPointGridFeature);

	if (retVal->CopyFrom(*this, mode)){
		return retVal.PopPtr();
	}

	return nullptr;
}


} // namespace iipr


