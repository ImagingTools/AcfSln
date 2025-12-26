#include <iipr/CSearchParams.h>


// ACF includes
#include <istd/CChangeNotifier.h>
#include <iser/IArchive.h>
#include <iser/CArchiveTag.h>
#include <iser/CPrimitiveTypesSerializer.h>


namespace iipr
{


CSearchParams::CSearchParams()
	:m_angleRange(-180.0, 180.0),
	m_scaleRange(0.5, 2.0),
	m_nominalModelsCount(-1),
	m_minScore(0.6),
	m_isRotationEnabled(false),
	m_isScaleEnabled(false)
{
}


// reimplemented (iipr::ISearchParams)

const ISearchConstraints* CSearchParams::GetSearchConstraints() const
{
	return nullptr;
}


double CSearchParams::GetMinScore() const
{
	return m_minScore;
}


void CSearchParams::SetMinScore(double minScore)
{
	if (m_minScore != minScore){
		istd::CChangeNotifier changeNotifier(this);

		m_minScore = minScore;
	}
}


const istd::CRange& CSearchParams::GetRotationRange() const
{
	return m_angleRange;
}


void CSearchParams::SetRotationRange(const istd::CRange& angleRange)
{
	if (m_angleRange != angleRange){
		istd::CChangeNotifier changeNotifier(this);

		m_angleRange = angleRange;
	}
}


const istd::CRange& CSearchParams::GetScaleRange() const
{
	return m_scaleRange;
}


void CSearchParams::SetScaleRange(const istd::CRange& scaleRange)
{
	if (m_scaleRange != scaleRange){
		istd::CChangeNotifier changeNotifier(this);

		m_scaleRange = scaleRange;
	}
}


int CSearchParams::GetNominalModelsCount() const
{
	return m_nominalModelsCount;
}


void CSearchParams::SetNominalModelsCount(int nominalModelsCount)
{
	if (m_nominalModelsCount != nominalModelsCount){
		istd::CChangeNotifier changeNotifier(this);

		m_nominalModelsCount = nominalModelsCount;
	}
}


bool CSearchParams::IsRotationEnabled() const 
{
	return m_isRotationEnabled;
}


void CSearchParams::SetRotationEnabled(bool isRotationEnabled)
{
	if (m_isRotationEnabled != isRotationEnabled){
		istd::CChangeNotifier notifierPtr(this);

		m_isRotationEnabled = isRotationEnabled;
	}
}


bool CSearchParams::IsScaleEnabled() const
{
	return m_isScaleEnabled;
}


void CSearchParams::SetScaleEnabled(bool isScaleEnabled)
{
	if (m_isScaleEnabled != isScaleEnabled){
		istd::CChangeNotifier notifierPtr(this);

		m_isScaleEnabled = isScaleEnabled;
	}
}


// reimplemented (iser::ISerializable)

bool CSearchParams::Serialize(iser::IArchive & archive)
{
	static iser::CArchiveTag angleRangeTag("AngleRange", "Search angle range", iser::CArchiveTag::TT_GROUP);
	static iser::CArchiveTag scaleRangeTag("ScaleRange", "Search scale range", iser::CArchiveTag::TT_GROUP);
	static iser::CArchiveTag minScoreTag("MinScore", "Minimum match score", iser::CArchiveTag::TT_LEAF);
	static iser::CArchiveTag isRotationEnabledTag("RotationEnabled", "Is angle range enabled", iser::CArchiveTag::TT_LEAF);
	static iser::CArchiveTag isScaleEnabledTag("ScaleEnabledTag", "Is scale range enabled", iser::CArchiveTag::TT_LEAF);
	static iser::CArchiveTag nominalModelsCountTag("NominalModelsCount", "Minimum model matches count", iser::CArchiveTag::TT_LEAF);

	istd::CChangeNotifier notifier(archive.IsStoring()? nullptr: this);
	Q_UNUSED(notifier);

	bool retVal = archive.BeginTag(angleRangeTag);
	retVal = retVal && iser::CPrimitiveTypesSerializer::SerializeRange(archive, m_angleRange);
	retVal = retVal && archive.EndTag(angleRangeTag);

	retVal = retVal && archive.BeginTag(scaleRangeTag);
	retVal = retVal && iser::CPrimitiveTypesSerializer::SerializeRange(archive, m_scaleRange);
	retVal = retVal && archive.EndTag(scaleRangeTag);

	retVal = retVal && archive.BeginTag(minScoreTag);
	retVal = retVal && archive.Process(m_minScore);
	retVal = retVal && archive.EndTag(minScoreTag);

	retVal = retVal && archive.BeginTag(isRotationEnabledTag);
	retVal = retVal && archive.Process(m_isRotationEnabled);
	retVal = retVal && archive.EndTag(isRotationEnabledTag);

	retVal = retVal && archive.BeginTag(isScaleEnabledTag);
	retVal = retVal && archive.Process(m_isScaleEnabled);
	retVal = retVal && archive.EndTag(isScaleEnabledTag);

	retVal = retVal && archive.BeginTag(nominalModelsCountTag);
	retVal = retVal && archive.Process(m_nominalModelsCount);
	retVal = retVal && archive.EndTag(nominalModelsCountTag);

	return retVal;
}


// reimplemented (istd::IChangeable)

bool CSearchParams::CopyFrom(const istd::IChangeable& object, istd::IChangeable::CompatibilityMode /*mode*/)
{
	if (const CSearchParams* sourcePtr = dynamic_cast<const CSearchParams*>(&object))
	{
		istd::CChangeNotifier notifier(this);

		m_angleRange = sourcePtr->m_angleRange;
		m_scaleRange = sourcePtr->m_scaleRange;
		m_minScore = sourcePtr->m_minScore;
		m_isRotationEnabled = sourcePtr->m_isRotationEnabled;
		m_isScaleEnabled = sourcePtr->m_isScaleEnabled;
		m_nominalModelsCount = sourcePtr->m_nominalModelsCount;

		return true;
	}

	return false;
}


} // namespace iipr


