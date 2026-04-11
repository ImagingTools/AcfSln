// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icmpstr/CComponentTreeComp.h>


// Qt includes
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

// ACF includes
#include <icomp/CCompositeComponentStaticInfo.h>
#include <icomp/CRegistryElement.h>
#include <ilog/CMessageContainer.h>


namespace icmpstr
{


CComponentTreeComp::CComponentTreeComp()
	:m_environmentObserver(this),
	 m_isSyncingSelection(false)
{
}


// protected methods

void CComponentTreeComp::RebuildRootComboBox()
{
	if (!IsGuiCreated()){
		return;
	}

	RootComboBox->blockSignals(true);

	int previousIndex = RootComboBox->currentIndex();
	QString previousText = RootComboBox->currentText();

	RootComboBox->clear();

	if (m_envManagerCompPtr.IsValid()){
		QStringList projectTargets = m_envManagerCompPtr->GetProjectTargets();

		for (		QStringList::ConstIterator iter = projectTargets.constBegin();
					iter != projectTargets.constEnd();
					++iter){
			const QString& targetPath = *iter;
			QFileInfo fileInfo(targetPath);
			RootComboBox->addItem(fileInfo.baseName(), targetPath);
		}

		// Try to restore previous selection
		if (previousIndex >= 0 && previousIndex < RootComboBox->count()){
			if (RootComboBox->itemText(previousIndex) == previousText){
				RootComboBox->setCurrentIndex(previousIndex);
			}
		}
	}

	RootComboBox->blockSignals(false);

	RebuildTree();
}


void CComponentTreeComp::RebuildTree()
{
	if (!IsGuiCreated()){
		return;
	}

	ComponentTree->clear();

	int currentIndex = RootComboBox->currentIndex();
	if (currentIndex < 0 || !m_registryLoaderCompPtr.IsValid()){
		return;
	}

	QString rootPath = RootComboBox->itemData(currentIndex).toString();
	const icomp::IRegistry* rootRegistryPtr = m_registryLoaderCompPtr->GetRegistryFromFile(rootPath);
	if (rootRegistryPtr == NULL){
		return;
	}

	QTreeWidgetItem* rootItem = new QTreeWidgetItem();
	rootItem->setText(0, RootComboBox->currentText());

	static QIcon okIcon(":/Icons/Ok");
	rootItem->setIcon(0, okIcon);

	ComponentTree->addTopLevelItem(rootItem);

	CreateRegistryTree(*rootRegistryPtr, rootItem);

	rootItem->setExpanded(true);

	SyncSelectionFromModel();
}


void CComponentTreeComp::CreateRegistryTree(
			const icomp::IRegistry& registry,
			QTreeWidgetItem* registryRootItemPtr)
{
	icomp::IRegistry::Ids elementIds = registry.GetElementIds();
	for (		icomp::IRegistry::Ids::iterator iter = elementIds.begin();
				iter != elementIds.end();
				++iter){
		const QByteArray& elementId = *iter;
		const icomp::IRegistry::ElementInfo* elementInfoPtr = registry.GetElementInfo(elementId);
		if ((elementInfoPtr != NULL) && elementInfoPtr->elementPtr.IsValid()){
			AddRegistryElementItem(registry, elementInfoPtr, elementId, registryRootItemPtr);
		}
	}
}


void CComponentTreeComp::AddSubcomponents(
			const icomp::IRegistry& registry,
			const icomp::CComponentAddress& address,
			QTreeWidgetItem* registryElementItemPtr)
{
	if (m_envManagerCompPtr.IsValid()){
		const icomp::IRegistry* componentRegistryPtr = NULL;

		if (!address.GetPackageId().isEmpty()){
			const icomp::IComponentStaticInfo* metaInfoPtr = m_envManagerCompPtr->GetComponentMetaInfo(address);

			if (metaInfoPtr != NULL && (metaInfoPtr->GetComponentType() == icomp::IComponentStaticInfo::CT_COMPOSITE)){
				const icomp::CCompositeComponentStaticInfo* compositeMetaInfoPtr = dynamic_cast<const icomp::CCompositeComponentStaticInfo*>(metaInfoPtr);
				if (compositeMetaInfoPtr != NULL){
					componentRegistryPtr = &compositeMetaInfoPtr->GetRegistry();
				}
			}
		}
		else{
			componentRegistryPtr = registry.GetEmbeddedRegistry(address.GetComponentId());
		}

		if (componentRegistryPtr != NULL){
			CreateRegistryTree(*componentRegistryPtr, registryElementItemPtr);
		}
	}
}


QTreeWidgetItem* CComponentTreeComp::AddRegistryElementItem(
			const icomp::IRegistry& registry,
			const icomp::IRegistry::ElementInfo* elementPtr,
			const QByteArray& elementId,
			QTreeWidgetItem* parentItemPtr)
{
	icomp::CRegistryElement* registryElementPtr = dynamic_cast<icomp::CRegistryElement*>(elementPtr->elementPtr.GetPtr());
	if (registryElementPtr != NULL){
		QTreeWidgetItem* elementItemPtr = new QTreeWidgetItem();

		QByteArray packageId = elementPtr->address.GetPackageId();

		elementItemPtr->setText(0, elementId);
		elementItemPtr->setData(0, DR_ELEMENT_NAME, elementId);
		elementItemPtr->setData(0, DR_ELEMENT_ID, elementPtr->address.GetComponentId());
		elementItemPtr->setData(0, DR_ELEMENT_PACKAGE_ID, packageId);
		elementItemPtr->setData(0, DR_REGISTRY, quintptr(&registry));

		static QIcon okIcon(":/Icons/Ok");
		elementItemPtr->setIcon(0, okIcon);

		if (parentItemPtr != NULL){
			parentItemPtr->addChild(elementItemPtr);
			parentItemPtr->setExpanded(true);
		}

		bool isConsistent = true;

		if (m_consistInfoCompPtr.IsValid()){
			ilog::CMessageContainer messageContainer;
			isConsistent = m_consistInfoCompPtr->IsElementValid(
						elementId,
						registry,
						false,
						true,
						&messageContainer);
		}

		if (!isConsistent){
			static QIcon errorIcon(":/Icons/Warning");
			elementItemPtr->setIcon(0, errorIcon);

			while (parentItemPtr != NULL){
				parentItemPtr->setIcon(0, errorIcon);
				parentItemPtr = parentItemPtr->parent();
			}
		}

		AddSubcomponents(registry, elementPtr->address, elementItemPtr);

		return elementItemPtr;
	}

	return NULL;
}


void CComponentTreeComp::SyncSelectionFromModel()
{
	if (!IsGuiCreated()){
		return;
	}

	const IElementSelectionInfo* selectionInfoPtr = GetObservedObject();
	if (selectionInfoPtr == NULL){
		return;
	}

	IElementSelectionInfo::Elements selectedElements = selectionInfoPtr->GetSelectedElements();
	if (selectedElements.isEmpty()){
		return;
	}

	m_isSyncingSelection = true;

	ComponentTree->clearSelection();

	for (		IElementSelectionInfo::Elements::const_iterator iter = selectedElements.constBegin();
				iter != selectedElements.constEnd();
				++iter){
		const QByteArray& elementName = iter.key();

		QTreeWidgetItem* foundItem = FindTreeItem(elementName);
		if (foundItem != NULL){
			foundItem->setSelected(true);

			// Ensure item is visible by expanding parents
			QTreeWidgetItem* parentPtr = foundItem->parent();
			while (parentPtr != NULL){
				parentPtr->setExpanded(true);
				parentPtr = parentPtr->parent();
			}

			ComponentTree->scrollToItem(foundItem);
		}
	}

	m_isSyncingSelection = false;
}


QTreeWidgetItem* CComponentTreeComp::FindTreeItem(
			const QByteArray& elementName,
			QTreeWidgetItem* parentPtr) const
{
	if (parentPtr == NULL){
		// Search all top-level items
		for (int i = 0; i < ComponentTree->topLevelItemCount(); ++i){
			QTreeWidgetItem* result = FindTreeItem(elementName, ComponentTree->topLevelItem(i));
			if (result != NULL){
				return result;
			}
		}
		return NULL;
	}

	// Check current item
	if (parentPtr->data(0, DR_ELEMENT_NAME).toByteArray() == elementName){
		return parentPtr;
	}

	// Search children
	for (int i = 0; i < parentPtr->childCount(); ++i){
		QTreeWidgetItem* result = FindTreeItem(elementName, parentPtr->child(i));
		if (result != NULL){
			return result;
		}
	}

	return NULL;
}


// reimplemented (iqtgui::TGuiObserverWrap)

void CComponentTreeComp::UpdateGui(const istd::IChangeable::ChangeSet& /*changeSet*/)
{
	SyncSelectionFromModel();
}


void CComponentTreeComp::OnGuiModelAttached()
{
	BaseClass::OnGuiModelAttached();

	SyncSelectionFromModel();
}


void CComponentTreeComp::OnGuiModelDetached()
{
	if (IsGuiCreated()){
		ComponentTree->clearSelection();
	}

	BaseClass::OnGuiModelDetached();
}


// reimplemented (iqtgui::CGuiComponentBase)

void CComponentTreeComp::OnGuiCreated()
{
	BaseClass::OnGuiCreated();

	RebuildRootComboBox();
}


// reimplemented (icomp::CComponentBase)

void CComponentTreeComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_envManagerModelCompPtr.IsValid()){
		m_envManagerModelCompPtr->AttachObserver(&m_environmentObserver);
	}
}


