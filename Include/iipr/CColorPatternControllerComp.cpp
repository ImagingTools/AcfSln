#include <iipr/CColorPatternControllerComp.h>


// ACF includes
#include <istd/CChangeNotifier.h>

// ACF-Solutions includes
#include <iipr/CBitmapOperations.h>


namespace iipr
{

	
// reimplemented (iipr::IPatternController)

bool CColorPatternControllerComp::TeachPattern(const istd::IChangeable* /*sourceObjectPtr*/)
{
	if (!m_patternBitmapProviderCompPtr.IsValid()){
		SendCriticalMessage(0, "Bad component configuration. Pattern bitmap provider 'PatternBitmapProvider' not set!");

		return false;
	}

	if (!m_bitmapRegionCompPtr.IsValid()){
		SendCriticalMessage(0, "Bad component configuration. Bitmap region 'BitmapRegion' not set!");

		return false;
	}

	if (!m_histogramProgressorCompPtr.IsValid()){
		SendCriticalMessage(0, "Bad component configuration. Histogram processor 'HistogramProgressor' not set!");

		return false;
	}


	const iimg::IBitmap* bitmapPtr = m_patternBitmapProviderCompPtr->GetBitmap();
	if (bitmapPtr == nullptr){
		SendErrorMessage(0, "Empty bitmap cannot be used for color pattern teaching");

		return false;
	}

	iimg::CGeneralBitmap patternBitmap;

	i2d::CRectangle patternBoundingBox = m_bitmapRegionCompPtr->GetBoundingBox();
	const i2d::ICalibration2d* aoiCalibrationPtr = m_bitmapRegionCompPtr->GetCalibration();
	if (aoiCalibrationPtr != nullptr){
		if (!patternBoundingBox.Transform(*aoiCalibrationPtr)){
			SendErrorMessage(0, "Teaching region could not be defined");

			return false;
		}
	}

	if (!iipr::CBitmapOperations::ReduceBitmapToRegion(*bitmapPtr, patternBoundingBox, patternBitmap)){
		SendErrorMessage(0, "Teaching region could not be extracted from the bitmap");

		return false;
	}

	imeas::CSimpleSamplesSequence32 histogram;

	if (!m_histogramProgressorCompPtr->DoProcessing(m_paramsSetCompPtr.GetPtr(), bitmapPtr, &histogram)){
		SendErrorMessage(0, "Histogram calculation failed");

		return false;
	}

	istd::CChangeNotifier updatePtr(this);

	return m_patternBitmap.CopyFrom(patternBitmap) && m_histogram.CopyFrom(histogram);
}


void CColorPatternControllerComp::ResetPattern()
{
	istd::CChangeNotifier updatePtr(this);

	m_patternBitmap.ResetImage();
	m_histogram.ResetSequence();
}


bool CColorPatternControllerComp::IsPatternValid() const
{
	return !m_patternBitmap.IsEmpty() && !m_histogram.IsEmpty();
}


const iser::ISerializable* CColorPatternControllerComp::GetPatternObject() const
{
	return &m_histogram;
}


// reimplemented (iimg::IBitmapProvider)

const iimg::IBitmap* CColorPatternControllerComp::GetBitmap() const
{
	return &m_patternBitmap;
}


// reimplemented (imeas::IDataSequenceProvider)

const imeas::IDataSequence* CColorPatternControllerComp::GetDataSequence() const
{
	return &m_histogram;
}


// reimplemented (iser::ISerializable)

bool CColorPatternControllerComp::Serialize(iser::IArchive& archive)
{
	static iser::CArchiveTag patternBitmapTag("PatternBitmap", "Bitmap used as pattern", iser::CArchiveTag::TT_GROUP);
	static iser::CArchiveTag histogramTag("Histogram", "Histogram of the pattern bitmap", iser::CArchiveTag::TT_GROUP);

	istd::CChangeNotifier notifier(archive.IsStoring() ? nullptr : this);
	Q_UNUSED(notifier);

	bool retVal = true;

	retVal = retVal && archive.BeginTag(patternBitmapTag);
	retVal = retVal && m_patternBitmap.Serialize(archive);
	retVal = retVal && archive.EndTag(patternBitmapTag);		

	retVal = retVal && archive.BeginTag(histogramTag);		
	retVal = retVal && m_histogram.Serialize(archive);
	retVal = retVal && archive.EndTag(histogramTag);		

	return retVal;
}


// reimplemented (istd::IChangeable)

bool CColorPatternControllerComp::CopyFrom(const IChangeable& object, CompatibilityMode mode)
{
	const CColorPatternControllerComp* sourcePtr = dynamic_cast<const CColorPatternControllerComp*>(&object);
	if (sourcePtr != nullptr){
		istd::CChangeNotifier notifier(this);

		bool retVal = m_patternBitmap.CopyFrom(sourcePtr->m_patternBitmap, mode);
		retVal = m_histogram.CopyFrom(sourcePtr->m_histogram, mode) && retVal;

		return retVal;
	}

	return false;
}


} // namespace iipr


