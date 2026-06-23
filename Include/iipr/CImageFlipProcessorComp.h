// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CImageFlipProcessorComp_included
#define iipr_CImageFlipProcessorComp_included


// ACF includes
#include <iproc/TSyncProcessorCompBase.h>

#include <iimg/IBitmap.h>


namespace iipr
{


class CImageFlipProcessorComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;

	I_BEGIN_COMPONENT(CImageFlipProcessorComp);
		I_ASSIGN(m_mirrorXParamIdAttrPtr, "MirrorXParamId", "Id used to get Mirror By X parameter", false, "MirrorXParamId");
		I_ASSIGN(m_mirrorYParamIdAttrPtr, "MirrorYParamId", "Id used to get Mirror By Y parameter", false, "MirrorYParamId");
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
		const iprm::IParamsSet* paramsPtr,
		const istd::IPolymorphic* inputPtr,
		istd::IChangeable* outputPtr,
		ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

protected:
	bool ConvertImage(
		const iimg::IBitmap& inputBitmap,
		iimg::IBitmap& outputBitmap,
		bool mirrorX, bool mirrorY) const;

private:
	I_ATTR(QByteArray, m_mirrorXParamIdAttrPtr);
	I_ATTR(QByteArray, m_mirrorYParamIdAttrPtr);
};


} // namespace iipr


#endif // !iipr_CImageFlipProcessorComp_included