void CComponentTreeComp::OnComponentDestroyed()
{
	if (m_envManagerModelCompPtr.IsValid() && m_envManagerModelCompPtr->IsAttached(&m_environmentObserver)){
		m_envManagerModelCompPtr->DetachObserver(&m_environmentObserver);
	}

	BaseClass::OnComponentDestroyed();
}


// protected slots

void CComponentTreeComp::on_RootComboBox_currentIndexChanged(int /*index*/)
{
	RebuildTree();
}


void CComponentTreeComp::on_ComponentTree_itemDoubleClicked(QTreeWidgetItem* itemPtr, int /*column*/)
{
	icomp::CComponentAddress componentAddress;

	componentAddress.SetComponentId(itemPtr->data(0, DR_ELEMENT_ID).toByteArray());
	componentAddress.SetPackageId(itemPtr->data(0, DR_ELEMENT_PACKAGE_ID).toByteArray());

	if (m_envManagerCompPtr.IsValid() && m_documentManagerCompPtr.IsValid()){
		const icomp::IComponentStaticInfo* metaInfoPtr = m_envManagerCompPtr->GetComponentMetaInfo(componentAddress);

		if (metaInfoPtr != NULL && (metaInfoPtr->GetComponentType() == icomp::IComponentStaticInfo::CT_COMPOSITE)){
			QDir packageDir(m_envManagerCompPtr->GetPackagePath(componentAddress.GetPackageId()));

			QByteArray componentId = componentAddress.GetComponentId();

			QString filePath = packageDir.absoluteFilePath(componentAddress.GetComponentId() + ".acc");
			QString filePathOld = packageDir.absoluteFilePath(componentId + ".arx");
			if (!QFileInfo(filePath).exists()){
				if (QFileInfo(filePathOld).exists()){
					filePath = filePathOld;
				}
			}

			m_documentManagerCompPtr->OpenDocument(NULL, &filePath);
		}
	}
}


