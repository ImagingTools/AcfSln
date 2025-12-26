#include <iipr/CMultiBitmapCacheComp.h>


// ACF includes
#include <istd/TDelPtr.h>


namespace iipr
{


CMultiBitmapCacheComp::CMultiBitmapCacheComp()
{
	m_copyConstraints = false;
}


// reimplemented (iimg::IMultiBitmapProvider)

const iprm::IOptionsList* CMultiBitmapCacheComp::GetBitmapListInfo() const
{
	if (!m_copyConstraints){
		return nullptr;
	}

	return &m_bitmapConstraints;
}


int CMultiBitmapCacheComp::GetBitmapsCount() const
{
	return m_bitmapsPtr.GetCount();
}


const iimg::IBitmap* CMultiBitmapCacheComp::GetBitmap(int bitmapIndex) const
{
	Q_ASSERT(bitmapIndex >= 0);
	Q_ASSERT(bitmapIndex < GetBitmapsCount());

	return m_bitmapsPtr.GetAt(bitmapIndex);
}


// reimplemented (i2d::IMultiCalibrationProvider)

const iprm::IOptionsList* CMultiBitmapCacheComp::GetCalibrationSelectionContraints() const
{
	return nullptr;
}


int CMultiBitmapCacheComp::GetCalibrationsCount() const
{
	return m_transformsPtr.GetCount();
}


const i2d::ICalibration2d* CMultiBitmapCacheComp::GetCalibration(int calibrationIndex) const
{
	Q_ASSERT(calibrationIndex >= 0);
	Q_ASSERT(calibrationIndex < GetBitmapsCount());
	Q_ASSERT(m_bitmapsPtr.GetCount() == m_transformsPtr.GetCount());

	return m_transformsPtr.GetAt(calibrationIndex);
}


// reimplemented (istd::IChangeable)

bool CMultiBitmapCacheComp::CopyFrom(const IChangeable& object, CompatibilityMode mode)
{
	bool retVal = false;
	
	if (m_copyConstraints){
		m_bitmapConstraints.Reset();
	}

	const iimg::IMultiBitmapProvider* providerPtr = CompCastPtr<const iimg::IMultiBitmapProvider>(&object);
	if (providerPtr != nullptr){
		int bitmapsCount = providerPtr->GetBitmapsCount();

		m_bitmapsPtr.SetCount(bitmapsCount);

		for (int i = 0; i < bitmapsCount; ++i){
			istd::TUniqueInterfacePtr<iimg::IBitmap> clonedBitmapPtr;
			const iimg::IBitmap* bitmapPtr = providerPtr->GetBitmap(i);
			if (bitmapPtr != nullptr){
				clonedBitmapPtr.MoveCastedPtr(bitmapPtr->CloneMe(mode));
			}
			m_bitmapsPtr.SetElementAt(i, clonedBitmapPtr.PopInterfacePtr());
		}

		retVal = true;

		if (m_copyConstraints){
			const iprm::IOptionsList* bitmapConstraintsPtr = providerPtr->GetBitmapListInfo();
			if (bitmapConstraintsPtr != nullptr){
				m_bitmapConstraints.m_count = bitmapConstraintsPtr->GetOptionsCount();
				m_bitmapConstraints.m_flags = bitmapConstraintsPtr->GetOptionsFlags();
				for (int i = 0; i < m_bitmapConstraints.m_count; i++){
					m_bitmapConstraints.m_names.append(bitmapConstraintsPtr->GetOptionName(i));
					m_bitmapConstraints.m_descriptions.append(bitmapConstraintsPtr->GetOptionDescription(i));
					m_bitmapConstraints.m_ids.append(bitmapConstraintsPtr->GetOptionId(i));
					m_bitmapConstraints.m_enabled.append(bitmapConstraintsPtr->IsOptionEnabled(i));
				}
			}
		}
	}

	const i2d::IMultiCalibrationProvider* calibrationProviderPtr = CompCastPtr<const i2d::IMultiCalibrationProvider>(&object);
	if (calibrationProviderPtr != nullptr){
		int calibrationsCount = calibrationProviderPtr->GetCalibrationsCount();

		m_transformsPtr.SetCount(calibrationsCount);

		for (int i = 0; i < calibrationsCount; ++i){
			istd::TUniqueInterfacePtr<i2d::ICalibration2d> clonedTransformPtr;
			const i2d::ICalibration2d* calibrationPtr = calibrationProviderPtr->GetCalibration(i);
			if (calibrationPtr != nullptr){
				clonedTransformPtr.MoveCastedPtr(calibrationPtr->CloneMe(mode));
			}
			m_transformsPtr.SetElementAt(i, clonedTransformPtr.PopInterfacePtr());
		}

		retVal = true;
	}

	return retVal;
}


// reimplemented (iser::ISerializable)

bool CMultiBitmapCacheComp::Serialize(iser::IArchive& /*archive*/)
{
	// do nothing; only for compatibility reasons
	return true;
}


// protected methods

// reimplemented (icomp::CComponentBase)

void CMultiBitmapCacheComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	m_copyConstraints = *m_copyConstraintsAttrPtr;
}


} // namespace iipr


