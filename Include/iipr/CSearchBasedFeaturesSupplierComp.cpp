#include <iipr/CSearchBasedFeaturesSupplierComp.h>


// ACF includes
#include <ilog/CMessage.h>
#include <iprm/IParamsManager.h>
#include <iprm/TParamsPtr.h>
#include <iprm/INameParam.h>

// ACF-Solutions includes
#include <iipr/CSearchFeature.h>
#include <iipr/ISearchParams.h>


namespace iipr
{


// public methods

CSearchBasedFeaturesSupplierComp::CSearchBasedFeaturesSupplierComp()
	: m_defaultInformationCategory(istd::IInformationProvider::IC_NONE)
{
	m_calibrationList.SetParentPtr(this);
}


// reimplemented (i2d::IMultiCalibrationProvider)

const iprm::IOptionsList* CSearchBasedFeaturesSupplierComp::GetCalibrationSelectionContraints() const
{
	return &m_calibrationList;
}


int CSearchBasedFeaturesSupplierComp::GetCalibrationsCount() const
{
	return m_calibrations.count();
}


const i2d::ICalibration2d* CSearchBasedFeaturesSupplierComp::GetCalibration(int calibrationIndex) const
{
	Q_ASSERT(calibrationIndex >= 0);
	Q_ASSERT(calibrationIndex < m_calibrations.count());

	return &m_calibrations.at(calibrationIndex).calibration;
}


// reimplemented (iipr::IFeaturesProvider)

int CSearchBasedFeaturesSupplierComp::GetFeaturesCount() const
{
	const CFeaturesContainer* containerPtr = GetWorkProduct();
	if (containerPtr != nullptr){
		return containerPtr->GetFeaturesCount();
	}

	return 0;
}
	

const imeas::INumericValue& CSearchBasedFeaturesSupplierComp::GetFeature(int index) const
{
	const CFeaturesContainer* containerPtr = GetWorkProduct();
	Q_ASSERT (containerPtr != nullptr);

	return containerPtr->GetFeature(index);
}


// reimplemented (istd::IInformationProvider)

QDateTime CSearchBasedFeaturesSupplierComp::GetInformationTimeStamp() const
{
	if (m_slaveInformationProviderCompPtr.IsValid()){
		return m_slaveInformationProviderCompPtr->GetInformationTimeStamp();
	}

	return QDateTime::currentDateTime();
}


istd::IInformationProvider::InformationCategory CSearchBasedFeaturesSupplierComp::GetInformationCategory() const
{
	if (m_slaveInformationProviderCompPtr.IsValid()){
		return m_slaveInformationProviderCompPtr->GetInformationCategory();
	}

	return m_defaultInformationCategory;
}


int CSearchBasedFeaturesSupplierComp::GetInformationId() const
{
	if (m_slaveInformationProviderCompPtr.IsValid()){
		return m_slaveInformationProviderCompPtr->GetInformationId();
	}

	return -1;
}


QString CSearchBasedFeaturesSupplierComp::GetInformationDescription() const
{
	if (m_slaveInformationProviderCompPtr.IsValid()){
		return m_slaveInformationProviderCompPtr->GetInformationDescription();
	}

	return "";
}


QString CSearchBasedFeaturesSupplierComp::GetInformationSource() const
{
	if (m_slaveInformationProviderCompPtr.IsValid()){
		return m_slaveInformationProviderCompPtr->GetInformationSource();
	}

	return "";
}


int CSearchBasedFeaturesSupplierComp::GetInformationFlags() const
{
	if (m_slaveInformationProviderCompPtr.IsValid()){
		return m_slaveInformationProviderCompPtr->GetInformationFlags();
	}

	return 0;
}


// protected methods

istd::IInformationProvider::InformationCategory CSearchBasedFeaturesSupplierComp::EvaluateResults(int featuresCount, int nominalModelsCount, QString& searchResultText) const
{
	istd::IInformationProvider::InformationCategory informationCategory = istd::IInformationProvider::IC_INFO;

	if (nominalModelsCount < 0){	// all models enabled
		if (featuresCount == 0){
			searchResultText = QT_TR_NOOP("No search feature(s) found");
			informationCategory = *m_anyNumberOfModelsAllowedAttr ? istd::IInformationProvider::IC_INFO : istd::IInformationProvider::IC_ERROR;
		}
		else {
			searchResultText = QT_TR_NOOP(QString("%1 search feature(s) found").arg(featuresCount));
			informationCategory = istd::IInformationProvider::IC_INFO;
		}
	}
	else
		if (nominalModelsCount == 0){	// no models allowed
			if (featuresCount == 0){
				searchResultText = QT_TR_NOOP("No search feature(s) found");	// not found and it must be so
				informationCategory = istd::IInformationProvider::IC_INFO;
			}
			else {
				searchResultText = QT_TR_NOOP(QString("%1 extra feature(s) found").arg(featuresCount));
				informationCategory = istd::IInformationProvider::IC_ERROR;
			}
		}
		else {	// the number of models
			if (featuresCount == 0){
				searchResultText = QT_TR_NOOP("No search feature(s) found");
				informationCategory = istd::IInformationProvider::IC_ERROR;
			}
			else if (featuresCount == nominalModelsCount){
				searchResultText = QT_TR_NOOP(QString("%1 search feature(s) found").arg(featuresCount));
				informationCategory = istd::IInformationProvider::IC_INFO;
			}
			else {
				searchResultText = QT_TR_NOOP(QString("Not every search feature(s) found (%1 of %2)").arg(featuresCount).arg(nominalModelsCount));
				informationCategory = istd::IInformationProvider::IC_WARNING;
			}
		}

	return informationCategory;
}


iinsp::ISupplier::WorkStatus CSearchBasedFeaturesSupplierComp::SearchOneParamsSet(
				const iprm::IParamsSet* paramsPtr,
				const iimg::IBitmap* bitmapPtr,
				CFeaturesContainer& oneSearchResult,
				istd::IInformationProvider::InformationCategory& informationCategory,
				const QString& sourceName) const
{
	m_searchProcessorCompPtr->InitProcessor(paramsPtr);

	int searchState = m_searchProcessorCompPtr->DoProcessing(
		paramsPtr,
		bitmapPtr,
		&oneSearchResult);

	if (searchState != iproc::IProcessor::TS_OK){
		ilog::CMessage* message = new ilog::CMessage(
			istd::IInformationProvider::IC_NONE,
			MI_SUPPLIER_RESULTS_STATUS,
			QT_TR_NOOP("Search not successfull"),
			sourceName);

		AddMessage(message);

		return WS_FAILED;
	}

	const int featuresCount = oneSearchResult.GetFeaturesCount();

	// logical backup status set to error if no models found
	const iipr::ISearchParams* searchParamsPtr = dynamic_cast<const iipr::ISearchParams*>(paramsPtr->GetParameter(*m_searchParamsIdAttrPtr));
	int nominalModelsCount = -1;
	if (searchParamsPtr != nullptr) {
		nominalModelsCount = searchParamsPtr->GetNominalModelsCount();
	}

	QString searchResultText;
	informationCategory = EvaluateResults(featuresCount, nominalModelsCount, searchResultText);

	// extract region name
	const iprm::INameParam* paramsSetNamePtr = dynamic_cast<const iprm::INameParam*>(paramsPtr->GetParameter("Name"));
	if (paramsSetNamePtr != nullptr){
		QString name = paramsSetNamePtr->GetName() + ": ";
		searchResultText.prepend(name);
	}

	ilog::CMessage* message = new ilog::CMessage(
		informationCategory,
		MI_SUPPLIER_RESULTS_STATUS,
		searchResultText,
		sourceName);

	AddMessage(message);

	return WS_OK;
}


bool CSearchBasedFeaturesSupplierComp::IsNegativeModelFound(const imeas::INumericValue* featurePtr, const QString& sourceName) const
{
	const iipr::CSearchFeature* searchFeaturePtr = dynamic_cast<const iipr::CSearchFeature*>(featurePtr);
	if ((searchFeaturePtr != nullptr) && (m_defaultInformationCategory != istd::IInformationProvider::IC_ERROR && searchFeaturePtr->IsNegativeModelEnabled())){
		m_defaultInformationCategory = istd::IInformationProvider::IC_ERROR;
		QString searchResultText = QT_TR_NOOP("Some not allowed feature(s) found");

		ilog::CMessage* message = new ilog::CMessage(
			m_defaultInformationCategory,
			MI_SUPPLIER_RESULTS_STATUS,
			searchResultText,
			sourceName);

		AddMessage(message);
		return true;
	}
	return false;
}


iinsp::ISupplier::WorkStatus CSearchBasedFeaturesSupplierComp::LabelAndStoreResult(CFeaturesContainer & fromContainer, CFeaturesContainer & toResult, const QString & sourceName) const
{
	const int featuresCount = fromContainer.GetFeaturesCount();
	for (int featureIndex = 0; featureIndex < featuresCount; featureIndex++) {
		const iipr::CObjectFeature* objectFeaturePtr = dynamic_cast<const iipr::CObjectFeature*>(&fromContainer.GetFeature(featureIndex));
		if (objectFeaturePtr == nullptr) {
			return WS_FAILED;
		}

		if (IsNegativeModelFound(&fromContainer.GetFeature(featureIndex), sourceName)) {
			continue;
		}

		QByteArray featureId = objectFeaturePtr->GetObjectId();
		QByteArray objectId = sourceName.toUtf8() + "/" + featureId;

		const_cast<iipr::CObjectFeature*>(objectFeaturePtr)->SetObjectId(objectId);

		istd::IChangeableUniquePtr featurePtr = objectFeaturePtr->CloneMe(CM_WITH_REFS);
		if (!featurePtr.IsValid()) {
			return WS_FAILED;
		}
		
		imeas::INumericValue* valuePtr = dynamic_cast<imeas::INumericValue*>(featurePtr.PopInterfacePtr());
		if (valuePtr == nullptr) {
			return WS_FAILED;
		}

		toResult.AddFeature(valuePtr);
	}

	return WS_OK;
}


void CSearchBasedFeaturesSupplierComp::UpdateCalibrationList(const CFeaturesContainer & results) const
{
	int featuresCount = results.GetFeaturesCount();
	for (int featureIndex = 0; featureIndex < featuresCount; featureIndex++) {
		CalibrationInfo calibrationInfo;

		const iipr::CObjectFeature* objectFeaturePtr = dynamic_cast<const iipr::CObjectFeature*>(&results.GetFeature(featureIndex));
		if (objectFeaturePtr != nullptr) {

			i2d::CAffine2d globalAffine;
			if (
				m_globalCalibrationPtr.IsValid() &&
				m_globalCalibrationPtr->GetLocalTransform(objectFeaturePtr->GetPosition(), globalAffine)
				) {
				const i2d::CMatrix2d& globalMatrix = globalAffine.GetDeformMatrix();
				const double angle = globalMatrix.GetApproxAngle() - objectFeaturePtr->GetAngle();
				const i2d::CVector2d scale = objectFeaturePtr->GetScale() * globalMatrix.GetApproxScale();
				calibrationInfo.calibration.Reset(objectFeaturePtr->GetPosition(), angle, scale);
			}
			else {
				calibrationInfo.calibration.Reset(objectFeaturePtr->GetPosition(), -objectFeaturePtr->GetAngle(), objectFeaturePtr->GetScale());
			}

			calibrationInfo.calibrationId = objectFeaturePtr->GetObjectId();

			m_calibrations.push_back(calibrationInfo);
		}
	}
}


// reimplemented (iinsp::TSupplierCompWrap)

bool CSearchBasedFeaturesSupplierComp::InitializeWork()
{
	if (m_searchProcessorCompPtr.IsValid()){
		if (!m_searchParamsManagerParamIdAttrPtr.IsValid()){
			m_searchProcessorCompPtr->InitProcessor(GetModelParametersSet());
		}

		return true;
	}

	return false;
}


iinsp::ISupplier::WorkStatus CSearchBasedFeaturesSupplierComp::ProduceObject(CFeaturesContainer& result) const
{
	result.ResetFeatures();

	m_calibrations.clear();

	if (		m_bitmapProviderCompPtr.IsValid() &&
				m_searchProcessorCompPtr.IsValid()){
		const iimg::IBitmap* bitmapPtr = m_bitmapProviderCompPtr->GetBitmap();
		if (bitmapPtr == nullptr){
			AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("No input image"), "FeatureSearch"));

			return WS_FAILED;
		}

