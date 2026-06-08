// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icmpstr/CMultiAttributeEditor.h>


// Qt includes
#include<QtCore/QtGlobal>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLineEdit>
#else
#include <QtGui/QHeaderView>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#endif

// ACF includes
#include <ilog/CMessageContainer.h>

	
namespace icmpstr
{


// public methods

CMultiAttributeEditor::CMultiAttributeEditor(
			QWidget* parentPtr,
			const CElementSelectionInfoManagerBase& elementSelectionInfoManager,
			const QByteArray& attributeId,
			int attributeFlags)
:	QDialog(parentPtr),
	m_elementSelectionInfoManager(elementSelectionInfoManager),
	m_attributeId(attributeId),
	m_attributeFlags(attributeFlags),
	m_valueItemDelegate(*this)
{
	m_dialog.setupUi(this);

	m_dialog.AttributeValuesTree->setItemDelegate(&m_valueItemDelegate);

	connect(m_dialog.AddButton, SIGNAL(clicked()), this, SLOT(OnItemAdd()));
	connect(m_dialog.RemoveButton, SIGNAL(clicked()), this, SLOT(OnItemRemove()));
	connect(m_dialog.UpButton, SIGNAL(clicked()), this, SLOT(OnItemMoveUp()));
	connect(m_dialog.DownButton, SIGNAL(clicked()), this, SLOT(OnItemMoveDown()));
	connect(m_dialog.AttributeValuesTree, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateButtonStates()));
	connect(m_dialog.AttributeValuesTree, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(OnItemChanged(QListWidgetItem*)));
}


void CMultiAttributeEditor::SetEditorText(const QString& attributeValuesText)
{
	QStringList values = attributeValuesText.split(";");

	CreateValuesTree(values);
}


QStringList CMultiAttributeEditor::GetAttributeValues() const
{
	QStringList itemsList;

	int itemsCount = m_dialog.AttributeValuesTree->count();

	for (int itemIndex = 0; itemIndex < itemsCount; itemIndex++){
		itemsList.push_back(m_dialog.AttributeValuesTree->item(itemIndex)->text());
	}

	return itemsList;
}


// protected slots

void CMultiAttributeEditor::OnItemAdd()
{
	int insertIndex = m_dialog.AttributeValuesTree->count();

	QList<QListWidgetItem*> selectedItems = m_dialog.AttributeValuesTree->selectedItems();
	if (!selectedItems.isEmpty()){
		QListWidgetItem* itemPtr = selectedItems.back();
		insertIndex = m_dialog.AttributeValuesTree->row(itemPtr) + 1;
	}

	QListWidgetItem* itemPtr = new QListWidgetItem();
	itemPtr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
	
	itemPtr->setText("");

	m_dialog.AttributeValuesTree->insertItem(insertIndex, itemPtr);
	m_dialog.AttributeValuesTree->editItem(itemPtr);

	UpdateButtonStates();
}


void CMultiAttributeEditor::OnItemRemove()
{
	QList<QListWidgetItem*> selectedItems = m_dialog.AttributeValuesTree->selectedItems();
	if (!selectedItems.isEmpty()){
		QListWidgetItem* itemPtr = selectedItems.back();

		int rowIndex = m_dialog.AttributeValuesTree->row(itemPtr);

		itemPtr = m_dialog.AttributeValuesTree->takeItem(rowIndex);
		Q_ASSERT(itemPtr != NULL);

		delete itemPtr;

		m_dialog.AttributeValuesTree->item(qMax(rowIndex - 1, 0))->setSelected(true);
	}

	UpdateButtonStates();
}


void CMultiAttributeEditor::OnItemMoveUp()
{
	QList<QListWidgetItem*> selectedItems = m_dialog.AttributeValuesTree->selectedItems();
	if (!selectedItems.isEmpty()){
		QListWidgetItem* itemPtr = selectedItems.back();

		int rowIndex = m_dialog.AttributeValuesTree->row(itemPtr);

		itemPtr = m_dialog.AttributeValuesTree->takeItem(rowIndex);
		Q_ASSERT(itemPtr != NULL);

		m_dialog.AttributeValuesTree->insertItem(rowIndex - 1, itemPtr);

		itemPtr->setSelected(true);
	}
}


void CMultiAttributeEditor::OnItemMoveDown()
{
	QList<QListWidgetItem*> selectedItems = m_dialog.AttributeValuesTree->selectedItems();
	if (!selectedItems.isEmpty()){
		QListWidgetItem* itemPtr = selectedItems.back();

		int rowIndex = m_dialog.AttributeValuesTree->row(itemPtr);

		itemPtr = m_dialog.AttributeValuesTree->takeItem(rowIndex);
		Q_ASSERT(itemPtr != NULL);

		m_dialog.AttributeValuesTree->insertItem(rowIndex + 1, itemPtr);

		itemPtr->setSelected(true);
	}
}


