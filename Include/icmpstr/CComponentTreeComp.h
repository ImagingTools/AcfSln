// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef icmpstr_CComponentTreeComp_included
#define icmpstr_CComponentTreeComp_included


// Qt includes
#include <QtCore/QMap>

// ACF includes
#include <icomp/IRegistry.h>
#include <icomp/IComponentEnvironmentManager.h>
#include <icomp/IRegistryLoader.h>
#include <icomp/CRegistryElement.h>

#include <idoc/IDocumentManager.h>

#include <iqtgui/TDesignerGuiObserverCompBase.h>
#include <iqtgui/TRestorableGuiWrap.h>

#include <icmpstr/IRegistryConsistInfo.h>
#include <icmpstr/IElementSelectionInfo.h>

#include <GeneratedFiles/icmpstr/ui_CComponentTreeComp.h>


namespace icmpstr
{


class CComponentTreeComp:
			public iqtgui::TRestorableGuiWrap<
						iqtgui::TDesignerGuiObserverCompBase<
							Ui::CComponentTreeComp, IElementSelectionInfo> >
{
	Q_OBJECT

public:
	typedef iqtgui::TRestorableGuiWrap<
				iqtgui::TDesignerGuiObserverCompBase<
					Ui::CComponentTreeComp, IElementSelectionInfo> > BaseClass;

	enum DataRole
	{
		DR_ELEMENT_NAME = Qt::UserRole + 1,
		DR_ELEMENT_ID = Qt::UserRole + 2,
		DR_ELEMENT_PACKAGE_ID = Qt::UserRole + 3,
		DR_REGISTRY = Qt::UserRole + 4
	};

	I_BEGIN_COMPONENT(CComponentTreeComp);
		I_ASSIGN(m_envManagerCompPtr, "EnvironmentManager", "Allows access to component environment information", true, "EnvironmentManager");
		I_ASSIGN_TO(m_envManagerModelCompPtr, m_envManagerCompPtr, false);
		I_ASSIGN_TO(m_registryLoaderCompPtr, m_envManagerCompPtr, false);
		I_ASSIGN(m_consistInfoCompPtr, "ConsistencyInfo", "Allows to check consistency of registries and attributes", false, "ConsistencyInfo");
		I_ASSIGN(m_documentManagerCompPtr, "DocumentManager", "Document manager allowing to load files on double click", false, "DocumentManager");
	I_END_COMPONENT;

	CComponentTreeComp();

protected:
	/**
		\internal
		Observes changes of component environment to force update if packages change.
	*/
	class EnvironmentObserver: public imod::TSingleModelObserverBase<icomp::IComponentEnvironmentManager>
	{
	public:
		EnvironmentObserver(CComponentTreeComp* parentPtr);

	protected:
		// reimplemented (imod::TSingleModelObserverBase)
		virtual void OnUpdate(const istd::IChangeable::ChangeSet& changeSet) override;

	private:
		CComponentTreeComp& m_parent;
	};

	void RebuildRootComboBox();
	void RebuildTree();

	void CreateRegistryTree(const icomp::IRegistry& registry, QTreeWidgetItem* registryRootItemPtr);

	void AddSubcomponents(
				const icomp::IRegistry& registry,
				const icomp::CComponentAddress& address,
				QTreeWidgetItem* registryElementItemPtr);

	QTreeWidgetItem* AddRegistryElementItem(
				const icomp::IRegistry& registry,
				const icomp::IRegistry::ElementInfo* elementPtr,
				const QByteArray& elementId,
				QTreeWidgetItem* parentItemPtr);

	void SyncSelectionFromModel();

	/**
		Find tree item matching the element name, disambiguating by registry context.
		When registryPtr is provided and multiple tree items share the same element name,
		the sibling set is compared against the registry's element IDs to find the correct match.
	*/
	QTreeWidgetItem* FindTreeItem(const QByteArray& elementName, const icomp::IRegistry* registryPtr = NULL) const;

	void CollectMatchingItems(const QByteArray& elementName, QTreeWidgetItem* parentPtr, QList<QTreeWidgetItem*>& results) const;

	/**
		Explicitly select a tree item and request element selection in the diagram.
		Used after opening a document to ensure the double-clicked element stays selected.
	*/
	void SelectTreeItem(QTreeWidgetItem* itemPtr, const QByteArray& elementName);

	/**
		Update visibility state of the tree items according to the filter text.
	*/
	void UpdateTreeItemsVisibility();

	/**
		Update the tree from the in-memory registry obtained through the active document.
		Used when structural changes (rename, add, remove) happen in the currently open document,
		since the file-based reload would not reflect unsaved changes.
	*/
	void UpdateTreeFromModel();

	// reimplemented (iqtgui::TRestorableGuiWrap)
	virtual void OnRestoreSettings(const QSettings& settings) override;
	virtual void OnSaveSettings(QSettings& settings) const override;

	// reimplemented (iqtgui::TGuiObserverWrap)
	virtual void UpdateGui(const istd::IChangeable::ChangeSet& changeSet) override;
	virtual void OnGuiModelAttached() override;
	virtual void OnGuiModelDetached() override;

	// reimplemented (iqtgui::CGuiComponentBase)
	virtual void OnGuiCreated() override;

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;
	virtual void OnComponentDestroyed() override;

protected Q_SLOTS:
	void on_RootComboBox_currentIndexChanged(int index);
	void on_FilterEdit_textChanged(const QString& filterText);
	void on_ComponentTree_itemDoubleClicked(QTreeWidgetItem* itemPtr, int column);
	void on_ComponentTree_itemSelectionChanged();

private:
	I_REF(icomp::IComponentEnvironmentManager, m_envManagerCompPtr);
	I_REF(imod::IModel, m_envManagerModelCompPtr);
	I_REF(icomp::IRegistryLoader, m_registryLoaderCompPtr);
	I_REF(IRegistryConsistInfo, m_consistInfoCompPtr);
	I_REF(idoc::IDocumentManager, m_documentManagerCompPtr);

	EnvironmentObserver m_environmentObserver;

	bool m_isSyncingSelection;

	// Temporary state used during tree rebuild to substitute in-memory registry
	// for the active document's file path (avoids stale file-based data)
	QString m_activeDocFilePath;
	const icomp::IRegistry* m_activeDocRegistryPtr;
};


} // namespace icmpstr


#endif // !icmpstr_CComponentTreeComp_included