		iprm::IParamsSet* paramsSetPtr = GetModelParametersSet();

		const iprm::IParamsManager* multiSearchParamsManagerPtr = nullptr;
		if (m_searchParamsManagerParamIdAttrPtr.IsValid()){
			iprm::TParamsPtr<iprm::IParamsManager> paramsManagerPtr(paramsSetPtr, *m_searchParamsManagerParamIdAttrPtr);
			if (!paramsManagerPtr.IsValid()){
				SendErrorMessage(iproc::IProcessor::MI_BAD_PARAMS, "No inspection list found");

				return WS_FAILED;
			}

			multiSearchParamsManagerPtr = paramsManagerPtr.GetPtr();
		}

		istd::IInformationProvider::InformationCategory informationCategory = istd::IInformationProvider::IC_INFO;
		iinsp::ISupplier::WorkStatus outputStatus = WS_OK;

		Timer performanceTimer(this, "Search of features");

		if (multiSearchParamsManagerPtr != nullptr){
			int searchCount = multiSearchParamsManagerPtr->GetParamsSetsCount();
			m_defaultInformationCategory = istd::IInformationProvider::IC_NONE;

			for (int searchIndex = 0; searchIndex < searchCount; searchIndex++){

				const QString sourceName = multiSearchParamsManagerPtr->GetParamsSetName(searchIndex);
				const iprm::IParamsSet* paramsPtr = multiSearchParamsManagerPtr->GetParamsSet(searchIndex);

				CFeaturesContainer oneSearchResult;
				oneSearchResult.ResetFeatures();

				iinsp::ISupplier::WorkStatus workStatus = SearchOneParamsSet(paramsPtr, bitmapPtr, oneSearchResult, informationCategory, sourceName);
				if (workStatus != WS_OK){
					outputStatus = qMax(outputStatus, workStatus);
				}

				workStatus = LabelAndStoreResult(oneSearchResult, result, sourceName);
				if (workStatus != WS_OK){
					outputStatus = qMax(outputStatus, workStatus);
				}

				m_defaultInformationCategory = qMax(m_defaultInformationCategory, informationCategory);
			}
		}
		else{ // Single search
			QString sourceName = GetDiagnosticName();
			if (sourceName.isEmpty()) {
				sourceName = "Search Result";
			}

			iinsp::ISupplier::WorkStatus singleStatus = SearchOneParamsSet(paramsSetPtr, bitmapPtr, result, informationCategory, sourceName);
			if (singleStatus != WS_OK){
				return singleStatus;
			}

			const int featuresCount = result.GetFeaturesCount();

			for (int featureIndex = 0; featureIndex < featuresCount; featureIndex++){
				if (IsNegativeModelFound(&result.GetFeature(featureIndex), sourceName)){
					break;
				}
			}
		}

