#ifndef iipr_CCheckboardCalibSupplierComp_included
#define iipr_CCheckboardCalibSupplierComp_included


// ACF includes
#include <imath/CFixedPointManip.h>
#include <imath/CGeneralUnitInfo.h>
#include <i2d/ICalibrationProvider.h>
#include <i2d/CLine2d.h>
#include <iimg/IBitmapProvider.h>

// ACF-Solutions includes
#include <imeas/INumericValue.h>
#include <imeas/CGeneralNumericConstraints.h>
#include <iproc/IProcessor.h>
#include <iinsp/TSupplierCompWrap.h>
#include <icalib/CPerspectiveCalibration2d.h>
#include <iipr/IImageToFeatureProcessor.h>


namespace iipr
{


/**
	\deprecated Please use CPerspectiveCalibrationSupplierComp in combination with CCheckerboardPointGridExtractorComp!
*/
class CCheckboardCalibSupplierComp:
			public iinsp::TSupplierCompWrap< icalib::CPerspectiveCalibration2d >,
			virtual public i2d::ICalibrationProvider
{
public:
	typedef iinsp::TSupplierCompWrap<icalib::CPerspectiveCalibration2d> BaseClass;

	I_BEGIN_COMPONENT(CCheckboardCalibSupplierComp);
		I_REGISTER_INTERFACE(i2d::ICalibrationProvider);
		I_REGISTER_SUBELEMENT(CheckboardParamContraints);
		I_REGISTER_SUBELEMENT_INTERFACE(CheckboardParamContraints, imeas::INumericConstraints, ExtractCheckboardParamContraints);
		I_REGISTER_SUBELEMENT_INTERFACE(CheckboardParamContraints, istd::IChangeable, ExtractCheckboardParamContraints);
		I_ASSIGN(m_bitmapProviderCompPtr, "BitmapProvider", "Provide input image", true, "BitmapProvider");
		I_ASSIGN_TO(m_bitmapProviderModelCompPtr, m_bitmapProviderCompPtr, false);
		I_ASSIGN_TO(m_bitmapProviderSupplierCompPtr, m_bitmapProviderCompPtr, false);
		I_ASSIGN(m_lineFinderCompPtr, "LineFinder", "Finder of lines on image", true, "LineFinder");
		I_ASSIGN(m_checkboardParamsId, "CheckboardParamsId", "ID of checkboard parameters in parameter set (type imeas::INumericValue):\n1\t1. grid size\n\t2. cell size in logical units", false, "CheckboardParams");
		I_ASSIGN(m_defaultCheckboardParamsCompPtr, "DefaultCheckboardParams", "Default parameter controlling size of single checkboard cell when it is not defined in parameter set:\n1\t1. grid size\n\t2. cell size in logical units", false, "DefaultCheckboardParams");
		I_ASSIGN(m_axisXParamIdAttrPtr, "AxisXParamId", "ID of axis X template in parameter set (type i2d::CLine2d), it is used to coarse knowing direction of X axis", false, "AxisX");
		I_ASSIGN(m_defaultAxisXParamCompPtr, "DefaultAxisXParam", "Default parameter controlling axis X template, it is used to coarse knowing direction of X axis", false, "DefaultAxisXParam");
		I_ASSIGN(m_axisYParamIdAttrPtr, "AxisYParamId", "ID of axis Y template in parameter set (type i2d::CLine2d), it is used to coarse knowing direction of Y axis", false, "AxisY");
		I_ASSIGN(m_defaultAxisYParamCompPtr, "DefaultAxisYParam", "Default parameter controlling axis Y template, it is used to coarse knowing direction of Y axis", false, "DefaultAxisYParam");
		I_ASSIGN(m_distanceUnitInfoCompPtr, "DistanceUnitInfo", "Information about used distance units", false, "DistanceUnitInfo");
		I_ASSIGN(m_defaultGridSizeAttrPtr, "DefaultGridSize", "Default size of grid if no checkboard parameters specified", true, 8);
		I_ASSIGN(m_defaultCellSizeAttrPtr, "DefaultCellSize", "Default size of single cell (in logical units) if no checkboard parameters specified", true, 10);
	I_END_COMPONENT;

	CCheckboardCalibSupplierComp();

	// reimplemented (i2d::ICalibrationProvider)
	virtual const i2d::ICalibration2d* GetCalibration() const override;

protected:
	struct LineInfo
	{
		double weight;
		i2d::CLine2d line;
	};

	typedef QList<LineInfo> Lines;

	class LinesConsumer: virtual public iipr::IFeaturesConsumer
	{
	public:
		// reimplemented (iipr::IFeaturesConsumer)
		virtual void ResetFeatures() override;
		virtual bool AddFeature(const imeas::INumericValue* featurePtr, bool* isFullPtr = nullptr) override;
	
		Lines lines;
	};

	class ChessboardParamsContraints: public imeas::INumericConstraints, protected iprm::IOptionsList
	{
	public:
		ChessboardParamsContraints(const CCheckboardCalibSupplierComp* supplierPtr);

		// reimplemented (imeas::INumericConstraints)
		virtual const iprm::IOptionsList& GetValueListInfo() const override;
		virtual const imath::IUnitInfo* GetNumericValueUnitInfo(int index) const override;

	protected:
		// reimplemented (iprm::IOptionsList)
		virtual int GetOptionsFlags() const override;
		virtual int GetOptionsCount() const override;
		virtual QString GetOptionName(int index) const override;
		virtual QString GetOptionDescription(int index) const override;
		virtual QByteArray GetOptionId(int index) const override;
		virtual bool IsOptionEnabled(int index) const override;

	private:
		imath::CGeneralUnitInfo m_cellSizeUnit;
		const CCheckboardCalibSupplierComp* m_parentPtr;
	};

	bool CalculateCalibration(const iimg::IBitmap& image, icalib::CPerspectiveCalibration2d& result) const;

	// reimplemented (iinsp::TSupplierCompWrap)
	virtual iinsp::ISupplier::WorkStatus ProduceObject(ProductType& result) const override;

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	I_REF(iimg::IBitmapProvider, m_bitmapProviderCompPtr);
	I_REF(imod::IModel, m_bitmapProviderModelCompPtr);
	I_REF(iinsp::ISupplier, m_bitmapProviderSupplierCompPtr);
	I_REF(iipr::IImageToFeatureProcessor, m_lineFinderCompPtr);
	I_ATTR(QByteArray, m_checkboardParamsId);
	I_REF(imeas::INumericValue, m_defaultCheckboardParamsCompPtr);
	I_ATTR(QByteArray, m_axisXParamIdAttrPtr);
	I_REF(i2d::CLine2d, m_defaultAxisXParamCompPtr);
	I_ATTR(QByteArray, m_axisYParamIdAttrPtr);
	I_REF(i2d::CLine2d, m_defaultAxisYParamCompPtr);
	I_REF(imath::IUnitInfo, m_distanceUnitInfoCompPtr);
	I_ATTR(int, m_defaultGridSizeAttrPtr);
	I_ATTR(double, m_defaultCellSizeAttrPtr);

	// static template methods for subelement access
	template <class InterfaceType>
	static InterfaceType* ExtractCheckboardParamContraints(CCheckboardCalibSupplierComp& component)
	{
		return &component.m_checkboardParamContraints;
	}

	ChessboardParamsContraints m_checkboardParamContraints;

	static imath::CFixedPointManip s_gridSizeValueManip;
	static imath::CGeneralUnitInfo s_gridSizeUnit;
};


} // namespace iipr


#endif // !iipr_CCheckboardCalibSupplierComp_included