void CMultiAttributeEditor::UpdateButtonStates()
{
	bool itemSelected = false;
	int selectedRowIndex = -1;
	QList<QListWidgetItem*> selectedItems = m_dialog.AttributeValuesTree->selectedItems();
	if (!selectedItems.isEmpty()){
		QListWidgetItem* itemPtr = selectedItems.at(0);

		selectedRowIndex = m_dialog.AttributeValuesTree->row(itemPtr);

		itemSelected = true;
	}

	m_dialog.UpButton->setEnabled(itemSelected && selectedRowIndex > 0);
	m_dialog.DownButton->setEnabled(itemSelected && selectedRowIndex < m_dialog.AttributeValuesTree->count() - 1);
	m_dialog.RemoveButton->setEnabled(itemSelected);
}


void CMultiAttributeEditor::OnItemChanged(QListWidgetItem* itemPtr)
{
	Q_ASSERT(itemPtr != NULL);

	UpdateItemState(*itemPtr);
}


// protected methods

void CMultiAttributeEditor::CreateValuesTree(const QStringList& values)
{
	for (int itemIndex = 0; itemIndex < values.count(); itemIndex++){
		QListWidgetItem* itemPtr = new QListWidgetItem();
		itemPtr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
		itemPtr->setText(values[itemIndex]);

		UpdateItemState(*itemPtr);

		m_dialog.AttributeValuesTree->addItem(itemPtr);
	}

	UpdateButtonStates();
}


void CMultiAttributeEditor::UpdateItemState(QListWidgetItem& item)
{
	QStringList textMessages;
	bool isOk = true;

	const icomp::IRegistry* registryPtr = m_elementSelectionInfoManager.GetRegistry();
	const IRegistryConsistInfo* consistInfoPtr = m_elementSelectionInfoManager.GetConsistencyInfoPtr();
	if ((registryPtr != NULL) && (consistInfoPtr != NULL)){
		const IElementSelectionInfo* selectionInfoPtr = m_elementSelectionInfoManager.GetObservedObject();
		if (selectionInfoPtr == NULL){
			return;
		}

		IElementSelectionInfo::Elements selectedElements = selectionInfoPtr->GetSelectedElements();
		if (selectedElements.isEmpty()){
			return;
		}

		icomp::IElementStaticInfo::Ids obligatoryInterfaces;
		icomp::IElementStaticInfo::Ids optionalInterfaces;

		for (		IElementSelectionInfo::Elements::const_iterator elemIter = selectedElements.begin();
					elemIter != selectedElements.end();
					++elemIter){
			const icomp::IRegistry::ElementInfo* elementInfoPtr = elemIter.value();
			Q_ASSERT(elementInfoPtr != NULL);
			if (!elementInfoPtr->elementPtr.IsValid()){
				continue;
			}

			const iser::IObject* attributePtr = m_elementSelectionInfoManager.GetAttributeObject(m_attributeId, *elementInfoPtr);
			if (attributePtr == NULL){
				continue;
			}

			if ((m_attributeFlags & (icomp::IAttributeStaticInfo::AF_REFERENCE | icomp::IAttributeStaticInfo::AF_FACTORY)) != 0){
				const icomp::IAttributeStaticInfo* staticInfoPtr = m_elementSelectionInfoManager.GetAttributeStaticInfo(m_attributeId, *elementInfoPtr);
				if (staticInfoPtr != NULL){
					obligatoryInterfaces += staticInfoPtr->GetRelatedMetaIds(icomp::IComponentStaticInfo::MGI_INTERFACES, icomp::IAttributeStaticInfo::AF_OBLIGATORY, icomp::IAttributeStaticInfo::AF_OBLIGATORY);
					optionalInterfaces += staticInfoPtr->GetRelatedMetaIds(icomp::IComponentStaticInfo::MGI_INTERFACES, 0, icomp::IAttributeStaticInfo::AF_OBLIGATORY);
				}
			}
		}

		ilog::CMessageContainer messageContainer;

		isOk = consistInfoPtr->IsAttributeValueValid(
					m_attributeFlags,
					item.text().toLatin1(),
					obligatoryInterfaces,
					optionalInterfaces,
					*registryPtr,
					true,
					&messageContainer);

		ilog::IMessageContainer::Messages messages = messageContainer.GetMessages();
		for (		ilog::IMessageContainer::Messages::const_iterator messageIter = messages.cbegin();
					messageIter != messages.cend();
					++messageIter){
			const ilog::IMessageConsumer::MessagePtr messagePtr = *messageIter;
			if (messagePtr.IsValid()){
				QString messageText;

				switch (messagePtr->GetInformationCategory()){
				case istd::IInformationProvider::IC_INFO:
					messageText = tr("Information: ");
					break;

				case istd::IInformationProvider::IC_WARNING:
					messageText = tr("Warning: ");
					break;

				case istd::IInformationProvider::IC_ERROR:
					messageText = tr("Error: ");
					break;

				case istd::IInformationProvider::IC_CRITICAL:
					messageText = tr("Critical error: ");
					break;

				default:
					break;
				}

				messageText += messagePtr->GetInformationDescription();

				textMessages += messageText;
			}
		}
	}

	item.setBackground(isOk? Qt::transparent: Qt::red);
	item.setToolTip(textMessages.join("\n"));
}


// public methods of embedded class ValueItemDelegate

