#include <iipr/CProcessedBitmapSupplierComp.h>


// ACF includes
#include <ilog/CMessage.h>
#include <iprm/TParamsPtr.h>
#include <iprm/IParamsManager.h>
#include <iipr/CImageCopyProcessorComp.h>


namespace iipr
{


// public methods

const iimg::IBitmapProvider* CProcessedBitmapSupplierBase::GetBitmapProvider() const
{
	if (m_bitmapProviderCompPtr.IsValid()) {
		return m_bitmapProviderCompPtr.GetPtr();
	}
	else
		return nullptr;
}


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CProcessedBitmapSupplierBase::GetCalibration() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr){
		// If exists, provide the calibration filled by underlaying processor:
		if (m_outputBitmapCalibrationCompPtr.IsValid()){
			return m_outputBitmapCalibrationCompPtr.GetPtr();
		}
		// Otherwise delegate the input calibration to output:
		else if (m_inputBitmapCalibrationProviderCompPtr.IsValid()){
			return m_inputBitmapCalibrationProviderCompPtr->GetCalibration();
		}
	}

	return nullptr;
}


// reimplemented (iimg::IBitmapProvider)

const iimg::IBitmap* CProcessedBitmapSupplierBase::GetBitmap() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr){
		return productPtr->GetPtr();
	}

	return nullptr;
}


// protected methods

bool CProcessedBitmapSupplierBase::EnsureBitmapCreated(ProductType& result) const
{
	if (!result.IsValid()){
		result.FromUnique(CreateBitmap());
	}

	// zero result
	if (result.IsValid()) {
		result.GetPtr()->ClearImage();
	}

	return result.IsValid();
}


iproc::IProcessor* CProcessedBitmapSupplierBase::GetImageProcessor() const
{
	return m_imageProcessorCompPtr.GetPtr();
}


// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CProcessedBitmapSupplierBase::ProduceObject(ProductType& result) const
{
	if (m_outputBitmapCalibrationCompPtr.IsValid()){
		m_outputBitmapCalibrationCompPtr->ResetData();
	}

	iproc::IProcessor* imageProcessorPtr = GetImageProcessor();

	if (!m_bitmapProviderCompPtr.IsValid() || (imageProcessorPtr == nullptr)){
		return WS_FAILED;
	}

	if (!EnsureBitmapCreated(result)){
		return WS_FAILED;
	}
	Q_ASSERT(result.IsValid());

	const iimg::IBitmap* bitmapPtr = m_bitmapProviderCompPtr->GetBitmap();
	if (bitmapPtr == nullptr){
		AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("No input image"), "BitmapProcessing"));

		return WS_FAILED;
	}

	Timer performanceTimer(this, "Bitmap processing");

	int status = ProcessBitmapWithParameters(bitmapPtr, result.GetPtr(), GetModelParametersSet(), imageProcessorPtr);

	switch (status) {
	case iproc::IProcessor::TS_OK:
		return WS_OK;

	case iproc::IProcessor::TS_CANCELED:
		return WS_CANCELED;

	default:
		return WS_FAILED;
	}

	// should not get here
	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CProcessedBitmapSupplierBase::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr(), m_bitmapSupplierCompPtr.GetPtr());
	}

	if (m_inputBitmapCalibrationProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_inputBitmapCalibrationProviderModelCompPtr.GetPtr(), m_inputBitmapCalibrationSupplierCompPtr.GetPtr());
	}

	m_bitmapProviderCompPtr.EnsureInitialized();
	m_imageProcessorCompPtr.EnsureInitialized();
}


void CProcessedBitmapSupplierBase::OnComponentDestroyed()
{
	if (m_bitmapProviderModelCompPtr.IsValid()){
		UnregisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr());
	}

	if (m_inputBitmapCalibrationProviderModelCompPtr.IsValid()){
		UnregisterSupplierInput(m_inputBitmapCalibrationProviderModelCompPtr.GetPtr());
	}

	BaseClass::OnComponentDestroyed();
}


// virtual method base implementation

int CProcessedBitmapSupplierBase::ProcessBitmapWithParameters(const iimg::IBitmap* inputBitmapPtr, iimg::IBitmap* outputBitmapPtr, const iprm::IParamsSet* parametersPtr, iproc::IProcessor* imageProcessorPtr) const
{
	if (imageProcessorPtr == nullptr){
		imageProcessorPtr = GetImageProcessor();
	}
	if (imageProcessorPtr != nullptr){
		return imageProcessorPtr->DoProcessing(parametersPtr, inputBitmapPtr, outputBitmapPtr);
	}

	return iproc::IProcessor::TS_INVALID;
}


// reimplemented (icam::ICameraAcquisition)

bool CProcessedBitmapSupplierBase::StartCamera() const
{
	if (m_inputCameraAcquisitionCompPtr.IsValid()) {
		return m_inputCameraAcquisitionCompPtr->StartCamera();
	}

	return false;
}


bool CProcessedBitmapSupplierBase::StopCamera() const
{
	if (m_inputCameraAcquisitionCompPtr.IsValid()) {
		return m_inputCameraAcquisitionCompPtr->StopCamera();
	}

	return false;
}


