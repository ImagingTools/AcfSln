// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CCheckerboardPatternGeneratorComp_included
#define iipr_CCheckerboardPatternGeneratorComp_included


// ACF includes
#include <iimg/IBitmap.h>

#include <iproc/TSyncProcessorCompBase.h>


namespace iipr
{


class CCheckerboardPatternGeneratorComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;
	
	I_BEGIN_COMPONENT(CCheckerboardPatternGeneratorComp);
		I_ASSIGN(m_paramsIdAttrPtr, "ParamsId", "ID of the processor parameter in the parameter set", false, "ParamsId");
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

private:
	I_ATTR(QByteArray, m_paramsIdAttrPtr);
};


} // namespace iipr


#endif // !iipr_CCheckerboardPatternGeneratorComp_included

