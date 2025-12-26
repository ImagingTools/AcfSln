#ifndef iipr_CProcessedBitmapSupplierComp_included
#define iipr_CProcessedBitmapSupplierComp_included


// ACF includes
#include <i2d/ICalibrationProvider.h>
#include <iimg/IBitmapProvider.h>
#include <icam/ICameraAcquisition.h>

// ACF-Solutions includes
#include <iproc/IProcessor.h>
#include <iinsp/ISupplier.h>
#include <iinsp/TSupplierCompWrap.h>


namespace iipr
{


/**
	Image supplier providing processed image from some other input image supplier.
	This implementation delegates the provided input calibration and has the role of the calibration provider.
	If the connected processor creates a new calibration, then she should be pushed into the 'OutputBitmapCalibration'. In this case it will be the provided calibration
*/
class CProcessedBitmapSupplierBase:
			public iinsp::TSupplierCompWrap<iimg::IBitmapSharedPtr>,
			virtual public iimg::IBitmapProvider,
			virtual public i2d::ICalibrationProvider,
			virtual public icam::ICameraAcquisition
{
public:
	typedef iinsp::TSupplierCompWrap<iimg::IBitmapSharedPtr> BaseClass;

	I_BEGIN_BASE_COMPONENT(CProcessedBitmapSupplierBase);
		I_REGISTER_INTERFACE(iimg::IBitmapProvider);
		I_REGISTER_INTERFACE(icam::ICameraAcquisition);
		I_REGISTER_INTERFACE(i2d::ICalibrationProvider);
		I_ASSIGN(m_bitmapProviderCompPtr, "BitmapProvider", "Provide input image", true, "BitmapProvider");
		I_ASSIGN_TO(m_bitmapSupplierCompPtr, m_bitmapProviderCompPtr, false);
		I_ASSIGN_TO(m_bitmapProviderModelCompPtr, m_bitmapProviderCompPtr, false);
		I_ASSIGN_TO(m_inputCameraAcquisitionCompPtr, m_bitmapProviderCompPtr, false);
		I_ASSIGN(m_inputBitmapCalibrationProviderCompPtr, "InputBitmapCalibrationProvider", "Provide the the calibration of the input image", false, "InputBitmapCalibrationProvider");
		I_ASSIGN_TO(m_inputBitmapCalibrationSupplierCompPtr, m_inputBitmapCalibrationProviderCompPtr, false);
		I_ASSIGN_TO(m_inputBitmapCalibrationProviderModelCompPtr, m_inputBitmapCalibrationProviderCompPtr, false);	
		I_ASSIGN(m_imageProcessorCompPtr, "BitmapProcessor", "Bitmap conversion processor (takes bitmap as input and output)", true, "BitmapProcessor");
		I_ASSIGN(m_outputBitmapCalibrationCompPtr, "OutputBitmapCalibration", "Resulting calibration of the processor output", false, "OutputBitmapCalibration");
	I_END_COMPONENT;

	const iimg::IBitmapProvider* GetBitmapProvider() const;

	// reimplemented (i2d::ICalibrationProvider)
	virtual const i2d::ICalibration2d* GetCalibration() const override;

	// reimplemented (iimg::IBitmapProvider)
	virtual const iimg::IBitmap* GetBitmap() const override;

protected:
	bool EnsureBitmapCreated(ProductType& result) const;

	/**
		Get access to the image processor used for process the input bitmap.
	*/
	virtual iproc::IProcessor* GetImageProcessor() const;

	// reimplemented (iinsp::TSupplierCompWrap)
	virtual iinsp::ISupplier::WorkStatus ProduceObject(ProductType& result) const override;

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;
	virtual void OnComponentDestroyed() override;

	// abstract methods
	virtual iimg::IBitmapUniquePtr CreateBitmap() const = 0;
	virtual int ProcessBitmapWithParameters(const iimg::IBitmap* inputBitmapPtr, iimg::IBitmap* outputBitmapPtr, const iprm::IParamsSet* parametersPtr, iproc::IProcessor* imageProcessorPtr = nullptr) const;

	// reimplemented (icam::ICameraAcquisition)
	virtual bool StartCamera() const override;
	virtual bool StopCamera() const override;
	virtual bool GetLastImage(iimg::IBitmap& snapBitmap) const override;

protected:
	I_REF(iimg::IBitmapProvider, m_bitmapProviderCompPtr);
	I_REF(iinsp::ISupplier, m_bitmapSupplierCompPtr);
	I_REF(imod::IModel, m_bitmapProviderModelCompPtr);
	I_REF(icam::ICameraAcquisition, m_inputCameraAcquisitionCompPtr);

	I_REF(i2d::ICalibrationProvider, m_inputBitmapCalibrationProviderCompPtr);
	I_REF(iinsp::ISupplier, m_inputBitmapCalibrationSupplierCompPtr);
	I_REF(imod::IModel, m_inputBitmapCalibrationProviderModelCompPtr);
	
	I_REF(i2d::ICalibration2d, m_outputBitmapCalibrationCompPtr);
	I_REF(iproc::IProcessor, m_imageProcessorCompPtr);
};


class CProcessedBitmapSupplierComp: public CProcessedBitmapSupplierBase
{
public:
	typedef CProcessedBitmapSupplierBase BaseClass;

