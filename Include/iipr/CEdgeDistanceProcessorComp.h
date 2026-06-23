// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CEdgeDistanceProcessorComp_included
#define iipr_CEdgeDistanceProcessorComp_included


// ACF includes
#include <i2d/CCircle.h>
#include <i2d/CAnnulus.h>
#include <i2d/ICalibrationProvider.h>
#include <icomp/CComponentBase.h>
#include <iproc/TSyncProcessorWrap.h>

#include <iipr/IImageToFeatureProcessor.h>
#include <iipr/IFeaturesProvider.h>
#include <iipr/IFeatureToImageMapper.h>
#include <iipr/ICaliperParams.h>
#include <iipr/TWeightedFeatureWrap.h>


namespace iipr
{


/**
	Cliper based distance calculation.
*/
class CEdgeDistanceProcessorComp:
			public icomp::CComponentBase,
			public iproc::TSyncProcessorWrap<IImageToFeatureProcessor>
{
public:
	typedef icomp::CComponentBase BaseClass;

	I_BEGIN_COMPONENT(CEdgeDistanceProcessorComp);
		I_REGISTER_INTERFACE(IImageToFeatureProcessor);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_ASSIGN(m_slaveProcessorCompPtr, "SlaveEdgeProcessor", "Slave edge finder processor converting image to list of found edges", true, "SlaveEdgeProcessor");
		I_ASSIGN(m_featuresMapperCompPtr, "FeaturesMapper", "Calculate position from caliper extracted features", true, "FeaturesMapper");
		I_ASSIGN(m_aoiParamIdAttrPtr, "AoiParamId", "ID of area of interest in parameter set", true, "AoiParams");
		I_ASSIGN(m_slaveLineIdAttrPtr, "SlaveLineId", "ID of line parameter added by this processor to parameter set for slave edge processor", true, "Line");
		I_ASSIGN(m_slaveCaliperParamsIdAttrPtr, "SlaveCaliperParamsId", "ID of calipera parameters added by this processor to parameter set for slave edge processor", true, "Caliper");
	I_END_COMPONENT;

	// reimplemented (iipr::IImageToFeatureProcessor)
	virtual iproc::IProcessor::TaskState DoExtractFeatures(
				const iprm::IParamsSet* paramsPtr,
				const iimg::IBitmap& image,
				IFeaturesConsumer& results,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

protected:
	struct Point
	{
		double weight;
		i2d::CVector2d position;

		Point()
			:weight(0.0),
			position(0, 0)
		{
		}
	};

	struct CaliperLine
	{
		Point points[2];
	};

	typedef std::vector<CaliperLine> CaliperLines;

	bool CalculateCaliperLines(
				const istd::IChangeable& aoiObject,
				const iprm::IParamsSet& params,
				const iimg::IBitmap& image,
				CaliperLines& rays,
				i2d::CLine2d& projectionLine,
				i2d::CVector2d& center);

	bool CalculateCaliper(
				const iprm::IParamsSet& params,
				iipr::ICaliperParams& workingCaliperParams,
				ICaliperParams::DirectionMode caliperDirectionMode,
				const iimg::IBitmap& image,
				CaliperLine& ray) const;

	void SetCaliperResults(
				const iprm::IParamsSet& params,
				const IFeaturesProvider& container,
				ICaliperParams::DirectionMode caliperDirectionMode,
				CaliperLine& ray) const;

private:
	I_REF(iproc::IProcessor, m_slaveProcessorCompPtr);
	I_REF(iipr::IFeatureToImageMapper, m_featuresMapperCompPtr);
	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);
	I_ATTR(QByteArray, m_slaveLineIdAttrPtr);
	I_ATTR(QByteArray, m_slaveCaliperParamsIdAttrPtr);
};


} // namespace iipr


#endif // !iipr_CEdgeDistanceProcessorComp_included


