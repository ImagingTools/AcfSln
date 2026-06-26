// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CHoughLineFromPosFinderComp_included
#define iipr_CHoughLineFromPosFinderComp_included


// ACF includes
#include <icomp/CComponentBase.h>
#include <ilog/IMessageConsumer.h>
#include <ilog/TLoggerCompWrap.h>
#include <i2d/CLine2d.h>
#include <i2d/CPolypoint.h>

// ACF-Solutions includes
#include <iproc/TSyncProcessorWrap.h>
#include <iipr/IImageToFeatureProcessor.h>
#include <iipr/TWeightedFeatureWrap.h>
#include <ialgo/CHoughSpace2d.h>
#include <iipr/IFeatureToFeatureProcessor.h>


namespace iipr
{


/**
	Finder using Hough transformation to find the lines from the point cloud.
	The outouted lines will be sorted from the best one (with the highest weight) to the worst.
*/
class CHoughLineFromPosFinderComp:
			public ilog::CLoggerComponentBase,
			public iproc::TSyncProcessorWrap<IFeatureToFeatureProcessor>
{
public:
	typedef QList<i2d::CLine2d> Lines;

	typedef ilog::CLoggerComponentBase BaseClass;

	I_BEGIN_COMPONENT(CHoughLineFromPosFinderComp);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_REGISTER_INTERFACE(IFeatureToFeatureProcessor);
		I_ASSIGN(m_defaultAngleResAttrPtr, "DefaultAngleResulution", "Resolution of angle grid", true, 360);
		I_ASSIGN(m_defaultRadiusResAttrPtr, "DefaultRadiusResulution", "Resolution of radius grid", true, 360);
		I_ASSIGN(m_maxLinesParamIdAttrPtr, "MaxLinesParamId", "ID of max lines parameter in parameter set (imeas::INumericValue)", true, "MaxLines");
		I_ASSIGN(m_defaultMaxLinesParamCompPtr, "DefaultMaxLinesParam", "Default value of parameter controlling number of maximal found lines", true, "DefaultMaxLinesParam");
		I_ASSIGN(m_directionParamIdAttrPtr, "DirectionParamId", "ID of direction parameter in parameter set (i2d::IObject2d - line or position)", true, "Direction");
		I_ASSIGN(m_defaultDirectionParamCompPtr, "DefaultDirectionParam", "Default value of parameter controlling direction", true, "DefaultDirectionParam");
		I_ASSIGN(m_defaultSmoothKernelAttrPtr, "DefaultSpaceSmoothKernel", "Size of kernel will be used to smooth hough space", true, 10);
		I_ASSIGN(m_resultConsumerCompPtr, "ResultConsumer", "Consumer of result messages with geometrical layout", false, "ResultConsumer");
		I_ASSIGN(m_tempConsumerCompPtr, "TempConsumer", "Consumer of temporary result messages with geometrical layout", false, "TempConsumer");
	I_END_COMPONENT;

	enum MessageId
	{
		FOUND_LINE = 0x934d750,
		HOUGH_SPACE
	};

	// reimplemented (iipr::IFeatureToFeatureProcessor)
	virtual iproc::IProcessor::TaskState DoConvertFeatures(
				const iprm::IParamsSet* paramsPtr,
				const IFeaturesProvider& container,
				IFeaturesConsumer& results) override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

protected:
	virtual int FindLines(
				const iprm::IParamsSet* paramsPtr,
				int maxLines,
				const i2d::CPolypoint& points,
				IFeaturesConsumer& results);

	bool CreateHoughSpace();
	i2d::CVector2d CalcHoughPos(const i2d::CVector2d& point1, const i2d::CVector2d& point2) const;
	i2d::CLine2d CalcCorrespondingLine(const i2d::CVector2d& houghPos) const;
	void UpdateHoughSpace(const i2d::CVector2d& point1, const i2d::CVector2d& point2);

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	typedef TWeightedFeatureWrap<i2d::CLine2d> LineFeature;

	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);
	I_ATTR(int, m_defaultAngleResAttrPtr);
	I_ATTR(int, m_defaultRadiusResAttrPtr);
	I_ATTR(QByteArray, m_maxLinesParamIdAttrPtr);
	I_REF(imeas::INumericValue, m_defaultMaxLinesParamCompPtr);
	I_ATTR(QByteArray, m_directionParamIdAttrPtr);
	I_REF(i2d::IObject2d, m_defaultDirectionParamCompPtr);
	I_ATTR(int, m_defaultSmoothKernelAttrPtr);
	I_REF(ilog::IMessageConsumer, m_resultConsumerCompPtr);
	I_REF(ilog::IMessageConsumer, m_tempConsumerCompPtr);

	ialgo::CHoughSpace2d m_houghSpace;
	i2d::CVector2d m_direction;
	double m_directionAngleTolerance;
};


} // namespace iipr


#endif // !iipr_CHoughLineFromPosFinderComp_included