bool CProcessedBitmapSupplierBase::GetLastImage(iimg::IBitmap& snapBitmap) const
{
	if (!m_inputCameraAcquisitionCompPtr.IsValid()) {
		return false;
	}

	iproc::IProcessor* imageProcessorPtr = GetImageProcessor();
	if (imageProcessorPtr == nullptr) {
		return false;
	}

	iimg::IBitmapSharedPtr snapBmpPtr;
	if (!EnsureBitmapCreated(snapBmpPtr)) {
		return false;
	}

	if (!m_inputCameraAcquisitionCompPtr->GetLastImage(*snapBmpPtr)) {
		return false;
	}

	if (imageProcessorPtr->DoProcessing(GetModelParametersSet(), snapBmpPtr.GetPtr(), &snapBitmap) != iproc::IProcessor::TS_OK){
		return false;
	}

	return true;
}


// methods of CProcessedBitmapSupplierComp

int CProcessedBitmapSupplierComp::GetRegionsCount(const iprm::IParamsSet* parametersPtr, const QByteArray paramsManagerId) const
{
	if ((parametersPtr != nullptr) && (!paramsManagerId.isEmpty())){
		iprm::TParamsPtr<iprm::IParamsManager> managerPtr(parametersPtr, paramsManagerId);
		if (managerPtr.IsValid()){
			return managerPtr->GetParamsSetsCount();
		}
	}

	return 0;
}


int CProcessedBitmapSupplierComp::DoMultiParamsSetProcessing(const iimg::IBitmap* inputBitmapPtr, iimg::IBitmap* outputBitmapPtr, const iprm::IParamsSet* parametersPtr, iproc::IProcessor* imageProcessorPtr) const
{
	int status = iproc::IProcessor::TS_OK;

	Q_ASSERT(imageProcessorPtr != nullptr);

	iprm::TParamsPtr<iprm::IParamsManager> paramsManagerPtr(parametersPtr, *m_paramsManagerParamIdAttrPtr);
	const iprm::IParamsManager* managerPtr = paramsManagerPtr.GetPtr();
	Q_ASSERT(managerPtr != nullptr);

	const int paramsSetCount = managerPtr->GetParamsSetsCount();
	Q_ASSERT(paramsSetCount != 0);

	iimg::IBitmapUniquePtr tempBitmapPtr(CreateBitmap());
	tempBitmapPtr->CopyFrom(*outputBitmapPtr);

	for (int i = 0; i < paramsSetCount; ++i) {
		const iprm::IParamsSet* paramsPtr = managerPtr->GetParamsSet(i);
		if (paramsPtr != nullptr) {
			int oneStatus = imageProcessorPtr->DoProcessing(paramsPtr, inputBitmapPtr, tempBitmapPtr.GetPtr());

			if (!m_copyProcessorCompPtr.IsValid()){
				//SendWarningMessage(0, QString("No Region CopyProcessor provided, result skips processed region %1 (%2)").arg(i).arg(managerPtr->GetParamsSetName(i)));
				outputBitmapPtr->CopyFrom(*tempBitmapPtr);
			}
			else {
				m_copyProcessorCompPtr->DoProcessing(paramsPtr, tempBitmapPtr.GetPtr(), outputBitmapPtr);
			}

			status = qMax(status, oneStatus);	// max should be worse? 
		}
	}

	return status;
}


// reimplemented (CProcessedBitmapSupplierBase)

int CProcessedBitmapSupplierComp::ProcessBitmapWithParameters(const iimg::IBitmap* inputBitmapPtr, iimg::IBitmap* outputBitmapPtr, const iprm::IParamsSet* parametersPtr, iproc::IProcessor* imageProcessorPtr) const
{
	// multi region
	if (m_paramsManagerParamIdAttrPtr.IsValid()) 
	{
		// copy input to output before any region processing
		if (m_copyProcessorCompPtr.IsValid()) {
			m_copyProcessorCompPtr->DoProcessing(nullptr, inputBitmapPtr, outputBitmapPtr);
		}
		else
			outputBitmapPtr->CopyFrom(*inputBitmapPtr);

		// if managerId is valid and manager is not empty
		int regionsCount = GetRegionsCount(parametersPtr, *m_paramsManagerParamIdAttrPtr);
		if (regionsCount)
			return DoMultiParamsSetProcessing(inputBitmapPtr, outputBitmapPtr, parametersPtr, imageProcessorPtr);

		// if no global params set -> bail out
		if (!m_globalParamsIdAttrPtr.IsValid()) {
			// #10887: no regions defined - so tell about this
			AddMessage(ilog::CMessage::IC_WARNING, QT_TR_NOOP("No processing regions defined"));

			// we deliberately wont fail here - with "ok" we wanna see input image instead of /\ ...
			return iproc::IProcessor::TS_OK;
		}
	}

	// if manager is empty or not defined, try global
	if (imageProcessorPtr == nullptr)
		imageProcessorPtr = GetImageProcessor();

	if (imageProcessorPtr != nullptr){
		if (m_globalParamsIdAttrPtr.IsValid()){
			const iprm::TParamsPtr<iprm::IParamsSet> globalParamsPtr(parametersPtr, *m_globalParamsIdAttrPtr);
			if (globalParamsPtr.IsValid()){
				return imageProcessorPtr->DoProcessing(globalParamsPtr.GetPtr(), inputBitmapPtr, outputBitmapPtr);
			}
		}
		else{	//try the whole paramsSet
			return imageProcessorPtr->DoProcessing(parametersPtr, inputBitmapPtr, outputBitmapPtr);
		}
	}
	
	return iproc::IProcessor::TS_INVALID;
}


} // namespace iipr


