// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CSelectionParamCameraComp.h>


// ACF includes
#include <iprm/TParamsPtr.h>


namespace icam
{


// reimplemented (iipr::IBitmapAcquisition)

istd::CIndex2d CSelectionParamCameraComp::GetBitmapSize(const iprm::IParamsSet* paramsPtr) const
{
	ParamSetPtr extParamsPtr(CreateParamsSet(paramsPtr));

	return BaseClass::GetBitmapSize(extParamsPtr.GetPtr());
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::ProcessorState CSelectionParamCameraComp::GetProcessorState(const iprm::IParamsSet* paramsPtr) const
{
	ParamSetPtr extParamsPtr(CreateParamsSet(paramsPtr));

	return BaseClass::GetProcessorState(extParamsPtr.GetPtr());
}


bool CSelectionParamCameraComp::AreParamsAccepted(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			const istd::IChangeable* outputPtr) const
{
	ParamSetPtr extParamsPtr(CreateParamsSet(paramsPtr));

	return BaseClass::AreParamsAccepted(
				extParamsPtr.GetPtr(),
				inputPtr,
				outputPtr);
}


iproc::IProcessor::TaskState CSelectionParamCameraComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	ParamSetPtr extParamsPtr(CreateParamsSet(paramsPtr));

	return extParamsPtr != NULL ? BaseClass::DoProcessing(
				extParamsPtr.GetPtr(),
				inputPtr,
				outputPtr,
				progressManagerPtr,
				processingReportPtr)
		: TS_INVALID;
}


int CSelectionParamCameraComp::BeginTask(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr)
{
	ParamSetPtr extParamsPtr(CreateParamsSet(paramsPtr));

	int taskId = extParamsPtr != NULL ? BaseClass::BeginTask(
				extParamsPtr.GetPtr(),
				inputPtr,
				outputPtr,
				progressManagerPtr,
				processingReportPtr) 
		: -1;

	if (taskId >= 0){
		m_paramsMap[taskId].TakeOver(extParamsPtr);
	}

	return taskId;
}


iproc::IProcessor::TaskState CSelectionParamCameraComp::WaitTaskFinished(
				int taskId,
				double timeoutTime,
				bool killOnTimeout)
{
	iproc::IProcessor::TaskState retVal = BaseClass::WaitTaskFinished(
				taskId,
				timeoutTime,
				killOnTimeout);

	if (retVal != TS_WAIT){
		if (taskId >= 0){
			m_paramsMap.remove(taskId);
		}
		else{
			m_paramsMap.clear();
		}
	}

	return retVal;
}


void CSelectionParamCameraComp::CancelTask(int taskId)
{
	BaseClass::WaitTaskFinished(taskId);

	if (taskId >= 0){
		m_paramsMap.remove(taskId);
	}
	else{
		m_paramsMap.clear();
	}
}


void CSelectionParamCameraComp::InitProcessor(const iprm::IParamsSet* paramsPtr)
{
	m_paramsMap.clear();

	ParamSetPtr extParamsPtr(CreateParamsSet(paramsPtr));

	BaseClass::InitProcessor(extParamsPtr.GetPtr());
}


// protected methods

const iprm::IParamsSet* CSelectionParamCameraComp::CreateParamsSet(const iprm::IParamsSet* paramsPtr) const
{
	const iprm::IParamsSet* selectedParamsPtr = NULL;

	int selectedIndex = m_defaultIndexAttrPtr.IsValid() ? *m_defaultIndexAttrPtr : -1;
	iprm::TParamsPtr<iprm::ISelectionParam> selectionPtr(paramsPtr, m_selectionIdAttrPtr, m_defaultSelectionCompPtr, false);
	if (selectionPtr.IsValid()){
		selectedIndex = selectionPtr->GetSelectedOptionIndex();
	}

	iprm::TParamsPtr<iprm::IParamsManager> managerPtr(paramsPtr, m_paramsManagerIdAttrPtr, m_defaultParamsManagerCompPtr, true);
	if (managerPtr.IsValid()){
		if ((selectedIndex >= 0) && selectedIndex < managerPtr->GetParamsSetsCount()){
			selectedParamsPtr = managerPtr->GetParamsSet(selectedIndex);
		}
	}

	if (selectedParamsPtr != NULL) {
		return new JoinParamsSet(paramsPtr, selectedParamsPtr);
	}

	return NULL;
}


// public methods of embedded class JoinParamsSet

CSelectionParamCameraComp::JoinParamsSet::JoinParamsSet(const iprm::IParamsSet* origParamsPtr, const iprm::IParamsSet* selectedParamsPtr)
:	m_origParamsPtr(origParamsPtr),
	m_selectedParamsPtr(selectedParamsPtr)
{
}


// reimplemented (iprm::IParamsSet)

iprm::IParamsSet::Ids CSelectionParamCameraComp::JoinParamsSet::GetParamIds(bool editableOnly) const
{
	iprm::IParamsSet::Ids retVal;

	if (!editableOnly){
		if (m_origParamsPtr != NULL){
			retVal += m_origParamsPtr->GetParamIds(false);
		}

		if (m_selectedParamsPtr != NULL){
			retVal += m_selectedParamsPtr->GetParamIds(false);
		}
	}

	return retVal;
}


const iser::ISerializable* CSelectionParamCameraComp::JoinParamsSet::GetParameter(const QByteArray& id) const
{
	const iser::ISerializable* retVal = NULL;

	if (m_origParamsPtr != NULL){
		retVal = m_origParamsPtr->GetParameter(id);
	}

	if ((retVal == NULL) && (m_selectedParamsPtr != NULL)){
		retVal = m_selectedParamsPtr->GetParameter(id);
	}

	return retVal;
}


iser::ISerializable* CSelectionParamCameraComp::JoinParamsSet::GetEditableParameter(const QByteArray& /*id*/)
{
	return NULL;
}


const iprm::IParamsInfoProvider* CSelectionParamCameraComp::JoinParamsSet::GetParamsInfoProvider() const
{
	return nullptr;
}


// reimplemented (iser::IObject)

QByteArray CSelectionParamCameraComp::JoinParamsSet::GetFactoryId() const
{
	QByteArray retVal;

	if (m_selectedParamsPtr != NULL){
		retVal = m_selectedParamsPtr->GetFactoryId();
	}

	return retVal;
}


// reimplemented (iser::ISerializable)

bool CSelectionParamCameraComp::JoinParamsSet::Serialize(iser::IArchive& /*archive*/)
{
	return true;
}


} // namespace icam


