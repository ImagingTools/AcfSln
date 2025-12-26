#include <iipr/CCircleFinderParams.h>


// ACF includes
#include <istd/CChangeNotifier.h>
#include <iser/IArchive.h>
#include <iser/CArchiveTag.h>
#include <imath/CGeneralUnitInfo.h>


namespace iipr
{


CCircleFinderParams::CCircleFinderParams()
:	m_isOutlierEliminationEnabled(false),
	m_minOutlierDistance(5),
	m_raysCount(-1),
	m_caliperMode(CCM_FIRST)
{
}


// reimplemented (iipr::ICircleFinderParams)

int CCircleFinderParams::GetCaliperMode() const
{
	return m_caliperMode;
}


void CCircleFinderParams::SetCaliperMode(int caliperMode)
{
	if (caliperMode != m_caliperMode){
		istd::CChangeNotifier notifier(this);

		m_caliperMode = caliperMode;
	}
}


bool CCircleFinderParams::IsOutlierEliminationEnabled() const
{
	return m_isOutlierEliminationEnabled;
}


void CCircleFinderParams::SetOutlierEliminationEnabled(bool isOutlierEliminationEnabled)
{
	if (isOutlierEliminationEnabled != m_isOutlierEliminationEnabled){
		istd::CChangeNotifier notifier(this);

		m_isOutlierEliminationEnabled = isOutlierEliminationEnabled;
	}
}


double CCircleFinderParams::GetMinOutlierDistance() const
{
	return m_minOutlierDistance;
}


void CCircleFinderParams::SetMinOutlierDistance(double minOutlierDistance)
{
	if (minOutlierDistance != m_minOutlierDistance){
		istd::CChangeNotifier notifier(this);

		m_minOutlierDistance = minOutlierDistance;
	}
}


const imath::IUnitInfo* CCircleFinderParams::GetDistanceUnitInfo() const
{
	return nullptr;
}


int CCircleFinderParams::GetRaysCount() const
{
	return m_raysCount;
}


void CCircleFinderParams::SetRaysCount(int raysCount)
{
	if (raysCount != m_raysCount){
		istd::CChangeNotifier notifier(this);

		m_raysCount = raysCount;
	}
}


// reimplemented (iser::ISerializable)

bool CCircleFinderParams::Serialize(iser::IArchive& archive)
{
	static iser::CArchiveTag caliperModeTag("CaliperMode", "Caliper mode", iser::CArchiveTag::TT_LEAF);
	static iser::CArchiveTag isOutlierEliminationEnabledTag("OutlierEliminationEnabled", "Outliers elimination is enabled", iser::CArchiveTag::TT_LEAF);
	static iser::CArchiveTag minOutlierDistanceTag("MinOutlierDistance", "Minimal outliers distance", iser::CArchiveTag::TT_LEAF);
	static iser::CArchiveTag raysCountTag("RaysCount", "Number of projection rays", iser::CArchiveTag::TT_LEAF);

	istd::CChangeNotifier notifier(archive.IsStoring()? nullptr: this);
	Q_UNUSED(notifier);

	bool retVal = true;

	retVal = retVal && archive.BeginTag(caliperModeTag);
	retVal = retVal && archive.Process(m_caliperMode);
	retVal = retVal && archive.EndTag(caliperModeTag);

	retVal = retVal && archive.BeginTag(isOutlierEliminationEnabledTag);
	retVal = retVal && archive.Process(m_isOutlierEliminationEnabled);
	retVal = retVal && archive.EndTag(isOutlierEliminationEnabledTag);

	retVal = retVal && archive.BeginTag(minOutlierDistanceTag);
	retVal = retVal && archive.Process(m_minOutlierDistance);
	retVal = retVal && archive.EndTag(minOutlierDistanceTag);

	retVal = retVal && archive.BeginTag(raysCountTag);
	retVal = retVal && archive.Process(m_raysCount);
	retVal = retVal && archive.EndTag(raysCountTag);

	return retVal;
}


} // namespace iipr

