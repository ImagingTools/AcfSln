// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iblob_CBlobProcessorCompBase_included
#define iblob_CBlobProcessorCompBase_included


// ACF includes
#include <iprm/COptionsManager.h>
#include <iproc/TSyncProcessorCompBase.h>

// ACF-Solutions includes
#include <iipr/IImageToFeatureProcessor.h>
#include <iblob/IBlobFilterParams.h>


namespace iblob
{


class CBlobProcessorCompBase: public iproc::TSyncProcessorCompBase<iipr::IImageToFeatureProcessor>
{
public:
	typedef iproc::TSyncProcessorCompBase<iipr::IImageToFeatureProcessor> BaseClass;

	I_BEGIN_BASE_COMPONENT(CBlobProcessorCompBase);
		I_REGISTER_SUBELEMENT(FilterFeatureList);
		I_REGISTER_SUBELEMENT_INTERFACE(FilterFeatureList, iprm::IOptionsList, ExtractFilterFeatureList);
		I_REGISTER_SUBELEMENT_INTERFACE(FilterFeatureList, istd::IChangeable, ExtractFilterFeatureList);
		I_ASSIGN(m_aoiParamIdAttrPtr, "AoiParamId", "ID of area of interest in parameter set", false, "AoiParams");
		I_ASSIGN(m_defaultAoiCompPtr, "DefaultAoi", "Area of interest used if not specified in parameters", false, "DefaultAoi");
		I_ASSIGN(m_filterParamsIdAttrPtr, "FilterParamsId", "ID of blob filter parameters in the parameter set (iblob::IBlobFilterParams)", false, "FilterParams");
		I_ASSIGN(m_defaultFilterParamsCompPtr, "DefaultFilterParams", "Default blob filter parameters", false, "DefaultFilterParams");
	I_END_COMPONENT;

	// reimplemented (iipr::IImageToFeatureProcessor)
	virtual iproc::IProcessor::TaskState DoExtractFeatures(
				const iprm::IParamsSet* paramsPtr,
				const iimg::IBitmap& image,
				iipr::IFeaturesConsumer& results,
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
	static bool IsBlobAcceptedByFilter(const iblob::IBlobFilterParams& filterParams, double area, double perimeter, double circularity);

	// abstract methods
	virtual bool CalculateBlobs(
				const iprm::IParamsSet* paramsPtr,
				const iblob::IBlobFilterParams* filterParamsPtr,
				const i2d::IObject2d* aoiPtr,
				const iimg::IBitmap& image,
				iipr::IFeaturesConsumer& result) = 0;

private:
	class FilterFeatureList: public iprm::COptionsManager
	{
	public:
		FilterFeatureList();
	};

	// static template methods for sub-element access
	template <class InterfaceType>
	static InterfaceType* ExtractFilterFeatureList(CBlobProcessorCompBase& component)
	{
		return &component.m_filterFeatureList;
	}

private:
	I_ATTR(QByteArray, m_aoiParamIdAttrPtr);
	I_REF(i2d::IObject2d, m_defaultAoiCompPtr);

	I_ATTR(QByteArray, m_filterParamsIdAttrPtr);
	I_REF(iblob::IBlobFilterParams, m_defaultFilterParamsCompPtr);

	FilterFeatureList m_filterFeatureList;
};


} // namespace iblob


#endif // !iblob_CBlobProcessorCompBase_included