void CComponentTreeComp::on_ComponentTree_itemSelectionChanged()
{
	if (m_isSyncingSelection){
		return;
	}

	// When user selects in tree, request selection in the diagram
	const IElementSelectionInfo* selectionInfoPtr = GetObservedObject();
	if (selectionInfoPtr == NULL){
		return;
	}

	QList<QTreeWidgetItem*> selectedItems = ComponentTree->selectedItems();
	if (selectedItems.size() == 1){
		QTreeWidgetItem* itemPtr = selectedItems.first();
		QByteArray elementName = itemPtr->data(0, DR_ELEMENT_NAME).toByteArray();
		if (!elementName.isEmpty()){
			selectionInfoPtr->RequestElementSelection(elementName);
		}
	}
}


// public methods of embedded class EnvironmentObserver

CComponentTreeComp::EnvironmentObserver::EnvironmentObserver(CComponentTreeComp* parentPtr)
:	m_parent(*parentPtr)
{
	Q_ASSERT(parentPtr != NULL);
}


// protected methods of embedded class EnvironmentObserver

// reimplemented (imod::TSingleModelObserverBase)

void CComponentTreeComp::EnvironmentObserver::OnUpdate(const istd::IChangeable::ChangeSet& /*changeSet*/)
{
	m_parent.RebuildRootComboBox();
}


} // namespace icmpstr


