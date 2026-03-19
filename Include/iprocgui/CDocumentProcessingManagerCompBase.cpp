// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iprocgui/CDocumentProcessingManagerCompBase.h>


// Qt includes
#include <QtWidgets/QMessageBox>

// ACF includes
#include <istd/CChangeNotifier.h>
#include <iqtgui/CGuiComponentDialog.h>
#include <iqtgui/CSubtaskProgressDialog.h>

// AcfSln includes
#include <iprocgui/CDocumentProcessingWorkerThread.h>


namespace iprocgui
{


// public methods

CDocumentProcessingManagerCompBase::CDocumentProcessingManagerCompBase()
:	m_rootCommands("", 100, ibase::ICommand::CF_GLOBAL_MENU),
	m_processingCommand("", 100, ibase::ICommand::CF_GLOBAL_MENU)
{
}


bool CDocumentProcessingManagerCompBase::IsInputDocumentRequired() const
{
	bool retVal = true;

	if (m_inputDocumentRequiredAttrPtr.IsValid()){
		retVal = *m_inputDocumentRequiredAttrPtr;
	}

	return retVal;
}


void CDocumentProcessingManagerCompBase::SetProcessingCommandEnabled(bool isProcessingCommandEnabled)
{
	m_processingCommand.setEnabled(isProcessingCommandEnabled);
}


// reimpemented (ibase::ICommandsProvider)

const ibase::IHierarchicalCommand* CDocumentProcessingManagerCompBase::GetCommands() const
{
	return &m_processingMenu;
}


// reimpemented (icomp::IComponent)

void CDocumentProcessingManagerCompBase::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_documentManagerModelCompPtr.IsValid()){
		RegisterModel(m_documentManagerModelCompPtr.GetPtr());
	}
	
	QString menuName;

	if (m_menuNameAttrPtr.IsValid()){
		menuName = *m_menuNameAttrPtr;
	}

	if (m_commandNameAttrPtr.IsValid() && !(*m_commandNameAttrPtr).isEmpty()){
		QString commandName = *m_commandNameAttrPtr;

		QIcon commandIcon;

		if (m_commandIconPathAttrPtr.IsValid()){
			commandIcon = GetIcon(*m_commandIconPathAttrPtr);
		}
		
		if (*m_commandShowInToolBarAttrPtr){
			m_processingCommand.SetStaticFlags(m_processingCommand.GetStaticFlags() | ibase::ICommand::CF_TOOLBAR);
		}

		m_processingCommand.SetGroupId(*m_commandGroupIdAttrPtr);
		m_processingCommand.SetVisuals(commandName, commandName, commandName, commandIcon);

		m_rootCommands.SetVisuals(menuName, menuName, tr("Document processing actions"));
		m_rootCommands.InsertChild(&m_processingCommand);
		m_processingMenu.InsertChild(&m_rootCommands);

		connect(&m_processingCommand, SIGNAL(triggered()), this, SLOT(OnDoProcessing()));
	}
}


void CDocumentProcessingManagerCompBase::OnComponentDestroyed()
{
	UnregisterAllModels();

	BaseClass::OnComponentDestroyed();
}


// protected methods

// reimplemented (ibase::TDesignSchemaHandlerWrap)

void CDocumentProcessingManagerCompBase::OnDesignSchemaChanged(const QByteArray& themeId)
{
	BaseClass2::OnDesignSchemaChanged(themeId);

	QIcon commandIcon;

	if (m_commandIconPathAttrPtr.IsValid()){
		commandIcon = GetIcon(*m_commandIconPathAttrPtr);
	}

	m_processingCommand.SetVisuals(m_processingCommand.GetName(), m_processingCommand.GetName(), m_processingCommand.GetName(), commandIcon);
}


// reimplemented (imod::CMultiModelDispatcherBase)

void CDocumentProcessingManagerCompBase::OnModelChanged(int modelId, const istd::IChangeable::ChangeSet& /*changeSet*/)
{
	idoc::IDocumentManager* objectPtr = GetObjectAt<idoc::IDocumentManager>(modelId);
	Q_ASSERT(objectPtr != NULL);
	if (objectPtr != NULL){
		SetProcessingCommandEnabled((objectPtr->GetActiveView() != NULL) || !IsInputDocumentRequired());
	}
}


