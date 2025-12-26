#ifndef iipr_CHoughLineFinderComp_included
#define iipr_CHoughLineFinderComp_included


// ACF includes
#include <icomp/CComponentBase.h>
#include <ilog/IMessageConsumer.h>
#include <ilog/TLoggerCompWrap.h>
#include <i2d/CLine2d.h>

// ACF-Solutions includes
#include <iproc/TSyncProcessorWrap.h>
#include <iipr/IImageToFeatureProcessor.h>
#include <iipr/TWeightedFeatureWrap.h>
#include <ialgo/CHoughSpace2d.h>


namespace iipr
{


/**
	Processor using Hough transformation to find the lines on the image.
*/
class CHoughLineFinderComp:
			public ilog::CLoggerComponentBase,
			public iproc::TSyncProcessorWrap<IImageToFeatureProcessor>
{
public:
	typedef ilog::CLoggerComponentBase BaseClass;

	I_BEGIN_COMPONENT(CHoughLineFinderComp);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_REGISTER_INTERFACE(IImageToFeatureProcessor);
		I_ASSIGN(m_aoiParamIdAttrPtr, "AoiParamId", "ID of AOI in parameter set", false, "Aoi");
		I_ASSIGN(m_defaultAngleResAttrPtr, "DefaultAngleResulution", "Resolution of angle grid", true, 360);
		I_ASSIGN(m_defaultRadiusResAttrPtr, "DefaultRadiusResulution", "Resolution of radius grid", true, 200);
		I_ASSIGN(m_defaultMaxLinesAttrPtr, "DefaultMaxLines", "Number of maximal found lines", true, 100);
		I_ASSIGN(m_defaultSmoothKernelAttrPtr, "DefaultSpaceSmoothKernel", "Size of kernel will be used to smooth hough space", true, 10);
		I_ASSIGN(m_defaultMinEdgeStrengthAttrPtr, "DefaultMinEdgeStrength", "Minimal strength of edge to be considered", true, 0.01);
		I_ASSIGN(m_defaultAoiCompPtr, "DefaultAoi", "Area of interest used if not specified in parameters", false, "DefaultAoi");
		I_ASSIGN(m_resultConsumerCompPtr, "ResultConsumer", "Consumer of result messages with geometrical layout", false, "ResultConsumer");
		I_ASSIGN(m_tempConsumerCompPtr, "TempConsumer", "Consumer of temporary result messages with geometrical layout", false, "TempConsumer");
	I_END_COMPONENT;

	enum MessageId
	{
		FOUND_LINE = 0x934d740,
		HOUGH_SPACE
	};

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
	bool CreateHoughSpace();
	void UpdateHoughSpace(const i2d::CVector2d& position, const i2d::CVector2d& direction, double radiusOffset);

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	typedef TWeightedFeatureWrap<i2d::CLine2d> LineFeature;

	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);
	I_ATTR(int, m_defaultAngleResAttrPtr);
	I_ATTR(int, m_defaultRadiusResAttrPtr);
	I_ATTR(int, m_defaultMaxLinesAttrPtr);
	I_ATTR(int, m_defaultSmoothKernelAttrPtr);
	I_ATTR(double, m_defaultMinEdgeStrengthAttrPtr);
	I_REF(i2d::IObject2d, m_defaultAoiCompPtr);
	I_REF(ilog::IMessageConsumer, m_resultConsumerCompPtr);
	I_REF(ilog::IMessageConsumer, m_tempConsumerCompPtr);

	ialgo::CHoughSpace2d m_houghSpace;

	typedef QVector<i2d::CVector2d> AngleVectors;
	AngleVectors m_angleVectors;
};


} // namespace iipr


#endif // !iipr_CHoughLineFinderComp_included


