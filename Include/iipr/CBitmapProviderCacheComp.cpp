#include <iipr/CBitmapProviderCacheComp.h>


namespace iipr
{


// reimplemented (iimg::IBitmapProvider)

const iimg::IBitmap* CBitmapProviderCacheComp::GetBitmap() const
{
	return m_bitmapPtr.GetPtr();
}


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CBitmapProviderCacheComp::GetCalibration() const
{
	return m_transformPtr.GetPtr();
}


// reimplemented (istd::IChangeable)

bool CBitmapProviderCacheComp::CopyFrom(const IChangeable& object, CompatibilityMode mode)
{
	const iimg::IBitmapProvider* providerPtr = dynamic_cast<const iimg::IBitmapProvider*>(&object);
	if (providerPtr != nullptr){
		m_bitmapPtr.Reset();
		const iimg::IBitmap* bitmapPtr = providerPtr->GetBitmap();
		if (bitmapPtr != nullptr){
			m_bitmapPtr.MoveCastedPtr(bitmapPtr->CloneMe(mode));
		}

		m_transformPtr.Reset();

		const i2d::ICalibrationProvider* calibrationProviderPtr = dynamic_cast<const i2d::ICalibrationProvider*>(&object);
		if (calibrationProviderPtr != nullptr){
			const i2d::ICalibration2d* transformPtr = calibrationProviderPtr->GetCalibration();
			if (transformPtr != nullptr){
				m_transformPtr.MoveCastedPtr(transformPtr->CloneMe(mode));
			}
		}

		return true;
	}

	return false;
}


bool CBitmapProviderCacheComp::ResetData(CompatibilityMode /*mode*/)
{
	if (m_bitmapPtr.IsValid()){
		m_bitmapPtr->ResetImage();
	}

	if (m_transformPtr.IsValid()){
		m_transformPtr->ResetData();
	}

	return true;
}


} // namespace iipr


