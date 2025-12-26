#ifndef iipr_CLensCorrFindSupplierComp_included
#define iipr_CLensCorrFindSupplierComp_included


// ACF includes
#include <imath/CFixedPointManip.h>
#include <imath/CGeneralUnitInfo.h>
#include <i2d/ICalibrationProvider.h>
#include <iprm/IEnableableParam.h>
#include <iimg/IBitmapProvider.h>

// ACF-Solutions includes
#include <imeas/INumericValue.h>
#include <imeas/CGeneralNumericConstraints.h>
#include <iproc/IProcessor.h>
#include <iinsp/TSupplierCompWrap.h>
#include <icalib/CSimpleLensCorrection.h>
#include <iipr/IImageToFeatureProcessor.h>
#include <ialgo/CHoughSpace2d.h>


namespace iipr
{


/**
	Supplier of bitmap built on difference of two images.
*/
class CLensCorrFindSupplierComp:
			public iinsp::TSupplierCompWrap< icalib::CSimpleLensCorrection >,
			virtual public i2d::ICalibrationProvider
{
public:
	typedef iinsp::TSupplierCompWrap<icalib::CSimpleLensCorrection> BaseClass;

	I_BEGIN_COMPONENT(CLensCorrFindSupplierComp);
		I_REGISTER_INTERFACE(i2d::ICalibrationProvider);
		I_REGISTER_SUBELEMENT(SearchParamsConstraints);
		I_REGISTER_SUBELEMENT_INTERFACE(SearchParamsConstraints, imeas::INumericConstraints, ExtractSearchParamsConstraints);
		I_REGISTER_SUBELEMENT_INTERFACE(SearchParamsConstraints, istd::IChangeable, ExtractSearchParamsConstraints);
		I_ASSIGN(m_bitmapProviderCompPtr, "BitmapProvider", "Provide input image", true, "BitmapProvider");
		I_ASSIGN_TO(m_bitmapProviderModelCompPtr, m_bitmapProviderCompPtr, false);
		I_ASSIGN_TO(m_bitmapProviderSupplierCompPtr, m_bitmapProviderCompPtr, false);
		I_ASSIGN(m_pointFinderCompPtr, "PointFinder", "Finder of points on image", true, "PointFinder");
		I_ASSIGN(m_gridSearchParamsIdAttrPtr, "GridSearchParamsId", "ID of general search parameters (imeas::INumericValue)", false, "GridSearchParams");
		I_ASSIGN(m_defaultGridSearchParamsCompPtr, "DefaultGridSearchParams", "Default search parameters as defined in subelement DefaultSearchParamsConstraints", false, "DefaultGridSearchParams");
		I_ASSIGN(m_smoothKernelAttrPtr, "SpaceSmoothKernel", "Size of kernel will be used to smooth hough space", true, 10);
		I_ASSIGN(m_searchCenterParamIdAttrPtr, "SearchCenterParamId", "ID of parameter to enable search of center (iprm::IEnableableParam)", false, "SearchCenter");
		I_ASSIGN(m_defaultSearchCenterParamCompPtr, "DefaultSearchCenterParam", "Default parameter enabling search of center", false, "DefaultSearchCenterParam");
	I_END_COMPONENT;

	// reimplemented (i2d::ICalibrationProvider)
	virtual const i2d::ICalibration2d* GetCalibration() const override;

protected:
	struct FeatureInfo
	{
		double weight;
		i2d::CVector2d position;
	};

	typedef QList<FeatureInfo> Features;

	class FeaturesConsumer: virtual public iipr::IFeaturesConsumer
	{
	public:
		// reimplemented (iipr::IFeaturesConsumer)
		virtual void ResetFeatures() override;
		virtual bool AddFeature(const imeas::INumericValue* featurePtr, bool* isFullPtr = nullptr) override;
	
		Features features;
	};

	struct CorrectionInfo
	{
		i2d::CVector2d foundPos;
		i2d::CVector2d onLinePos;
	};
	typedef QList<CorrectionInfo> CorrectionInfos;

	struct CorrectionLineInfo
	{
		CorrectionInfos infos;
		i2d::CVector2d orthoVector;
	};
	typedef QList<CorrectionLineInfo> AllCorrectionInfos;

	bool CalculateCalibrationFactors(
				const AllCorrectionInfos& allCorrectionInfos,
				bool searchCenterFlag,
				const icalib::CSimpleLensCorrection& inputCorrection,
				icalib::CSimpleLensCorrection& result) const;

	void AddCorrectPointsMessage(const AllCorrectionInfos& allCorrectionInfos, const icalib::CSimpleLensCorrection& inputCorrection, const i2d::CVector2d& imageCenter) const;

	bool CalculateCalibration(const iimg::IBitmap& image, icalib::CSimpleLensCorrection& result) const;
	i2d::CVector2d CalcHoughPos(const i2d::CVector2d& point1, const i2d::CVector2d& point2, const i2d::CVector2d& imageCenter, const istd::CIndex2d& spaceSize) const;
	i2d::CLine2d CalcCorrespondingLine(const i2d::CVector2d& houghPos, const i2d::CVector2d& imageCenter, const istd::CIndex2d& spaceSize) const;
	void UpdateHoughSpace(const i2d::CVector2d& point1, const i2d::CVector2d& point2, double weight, const i2d::CVector2d& imageCenter, ialgo::CHoughSpace2d& space) const;

	// reimplemented (iinsp::TSupplierCompWrap)
	virtual iinsp::ISupplier::WorkStatus ProduceObject(ProductType& result) const;

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	// static template methods for sub element access
	template <class InterfaceType>
	static InterfaceType* ExtractSearchParamsConstraints(CLensCorrFindSupplierComp& component)
	{
		return &component.m_searchParamsContraints;
	}

	I_REF(iimg::IBitmapProvider, m_bitmapProviderCompPtr);
	I_REF(imod::IModel, m_bitmapProviderModelCompPtr);
	I_REF(iinsp::ISupplier, m_bitmapProviderSupplierCompPtr);
	I_REF(iipr::IImageToFeatureProcessor, m_pointFinderCompPtr);
	I_ATTR(QByteArray, m_gridSearchParamsIdAttrPtr);
	I_REF(imeas::INumericValue, m_defaultGridSearchParamsCompPtr);
	I_ATTR(int, m_smoothKernelAttrPtr);
	I_ATTR(QByteArray, m_searchCenterParamIdAttrPtr);
	I_REF(iprm::IEnableableParam, m_defaultSearchCenterParamCompPtr);

	imeas::CGeneralNumericConstraints m_searchParamsContraints;

	static imath::CFixedPointManip s_integerManip;
	static imath::CGeneralUnitInfo s_angleGridUnitInfo;
	static imath::CGeneralUnitInfo s_distanceGridUnitInfo;
	static imath::CGeneralUnitInfo s_maxLinesUnitInfo;
	static imath::CGeneralUnitInfo s_minDistanceUnitInfo;
};


} // namespace iipr


#endif // !iipr_CLensCorrFindSupplierComp_included


