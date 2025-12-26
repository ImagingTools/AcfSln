#ifndef iipr_CFeatureToImageMapperProcessorComp_included
#define iipr_CFeatureToImageMapperProcessorComp_included


// ACF includes
#include <i2d/CPosition2d.h>
#include <icomp/CComponentBase.h>
#include <iproc/TSyncProcessorWrap.h>

#include <iipr/IFeatureToFeatureProcessor.h>
#include <iipr/IFeatureToImageMapper.h>
#include <iipr/TWeightedFeatureWrap.h>


namespace iipr
{


/**
	Processor converting feature container into set of positions using image position mapper.
	It can be used to achive set of image positions from set of caliper results.
*/
class CFeatureToImageMapperProcessorComp:
			public icomp::CComponentBase,
			public iproc::TSyncProcessorWrap<IFeatureToFeatureProcessor>
{
public:
	typedef icomp::CComponentBase BaseClass;

	I_BEGIN_COMPONENT(CFeatureToImageMapperProcessorComp);
		I_REGISTER_INTERFACE(IFeatureToFeatureProcessor);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_ASSIGN(m_featuresMapperCompPtr, "FeaturesMapper", "Calculate position from caliper extracted features", true, "FeaturesMapper");
		I_ASSIGN(m_ignoreNotMappableAttrPtr, "IgnoreNonMappable", "Enable ignoring of non-mappable features, if it is false and some feature cannot be mapped, error occures", true, false);
	I_END_COMPONENT;

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
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

protected:
	typedef TWeightedFeatureWrap<i2d::CPosition2d> PositionFeature;

private:
	I_REF(iipr::IFeatureToImageMapper, m_featuresMapperCompPtr);
	I_ATTR(bool, m_ignoreNotMappableAttrPtr);
};


} // namespace iipr


#endif // !iipr_CFeatureToImageMapperProcessorComp_included


