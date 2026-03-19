// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// Qt includes
#include <QtCore/QObject>

// ACF includes
#include <imod/CMultiModelDispatcherBase.h>
#include <ilog/TLoggerCompWrap.h>
#include <ibase/ICommandsProvider.h>
#include <idoc/IDocumentManager.h>
#include <iqtgui/IGuiObject.h>
#include <iqtgui/CHierarchicalCommand.h>
#include <iqtgui/TMakeIconProviderCompWrap.h>
#include <iproc/IProcessor.h>
#include <idocproc/IDocumentProcessingParamsController.h>

namespace istd
{
class CChangeNotifier;
}


namespace iprocgui
{


class CDocumentProcessingManagerCompBase:
			public QObject,
			public ilog::CLoggerComponentBase,
			protected imod::CMultiModelDispatcherBase,
			public iqtgui::TMakeIconProviderCompWrap <ibase::ICommandsProvider>
{
	Q_OBJECT
public:
	typedef ilog::CLoggerComponentBase BaseClass;
	typedef iqtgui::TMakeIconProviderCompWrap <ibase::ICommandsProvider> BaseClass2;
	
	I_BEGIN_BASE_COMPONENT(CDocumentProcessingManagerCompBase);
		I_REGISTER_INTERFACE(ibase::ICommandsProvider);
		I_ASSIGN(m_documentManagerCompPtr, "DocumentManager", "Application's document manager", true, "DocumentManager");
		I_ASSIGN_TO(m_documentManagerModelCompPtr, m_documentManagerCompPtr, true);
		I_ASSIGN(m_processorCompPtr, "Processor", "Target processing component", true, "Processor");
		I_ASSIGN(m_progressManagerCompPtr, "ProgressManager", "Processing progress manager", false, "ProgressManager");
		I_ASSIGN(m_paramsSetCompPtr, "ProcessingParamsSet", "Processing parameters", false, "ProcessingParameters");
		I_ASSIGN(m_paramsGuiCompPtr, "ProcessingParamsGui", "UI for processing parameters", false, "ProcessingParamsGui");
		I_ASSIGN(m_commandNameAttrPtr, "ProcessingCommandName", "Name of the processing action in menu", true, "");
		I_ASSIGN(m_menuNameAttrPtr, "ProcessingMenuName", "Name of the processing menu", false, "Processing");
		I_ASSIGN(m_inputDocumentRequiredAttrPtr, "InputDocumentRequired", "If enabled, the input document is required for processing", false, true);
		I_ASSIGN(m_documentTypeIdAttrPtr, "DocumentTypeId", "Document type ID used for document creation", false, "Default");
		I_ASSIGN(m_commandGroupIdAttrPtr, "CommandGroup", "Specify the command group ID", true, 0);
		I_ASSIGN(m_commandShowInToolBarAttrPtr, "ShowInToolsBar", "If enabled the command button will be shown also in the tool bar", true, false);
		I_ASSIGN(m_commandIconPathAttrPtr, "CommandIconPath", "Path to the icon resource used for the command", false, "CommandIconPath");
		I_ASSIGN(m_paramsControllerCompPtr, "ParamsController", "Controller for initialization of default parameters based on the input document", false, "ParamsController");
	I_END_COMPONENT;

	CDocumentProcessingManagerCompBase();

	/**
		Return \c true, if the input document is required for processing.
	*/
	bool IsInputDocumentRequired() const;

	/**
		Enable or disable processing command.
	*/
	void SetProcessingCommandEnabled(bool isProcessingCommandEnabled = true);

	// reimpemented (ibase::ICommandsProvider)
	virtual const ibase::IHierarchicalCommand* GetCommands() const override;

	// reimpemented (icomp::IComponent)
	virtual void OnComponentCreated()override;
	virtual void OnComponentDestroyed()override;

	// abstract methods

	/**
		Prepare for document processing. Called on UI thread before the worker thread starts.
		\param inputDocumentPtr input document for processing.
		\param documentTypeId type ID of the document.
		\param progressManagerPtr progress manager for tracking processing progress.
		\param outputDocumentPtr [out] the output document that will receive processing results.
		\param changeTargetPtr [out] the object that will be wrapped in CChangeNotifier for change notification.
		\return true if processing should proceed, false to cancel.
	*/
	virtual bool PrepareProcessing(
				const istd::IChangeable* inputDocumentPtr,
				const QByteArray& documentTypeId,
				ibase::IProgressManager* progressManagerPtr,
				istd::IChangeable*& outputDocumentPtr,
				istd::IChangeable*& changeTargetPtr) = 0;

	/**
		Finalize document processing. Called on UI thread after the worker thread completes.
		\param inputDocumentPtr input document that was processed.
		\param documentTypeId type ID of the document.
		\param outputDocumentPtr the output document that received processing results.
		\param resultCode result code from DoProcessing.
		\param processingTime elapsed processing time.
		\param changeNotifier change notifier wrapping the processing, can be used for Reset() or Abort().
	*/
	virtual void FinalizeProcessing(
				const istd::IChangeable* inputDocumentPtr,
				const QByteArray& documentTypeId,
				istd::IChangeable* outputDocumentPtr,
				int resultCode,
				double processingTime,
				istd::CChangeNotifier& changeNotifier) = 0;

protected:
	// reimplemented (ibase::TDesignSchemaHandlerWrap)
	virtual void OnDesignSchemaChanged(const QByteArray& themeId) override;

	// reimplemented (imod::CMultiModelDispatcherBase)
	virtual void OnModelChanged(int modelId, const istd::IChangeable::ChangeSet& changeSet) override;

private Q_SLOTS:
	void OnDoProcessing();

protected:
	I_REF(idoc::IDocumentManager, m_documentManagerCompPtr);
	I_REF(imod::IModel, m_documentManagerModelCompPtr);
	I_REF(iproc::IProcessor, m_processorCompPtr);
	I_REF(iprm::IParamsSet, m_paramsSetCompPtr);
	I_REF(iqtgui::IGuiObject, m_paramsGuiCompPtr);
	I_TEXTATTR(m_commandNameAttrPtr);
	I_TEXTATTR(m_menuNameAttrPtr);
	I_ATTR(bool, m_inputDocumentRequiredAttrPtr);
	I_ATTR(QByteArray, m_documentTypeIdAttrPtr);
	I_ATTR(int, m_commandGroupIdAttrPtr);
	I_ATTR(bool, m_commandShowInToolBarAttrPtr);
	I_ATTR(QString, m_commandIconPathAttrPtr);
	I_REF(idocproc::IDocumentProcessingParamsController, m_paramsControllerCompPtr);

	iqtgui::CHierarchicalCommand m_processingMenu;
	iqtgui::CHierarchicalCommand m_rootCommands;
	iqtgui::CHierarchicalCommand m_processingCommand;

private:
	I_REF(ibase::IProgressManager, m_progressManagerCompPtr);
};


} // namespace iprocgui


