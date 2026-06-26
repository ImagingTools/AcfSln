// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CImageRegionProcessorCompBase_included
#define iipr_CImageRegionProcessorCompBase_included


// ACF includes
#include <i2d/ICalibrationProvider.h>
#include <iproc/TSyncProcessorCompBase.h>


namespace i2d
{
	class IObject2d;
}


namespace iimg
{
	class IBitmap;
}


namespace iipr
{


/**
	Basic implementation for a image region processor.
*/
class CImageRegionProcessorCompBase: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;

	I_BEGIN_BASE_COMPONENT(CImageRegionProcessorCompBase);
		I_ASSIGN(m_aoiParamIdAttrPtr, "AoiParamId", "ID of the AOI region in the parameter set (i2d::IObject2d)", true, "AoiParams");
		I_ASSIGN(m_defaultAoiCompPtr, "DefaultAoi", "Area of interest used if not specified in parameters", false, "DefaultAoi");
		I_ASSIGN(m_allowEmptyRegionAttrPtr, "AllowEmptyAoi", "If enabled the full image area is used of no AOI was set", true, false);
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

protected:
	// abstract methods
	/**
		Process the defined image region.
	*/
	virtual bool ProcessImageRegion(
				const iimg::IBitmap& inputBitmap,
				const iprm::IParamsSet* paramsPtr,
				const i2d::IObject2d* aoiPtr,
				istd::IChangeable* outputPtr) const = 0;

protected:
	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);
	I_REF(i2d::IObject2d, m_defaultAoiCompPtr);
	I_ATTR(bool, m_allowEmptyRegionAttrPtr);
};


} // namespace iipr


#endif // !iipr_CImageRegionProcessorCompBase_included


