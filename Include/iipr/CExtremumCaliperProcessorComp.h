#ifndef iipr_CExtremumCaliperProcessorComp_included
#define iipr_CExtremumCaliperProcessorComp_included


// ACF includes
#include <QtCore/QString>
#include <icomp/CComponentBase.h>
#include <iproc/TSyncProcessorWrap.h>

#include <iipr/ICaliperProcessor.h>
#include <iipr/IFeatureToProjectionMapper.h>


namespace iipr
{


class CExtremumCaliperProcessorComp:
			public icomp::CComponentBase,
			public iproc::TSyncProcessorWrap<ICaliperProcessor>,
			virtual public IFeatureToProjectionMapper
{
public:
	typedef icomp::CComponentBase BaseClass;
	typedef iproc::TSyncProcessorWrap<ICaliperProcessor> BaseClass2;

	I_BEGIN_COMPONENT(CExtremumCaliperProcessorComp);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_REGISTER_INTERFACE(ICaliperProcessor);
		I_REGISTER_INTERFACE(IFeatureToProjectionMapper);
		I_ASSIGN(m_caliperParamsIdAttrPtr, "CaliperParamsId", "ID of caliper parameters in parameter set", true, "CaliperParams");
	I_END_COMPONENT;

	// reimplemented (iipr::ICaliperProcessor)
	virtual bool DoCaliper(
				const imeas::IDataSequence& derivative,
				const ICaliperParams* paramsPtr,
				IFeaturesConsumer& results) override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

	// reimplemented (iipr::IFeatureToProjectionMapper)
	virtual bool GetProjectionPosition(
				const imeas::INumericValue& feature,
				const iprm::IParamsSet* paramsPtr,
				double& result) const override;

private:
	I_ATTR(QByteArray, m_caliperParamsIdAttrPtr);
};


} // namespace iipr


#endif // !iipr_CExtremumCaliperProcessorComp_included