		// Update calibration list
		UpdateCalibrationList(result);

		return outputStatus;
	}

	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CSearchBasedFeaturesSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr(), m_bitmapSupplierCompPtr.GetPtr());
	}

	m_searchProcessorCompPtr.EnsureInitialized();
}


void CSearchBasedFeaturesSupplierComp::OnComponentDestroyed()
{
	BaseClass::OnComponentDestroyed();

	if (m_bitmapProviderModelCompPtr.IsValid()){
		UnregisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr());
	}
}


// public methods of the embedded class CalibrationList

CSearchBasedFeaturesSupplierComp::CalibrationList::CalibrationList()
	:m_parentPtr(nullptr)
{
}


void CSearchBasedFeaturesSupplierComp::CalibrationList::SetParentPtr(CSearchBasedFeaturesSupplierComp* parentPtr)
{
	m_parentPtr = parentPtr;
}


// reimplemented (iprm::IOptionsList)

int CSearchBasedFeaturesSupplierComp::CalibrationList::GetOptionsFlags() const
{
	return SCF_SUPPORT_UNIQUE_ID;
}


int CSearchBasedFeaturesSupplierComp::CalibrationList::GetOptionsCount() const
{
	Q_ASSERT(m_parentPtr != nullptr);

	return m_parentPtr->m_calibrations.count();
}


QString CSearchBasedFeaturesSupplierComp::CalibrationList::GetOptionName(int index) const
{
	Q_ASSERT(m_parentPtr != nullptr);

	return m_parentPtr->m_calibrations[index].calibrationName;
}


QString CSearchBasedFeaturesSupplierComp::CalibrationList::GetOptionDescription(int /*index*/) const
{
	return QString();
}


QByteArray CSearchBasedFeaturesSupplierComp::CalibrationList::GetOptionId(int index) const
{
	Q_ASSERT(m_parentPtr != nullptr);

	return m_parentPtr->m_calibrations[index].calibrationId;
}


bool CSearchBasedFeaturesSupplierComp::CalibrationList::IsOptionEnabled(int /*index*/) const
{
	return true;
}


} // namespace iipr