// private slots

void CDocumentProcessingManagerCompBase::OnDoProcessing()
{
	if (!m_documentManagerCompPtr.IsValid()){
		SendErrorMessage(0, "Document manager was not set", "Document processing manager");

		return;
	}

	if (!m_processorCompPtr.IsValid()){
		SendErrorMessage(0, "Processor was not set", "Document processing manager");

		return;
	}

	bool inputDocumentRequired = IsInputDocumentRequired();

	istd::IPolymorphic* viewPtr = m_documentManagerCompPtr->GetActiveView();
	if ((viewPtr == NULL) && inputDocumentRequired){
		return;
	}

	istd::IChangeable* inputDocumentPtr = m_documentManagerCompPtr->GetDocumentFromView(*viewPtr);
	if ((inputDocumentPtr == NULL) && inputDocumentRequired){
		return;
	}
	
	QByteArray documentTypeId;
	if (inputDocumentPtr != NULL){
		documentTypeId = m_documentManagerCompPtr->GetDocumentTypeId(*inputDocumentPtr);
	}
	
	if (m_documentTypeIdAttrPtr.IsValid()){
		documentTypeId = *m_documentTypeIdAttrPtr;
	}

	// Prepare/initialize processing parameters for the document:
	if (m_paramsControllerCompPtr.IsValid() && (inputDocumentPtr != nullptr) && m_paramsSetCompPtr.IsValid()){
		QString errorMessage;
		bool success = m_paramsControllerCompPtr->ConfigureParametersForDocument(*inputDocumentPtr, *m_paramsSetCompPtr, &errorMessage);
		if (!success){
			QMessageBox::critical(nullptr, tr("Document Processing"), tr("Processing parameters could not be initialized. Operation canceled."));

			return;
		}
	}

	istd::TDelPtr<iqtgui::CGuiComponentDialog> dialogPtr;

	if (m_paramsGuiCompPtr.IsValid()){
		dialogPtr.SetPtr(
					new iqtgui::CGuiComponentDialog(
								m_paramsGuiCompPtr.GetPtr(),
								QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
								true));

		int retVal = dialogPtr->exec();
		if (retVal != QDialog::Accepted){
			return;
		}
	}

	// Force model update:
	dialogPtr.Reset();

	// Setup working progress manager:
	ibase::IProgressManager* progressPtr = m_progressManagerCompPtr.GetPtr();

	// If no user-specific progress manager was set, use the standard one:
	iqtgui::CSubtaskProgressDialog progressDialog(tr("Progress"), tr(""));
	if (!m_progressManagerCompPtr.IsValid()){
		progressPtr = &progressDialog;
	}

	// Prepare processing on UI thread:
	istd::IChangeable* outputDocumentPtr = nullptr;
	istd::IChangeable* changeTargetPtr = nullptr;

	if (!PrepareProcessing(inputDocumentPtr, documentTypeId, progressPtr, outputDocumentPtr, changeTargetPtr)){
		return;
	}

	// Begin change notification on UI thread (before worker thread modifies the output):
	istd::CChangeNotifier changeNotifier(changeTargetPtr);

	// Run only the pure processing (DoProcessing) on a worker thread:
	CDocumentProcessingWorkerThread workerThread(
				m_processorCompPtr.GetPtr(),
				m_paramsSetCompPtr.GetPtr(),
				inputDocumentPtr,
				outputDocumentPtr,
				progressPtr);

	if (!m_progressManagerCompPtr.IsValid()){
		connect(&workerThread, SIGNAL(finished()), &progressDialog, SLOT(close()));
	}

	workerThread.start();

	if (!m_progressManagerCompPtr.IsValid()){
		progressDialog.exec();
	}

	workerThread.wait();

	// Finalize processing on UI thread:
	FinalizeProcessing(
				inputDocumentPtr,
				documentTypeId,
				outputDocumentPtr,
				workerThread.GetResultCode(),
				workerThread.GetProcessingTime(),
				changeNotifier);

	// changeNotifier destructs here (EndChanges on UI thread, if not already Reset/Aborted)
}


} // namespace iprocgui


