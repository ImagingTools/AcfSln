#ifndef iipr_CRingPatternGeneratorComp_included
#define iipr_CRingPatternGeneratorComp_included


// ACF includes
#include <iimg/IBitmap.h>

#include <iproc/TSyncProcessorCompBase.h>


namespace iipr
{


class CRingPatternGeneratorComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;
	
	I_BEGIN_COMPONENT(CRingPatternGeneratorComp);
		I_ASSIGN(m_paramsIdAttrPtr, "ParamsId", "ID of the processor parameter in the parameter set", false, "ParamsId");
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = nullptr) override;

private:
	I_ATTR(QByteArray, m_paramsIdAttrPtr);
};


} // namespace iipr


#endif // !iipr_CRingPatternGeneratorComp_included

