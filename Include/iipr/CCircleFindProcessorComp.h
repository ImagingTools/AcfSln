#ifndef iipr_CCircleFindProcessorComp_included
#define iipr_CCircleFindProcessorComp_included


// ACF includes
#include <ilog/IMessageConsumer.h>
#include <i2d/CCircle.h>
#include <i2d/CAnnulus.h>
#include <i2d/CPolypoint.h>
#include <i2d/ICalibrationProvider.h>

#include <iproc/TSyncProcessorCompBase.h>
#include <iipr/IImageToFeatureProcessor.h>
#include <iipr/IFeaturesProvider.h>
#include <iipr/IFeatureToImageMapper.h>
#include <iipr/ICircleFinderParams.h>
#include <iipr/TWeightedFeatureWrap.h>


namespace iipr
{


/**
	Caliper based circle finder.
	This is realized as processor, as input the image, as output consumer of features must be given.
	It uses set of 1D caliper lines to find the position and radius of circle.
	It takes area of interests (AOI) from parameter set. As AOI can be used annulus, segment of annulus, rectangle and set of segments of annulus.
*/
class CCircleFindProcessorComp: public iproc::TSyncProcessorCompBase<IImageToFeatureProcessor>
{
public:
	typedef iproc::TSyncProcessorCompBase<IImageToFeatureProcessor> BaseClass;

	I_BEGIN_COMPONENT(CCircleFindProcessorComp);
		I_REGISTER_INTERFACE(IImageToFeatureProcessor);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_ASSIGN(m_slaveProcessorCompPtr, "SlaveEdgeProcessor", "Slave edge finder processor converting image to list of found edges", true, "SlaveEdgeProcessor");
		I_ASSIGN(m_featuresMapperCompPtr, "FeaturesMapper", "Calculate position from caliper extracted features", true, "FeaturesMapper");
		I_ASSIGN(m_resultCalibrationCompPtr, "ResultCalibration", "Calibration should be used for result", false, "ResultCalibration");
		I_ASSIGN(m_aoiParamIdAttrPtr, "AoiParamId", "ID of area of interest in parameter set", true, "AoiParams");
		I_ASSIGN(m_circleFinderParamsIdAttrPtr, "CircleFinderParamsId", "ID cirlcie finder parameters in parameter set", true, "CircleFinderParams");
		I_ASSIGN(m_slaveLineIdAttrPtr, "SlaveLineId", "ID of line parameter added by this processor to parameter set for slave edge processor", true, "LineParam");
		I_ASSIGN(m_searchForAnnulusAttrPtr, "SearchForAnnulus", "If it is true, annulus will be searched", true, false);
		I_ASSIGN(m_sendUsedPointsToTempAttrPtr, "SendUsedPointsToTemp", "If true, the used point positions will be send to temporary results", true, false);
		I_ASSIGN(m_sendLinesToTempAttrPtr, "SendLinesToTemp", "If true, the caliper lines will be send to temporary results", true, false);
		I_ASSIGN(m_resultConsumerCompPtr, "ResultConsumer", "Consumer of result messages with geometrical layout", false, "ResultConsumer");
		I_ASSIGN(m_tempConsumerCompPtr, "TempConsumer", "Consumer of temporary result messages", false, "TempResultsConsumer");
	I_END_COMPONENT;

	CCircleFindProcessorComp();

	// reimplemented (iipr::IImageToFeatureProcessor)
	virtual iproc::IProcessor::TaskState DoExtractFeatures(
				const iprm::IParamsSet* paramsPtr,
				const iimg::IBitmap& image,
				IFeaturesConsumer& results,
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

protected:
	typedef TWeightedFeatureWrap<i2d::CCircle> CircleFeature;
	typedef TWeightedFeatureWrap<i2d::CAnnulus> AnnulusFeature;

	struct Point
	{
		double weight;
		i2d::CVector2d position;
		bool wasChecked;
	};

	typedef std::vector<Point> Points;

	struct Ray
	{
		Ray()
		{
			usedIndex = -1;
		}

		i2d::CLine2d projectionLine;

		Points points;
		int usedIndex;
	};

	typedef std::vector<Ray> Rays;

	bool AddAoiToRays(
				const istd::IChangeable& aoiObject,
				const iprm::IParamsSet& params,
				const iimg::IBitmap& image,
				const iipr::ICircleFinderParams& circleFinderParams,
				Rays& inRays,
				Rays& outRays,
				i2d::CLine2d& projectionLine,
				i2d::CVector2d& center,
				int& totalRaysCount);
	bool CalculateCircle(
				const i2d::CVector2d& center,
				bool removeOutliers,
				double minOutliersDistance,
				Rays& rays,
				int totalRaysCount,
				CircleFeature& result);
	bool CalculateAnnulus(
				const i2d::CVector2d& center,
				Rays& inRays,
				Rays& outRays,
				int totalRaysCount,
				AnnulusFeature& result);

	void AddProjectionResultsToRays(
				const i2d::CLine2d& projectionLine,
				const iprm::IParamsSet& params,
				const IFeaturesProvider& container,
				Rays& inRays,
				Rays& outRays);

	void AddIntermediateResults(Rays& outRays);

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	I_REF(iproc::IProcessor, m_slaveProcessorCompPtr);
	I_REF(iipr::IFeatureToImageMapper, m_featuresMapperCompPtr);
	I_REF(i2d::ICalibration2d, m_resultCalibrationCompPtr);
	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);
	I_ATTR(QByteArray, m_slaveLineIdAttrPtr);
	I_ATTR(QByteArray, m_circleFinderParamsIdAttrPtr);
	I_ATTR(bool, m_searchForAnnulusAttrPtr);
	I_ATTR(bool, m_sendUsedPointsToTempAttrPtr);
	I_ATTR(bool, m_sendLinesToTempAttrPtr);
	I_REF(ilog::IMessageConsumer, m_resultConsumerCompPtr);
	I_REF(ilog::IMessageConsumer, m_tempConsumerCompPtr);
};


} // namespace iipr


#endif // !iipr_CCircleFindProcessorComp_included