	I_BEGIN_COMPONENT(CProcessedBitmapSupplierComp);
		I_ASSIGN(m_bitmapCompFact, "BitmapFactory", "Use to create bitmap object", true, "BitmapFactory");
		I_ASSIGN(m_paramsManagerParamIdAttrPtr, "ParamsManagerId", "Id of Region ParamsManager in ParamsSet", false, "RegionParamsManager");
		I_ASSIGN(m_copyProcessorCompPtr, "CopyProcessorMultiRegion", "Copy Processor: Mandatory for multi-region processing", false, "CopyProcessor");
		I_ASSIGN(m_globalParamsIdAttrPtr, "GlobalParamsId", "Id of global ParamsSet in ParamsSet", false, "GlobalParams");
		I_ASSIGN(m_defaultProcessingParamsIdAttrPtr, "DefaultParamsId", "Id of default Processing Parameters in paramsSet", false, "Default");
	I_END_COMPONENT;

protected:

	int GetRegionsCount(const iprm::IParamsSet* parametersPtr, const QByteArray paramsManagerId) const;

	/**
		Iterate through set of parameters
	*/
	int DoMultiParamsSetProcessing(const iimg::IBitmap* inputBitmapPtr, iimg::IBitmap* outputBitmapPtr, const iprm::IParamsSet* parametersPtr, iproc::IProcessor* imageProcessorPtr) const;

	// reimplemented (CProcessedBitmapSupplierBase)
	virtual int ProcessBitmapWithParameters(const iimg::IBitmap* inputBitmapPtr, iimg::IBitmap* outputBitmapPtr, const iprm::IParamsSet* parametersPtr, iproc::IProcessor* imageProcessorPtr) const override;

	// reimplemented (CProcessedBitmapSupplierBase)
	virtual iimg::IBitmapUniquePtr CreateBitmap() const override
	{
		if (!m_bitmapCompFact.IsValid()){
			return nullptr;
		}

		return m_bitmapCompFact.CreateInstance();
	}

private:
	I_FACT(iimg::IBitmap, m_bitmapCompFact);
	I_ATTR(QByteArray, m_paramsManagerParamIdAttrPtr);
	I_REF(iproc::IProcessor, m_copyProcessorCompPtr);
	I_ATTR(QByteArray, m_defaultProcessingParamsIdAttrPtr);
	I_ATTR(QByteArray, m_globalParamsIdAttrPtr);
};


} // namespace iipr


#endif // !iipr_CProcessedBitmapSupplierComp_included