CMultiAttributeEditor::ValueItemDelegate::ValueItemDelegate(const CMultiAttributeEditor& parent)
:	m_parent(parent)
{
}


// reimplemented (QItemDelegate)

QWidget* CMultiAttributeEditor::ValueItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (		((m_parent.m_attributeFlags & icomp::IAttributeStaticInfo::AF_REFERENCE) != 0) ||
				((m_parent.m_attributeFlags & icomp::IAttributeStaticInfo::AF_FACTORY) != 0)){
		QComboBox* comboEditor = new QComboBox(parent);
		QString text = index.data().toString();
		comboEditor->setEditable(true);
		comboEditor->setEditText(text);
	
		return comboEditor;
	} 

	return BaseClass::createEditor(parent, option, index);
}


void CMultiAttributeEditor::ValueItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	const IElementSelectionInfo* selectionInfoPtr = m_parent.m_elementSelectionInfoManager.GetObservedObject();
	if (selectionInfoPtr == NULL){
		return;
	}

	IElementSelectionInfo::Elements selectedElements = selectionInfoPtr->GetSelectedElements();
	if (selectedElements.isEmpty()){
		return;
	}

	const IRegistryConsistInfo* consistInfoPtr = m_parent.m_elementSelectionInfoManager.GetConsistencyInfoPtr();

	QByteArray attributeId = m_parent.m_attributeId;

	for (		IElementSelectionInfo::Elements::const_iterator elemIter = selectedElements.begin();
				elemIter != selectedElements.end();
				++elemIter){
		const icomp::IRegistry::ElementInfo* elementInfoPtr = elemIter.value();
		Q_ASSERT(elementInfoPtr != NULL);
		if (!elementInfoPtr->elementPtr.IsValid()){
			continue;
		}

		const iser::IObject* attributePtr = m_parent.m_elementSelectionInfoManager.GetAttributeObject(attributeId, *elementInfoPtr);
		if (attributePtr == NULL){
			continue;
		}

		if ((m_parent.m_attributeFlags & (icomp::IAttributeStaticInfo::AF_REFERENCE | icomp::IAttributeStaticInfo::AF_FACTORY)) != 0){
			QComboBox* comboEditor = dynamic_cast<QComboBox*>(editor);
			if (comboEditor == NULL){
				return;
			}

			const icomp::IRegistry* registryPtr = m_parent.m_elementSelectionInfoManager.GetRegistry();
			const icomp::IAttributeStaticInfo* staticInfoPtr = m_parent.m_elementSelectionInfoManager.GetAttributeStaticInfo(attributeId, *elementInfoPtr);
			if ((registryPtr != NULL) && (staticInfoPtr != NULL) && (consistInfoPtr != NULL)){
				// prepare queryFlags
				int queryFlags = IRegistryConsistInfo::QF_NONE;

				icomp::IElementStaticInfo::Ids obligatoryInterfaces = staticInfoPtr->GetRelatedMetaIds(
							icomp::IComponentStaticInfo::MGI_INTERFACES,
							0,
							icomp::IAttributeStaticInfo::AF_NULLABLE);	// Names of the interfaces which must be set
				if (obligatoryInterfaces.isEmpty()){
					obligatoryInterfaces = staticInfoPtr->GetRelatedMetaIds(icomp::IComponentStaticInfo::MGI_INTERFACES, 0, 0);	// All asked interface names
					queryFlags = queryFlags | IRegistryConsistInfo::QF_ANY_INTERFACE;	// for optional interfaces only we are looking for any of them
				}

				int attributeFlags = m_parent.m_attributeFlags;
				if ((attributeFlags & icomp::IAttributeStaticInfo::AF_REFERENCE) != 0){
					queryFlags = queryFlags | IRegistryConsistInfo::QF_INCLUDE_SUBELEMENTS;
				}
				else if ((attributeFlags & icomp::IAttributeStaticInfo::AF_FACTORY) != 0){
					queryFlags = queryFlags | IRegistryConsistInfo::QF_DETACHED_FROM_CONTAINER;
				}

				icomp::IRegistry::Ids compatIds = consistInfoPtr->GetCompatibleElements(
							obligatoryInterfaces,
							*registryPtr,
							queryFlags);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
				QList< QByteArray> compatIdList(compatIds.begin(), compatIds.end());
#else
				QList<QByteArray> compatIdList = compatIds.toList();
#endif

				std::sort(compatIdList.begin(), compatIdList.end());
				
				for(		QList< QByteArray>::const_iterator iter = compatIdList.begin();
							iter != compatIdList.end();
							++iter){
					comboEditor->addItem(*iter);
				}
			}

			comboEditor->setEditText(index.data().toString());

			return;
		}
	}

	BaseClass::setEditorData(editor, index);
}


void CMultiAttributeEditor::ValueItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	QComboBox* comboEditor = dynamic_cast<QComboBox*>(editor);
	if (comboEditor != NULL){
		QMap<int,QVariant> dataMap;
		dataMap.insert(Qt::DisplayRole, comboEditor->lineEdit()->text());
		model->setItemData(index, dataMap);

		return;
	}

	BaseClass::setModelData(editor, model, index);
}


} // namespace icmpstr


