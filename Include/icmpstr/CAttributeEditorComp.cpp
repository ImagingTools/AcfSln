// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icmpstr/CAttributeEditorComp.h>


// Qt includes
#if QT_VERSION >= 0x050000
#include <QtWidgets/QComboBox>
#include <QtWidgets/QItemDelegate>
#include <QtWidgets/QLineEdit>
#else
#include <QtGui/QComboBox>
#include <QtGui/QItemDelegate>
#include <QtGui/QLineEdit>
#endif

// ACF includes
#include <istd/TOptDelPtr.h>
#include <istd/CChangeNotifier.h>
#include <icomp/CInterfaceManipBase.h>
#include <icomp/TMultiAttributeMember.h>
#include <icomp/CCompositeComponentStaticInfo.h>
#include <icomp/CComponentMetaDescriptionEncoder.h>
#include <ilog/CMessageContainer.h>
#include <iqt/CSignalBlocker.h>

// ACF-Solutions includes
#include <icmpstr/CMultiAttributeDelegateWidget.h>
#include <icmpstr/CRegistryConsistInfoComp.h>



namespace icmpstr
{


const istd::IChangeable::ChangeSet s_changeAttributeChangeSet(icomp::IRegistryElement::CF_ATTRIBUTE_CHANGED, QObject::tr("Change attribute"));
const istd::IChangeable::ChangeSet s_importChangeSet(icomp::IRegistryElement::CF_ATTRIBUTE_CHANGED, QObject::tr("Import/export interface"));
const istd::IChangeable::ChangeSet s_changeFlagChangeSet(icomp::IRegistryElement::CF_FLAGS_CHANGED, QObject::tr("Change flag"));
const istd::IChangeable::ChangeSet s_setAttributeChangeSet(icomp::IRegistryElement::CF_ATTRIBUTE_CHANGED, QObject::tr("Set attribute value"));
const istd::IChangeable::ChangeSet s_setExportChangeSet(icomp::IRegistry::CF_ELEMENT_EXPORTED, QObject::tr("Set export name"));


// public methods

CAttributeEditorComp::CAttributeEditorComp()
:	m_attributeItemDelegate(this),
	m_registryObserver(this),
	m_lastRegistryModelPtr(NULL)
{
	m_attributeTypesMap[iattr::CIntegerAttribute::GetTypeName()] = TypeDescr(tr("Integer number"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CRealAttribute::GetTypeName()] = TypeDescr(tr("Real number"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CBooleanAttribute::GetTypeName()] = TypeDescr(tr("Boolean value"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CStringAttribute::GetTypeName()] = TypeDescr(tr("String"), AGT_ATTRIBUTE);
	m_attributeTypesMap[icomp::CTextAttribute::GetTypeName()] = TypeDescr(tr("Translatable text"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CIdAttribute::GetTypeName()] = TypeDescr(tr("ID"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CIntegerListAttribute::GetTypeName()] = TypeDescr(tr("List of integer numbers"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CRealListAttribute::GetTypeName()] = TypeDescr(tr("List of real numbers"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CBooleanListAttribute::GetTypeName()] = TypeDescr(tr("List of boolean values"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CStringListAttribute::GetTypeName()] = TypeDescr(tr("List of strings"), AGT_ATTRIBUTE);
	m_attributeTypesMap[icomp::CMultiTextAttribute::GetTypeName()] = TypeDescr(tr("List of translatable texts"), AGT_ATTRIBUTE);
	m_attributeTypesMap[iattr::CIdListAttribute::GetTypeName()] = TypeDescr(tr("List of ID's"), AGT_ATTRIBUTE);
	m_attributeTypesMap[icomp::CReferenceAttribute::GetTypeName()] = TypeDescr(tr("Component reference"), AGT_REFERENCE);
	m_attributeTypesMap[icomp::CMultiReferenceAttribute::GetTypeName()] = TypeDescr(tr("List of component reference"), AGT_REFERENCE);
	m_attributeTypesMap[icomp::CFactoryAttribute::GetTypeName()] = TypeDescr(tr("Component factory"), AGT_FACTORY);
	m_attributeTypesMap[icomp::CMultiFactoryAttribute::GetTypeName()] = TypeDescr(tr("List of component factory"), AGT_FACTORY);

	QObject::connect(this, SIGNAL(AfterAttributesChange()), this, SLOT(UpdateAttributesView()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(AfterInterfacesChange()), this, SLOT(UpdateInterfacesView()), Qt::QueuedConnection);
	QObject::connect(this, SIGNAL(AfterSubcomponentsChange()), this, SLOT(UpdateSubcomponentsView()), Qt::QueuedConnection);
}



// reimplemented (CElementSelectionInfoManagerBase)

const icomp::IMetaInfoManager* CAttributeEditorComp::GetMetaInfoManagerPtr() const
{
	return m_metaInfoManagerCompPtr.GetPtr();
}


const icmpstr::IRegistryConsistInfo* CAttributeEditorComp::GetConsistencyInfoPtr() const
{
	return m_consistInfoCompPtr.GetPtr();
}


// protected slots

void CAttributeEditorComp::on_AttributeTree_itemSelectionChanged()
{
	if (!m_metaInfoManagerCompPtr.IsValid()){
		return;
	}

	const icomp::IAttributeStaticInfo* attributeStaticInfoPtr = NULL;

	QList<QTreeWidgetItem*> items = AttributeTree->selectedItems();
	if (!items.isEmpty()){
		QTreeWidgetItem* attributeItemPtr = items.front();

		QByteArray attributeId = attributeItemPtr->data(AC_VALUE, AttributeId).toString().toLocal8Bit();
		if (!attributeId.isEmpty()){
			const IElementSelectionInfo* selectionInfoPtr = GetObservedObject();
			if (selectionInfoPtr != NULL){
				IElementSelectionInfo::Elements selectedElements = selectionInfoPtr->GetSelectedElements();
				if (!selectedElements.isEmpty()){
					const icomp::IRegistry::ElementInfo* firstElementInfo = selectedElements.begin().value();
					Q_ASSERT(firstElementInfo != NULL);

					const icomp::IComponentStaticInfo* componentInfoPtr = m_metaInfoManagerCompPtr->GetComponentMetaInfo(firstElementInfo->address);
					if (componentInfoPtr != NULL){
						attributeStaticInfoPtr = componentInfoPtr->GetAttributeInfo(attributeId);
					}
				}
			}
		}
	}
	else{
		AttributeTree->reset();
	}

	if (m_attributeSelectionObserverCompPtr.IsValid()){
		m_attributeSelectionObserverCompPtr->OnAttributeSelected(attributeStaticInfoPtr);
	}
}


void CAttributeEditorComp::on_AttributeTree_itemChanged(QTreeWidgetItem* item, int column)
{
	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr == NULL){
		return;
	}

	icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();
	if (registryPtr == NULL){
		return;
	}

	if (IsUpdateBlocked()){
		return;
	}

	UpdateBlocker updateBlocker(this);

	QByteArray attributeId = item->data(AC_VALUE, AttributeId).toString().toLocal8Bit();
	int attributeStatMeaning = item->data(AC_VALUE, AttributeMining).toInt();

	if (column == AC_NAME){
		istd::CChangeNotifier registryNotifier(registryPtr, &s_changeAttributeChangeSet);
		Q_UNUSED(registryNotifier);

		IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();
		for (		IElementSelectionInfo::Elements::ConstIterator iter = selectedElements.constBegin();
					iter != selectedElements.constEnd();
					++iter){
			const icomp::IRegistry::ElementInfo* selectedInfoPtr = iter.value();
			Q_ASSERT(selectedInfoPtr != NULL);

			icomp::IRegistryElement* elementPtr = selectedInfoPtr->elementPtr.Cast<icomp::IRegistryElement*>();
			if (elementPtr == NULL){
				continue;
			}

			istd::CChangeNotifier elementNotifier(elementPtr, &s_changeAttributeChangeSet);
			Q_UNUSED(elementNotifier);

			if (item->checkState(AC_NAME) == Qt::Unchecked){
				icomp::IRegistryElement::AttributeInfo* attributeInfoPtr =
							const_cast<icomp::IRegistryElement::AttributeInfo*>(elementPtr->GetAttributeInfo(attributeId));
				if (attributeInfoPtr != NULL){
					if ((attributeStatMeaning >= AM_REFERENCE) && (attributeStatMeaning <= AM_MULTI_ATTRIBUTE)){
						attributeInfoPtr->attributePtr.Reset();
					}
					else if (attributeStatMeaning == AM_EXPORTED_ATTR){
						attributeInfoPtr->exportId = "";
					}

					if (!attributeInfoPtr->attributePtr.IsValid() && attributeInfoPtr->exportId.isEmpty()){
						elementPtr->RemoveAttribute(attributeId);
					}
				}
			}
		}

		Q_EMIT AfterAttributesChange();
	}
}


void CAttributeEditorComp::on_InterfacesTree_itemSelectionChanged()
{
	QList<QTreeWidgetItem*> items = InterfacesTree->selectedItems();
	if ((items.count() > 0) && m_quickHelpViewerCompPtr.IsValid()){
		QTreeWidgetItem* attributeItemPtr = items.at(0);

		istd::CClassInfo info(attributeItemPtr->data(0, InterfaceName).toString().toLocal8Bit());

		m_quickHelpViewerCompPtr->ShowHelp(info.GetName(), &info);
	}
}


void CAttributeEditorComp::on_InterfacesTree_itemChanged(QTreeWidgetItem* item, int column)
{
	Q_ASSERT(item != NULL);

	if (IsUpdateBlocked()){
		return;
	}

	UpdateBlocker updateBlocker(this);

	if (column == 0){
		const IElementSelectionInfo* selectionInfoPtr = GetObservedObject();
		if (selectionInfoPtr == NULL){
			return;
		}

		icomp::IRegistry* registryPtr = selectionInfoPtr->GetSelectedRegistry();
		if (registryPtr == NULL){
			return;
		}

		istd::CChangeNotifier registryNotifier(registryPtr, &s_importChangeSet);
		Q_UNUSED(registryNotifier);

		QString interfaceName = item->data(0, InterfaceName).toString();
		QByteArray elementName = item->data(0, ElementId).toString().toLocal8Bit();

		bool isSelected = (item->checkState(column) == Qt::Checked);
		registryPtr->SetElementInterfaceExported(elementName, interfaceName.toLocal8Bit(), isSelected);

		Q_EMIT AfterInterfacesChange();
	}
}


void CAttributeEditorComp::on_AutoInstanceCB_toggled(bool checked)
{
	if (IsUpdateBlocked()){
		return;
	}

	UpdateBlocker updateBlocker(this);

	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr == NULL){
		return;
	}

	icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();
	if (registryPtr == NULL){
		return;
	}

	istd::CChangeNotifier registryNotifier(registryPtr, &s_changeFlagChangeSet);
	Q_UNUSED(registryNotifier);

	IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();
	for (		IElementSelectionInfo::Elements::ConstIterator iter = selectedElements.constBegin();
				iter != selectedElements.constEnd();
				++iter){
		const icomp::IRegistry::ElementInfo* selectedInfoPtr = iter.value();
		Q_ASSERT(selectedInfoPtr != NULL);

		if (!selectedInfoPtr->elementPtr.IsValid()){
			continue;
		}

		istd::CChangeNotifier elementNotifier(selectedInfoPtr->elementPtr.GetPtr(), &s_changeFlagChangeSet);
		Q_UNUSED(elementNotifier);

		quint32 flags = selectedInfoPtr->elementPtr->GetElementFlags();

		flags = checked?
					(flags | icomp::IRegistryElement::EF_AUTO_INSTANCE):
					(flags & ~icomp::IRegistryElement::EF_AUTO_INSTANCE);

		selectedInfoPtr->elementPtr->SetElementFlags(flags);
	}
}


void CAttributeEditorComp::on_IsDetachedCB_toggled(bool checked)
{
	if (IsUpdateBlocked()){
		return;
	}

	UpdateBlocker updateBlocker(this);

	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr == NULL){
		return;
	}

	icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();
	if (registryPtr == NULL){
		return;
	}

	istd::CChangeNotifier registryNotifier(registryPtr, &s_changeFlagChangeSet);
	Q_UNUSED(registryNotifier);

	IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();
	for (		IElementSelectionInfo::Elements::ConstIterator iter = selectedElements.constBegin();
				iter != selectedElements.constEnd();
				++iter){
		const icomp::IRegistry::ElementInfo* selectedInfoPtr = iter.value();
		Q_ASSERT(selectedInfoPtr != NULL);

		if (!selectedInfoPtr->elementPtr.IsValid()){
			continue;
		}

		istd::CChangeNotifier elementNotifier(selectedInfoPtr->elementPtr.GetPtr(), &s_changeFlagChangeSet);
		Q_UNUSED(elementNotifier);

		quint32 flags = selectedInfoPtr->elementPtr->GetElementFlags();

		flags = checked?
					(flags | icomp::IRegistryElement::EF_IS_DETACHED):
					(flags & ~icomp::IRegistryElement::EF_IS_DETACHED);

		selectedInfoPtr->elementPtr->SetElementFlags(flags);
	}

	UpdateInterfacesView();
	UpdateSubcomponentsView();
}


void CAttributeEditorComp::UpdateGeneralView()
{
	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr == NULL){
		return;
	}

	const icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();
	if (registryPtr == NULL){
		return;
	}

	QString names;
	QString description;
	QStringList companyInfoList;
	QStringList projectInfoList;
	QStringList authorInfoList;
	QStringList categoryInfoList;
	QStringList tagInfoList;
	QStringList keywordInfoList;

	icomp::CComponentAddress commonAddress;
	bool isMultiType = false;

	IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();
	for (		IElementSelectionInfo::Elements::ConstIterator iter = selectedElements.constBegin();
				iter != selectedElements.constEnd();
				++iter){
		const icomp::IRegistry::ElementInfo* selectedInfoPtr = iter.value();
		Q_ASSERT(selectedInfoPtr != NULL);

		const icomp::IRegistryElement* elementPtr = selectedInfoPtr->elementPtr.GetPtr();
		if (elementPtr != NULL){
			const QByteArray& elementId = iter.key();

			if (!names.isEmpty()){
				names += "\n";
			}
			names += elementId;
		}

		const icomp::CComponentAddress& address = selectedInfoPtr->address;

		if (m_metaInfoManagerCompPtr.IsValid()){
			const icomp::IComponentStaticInfo* infoPtr = m_metaInfoManagerCompPtr->GetComponentMetaInfo(address);
			if (infoPtr != NULL){
				description = infoPtr->GetDescription();

				icomp::CComponentMetaDescriptionEncoder encoder(infoPtr->GetKeywords());

				companyInfoList += (encoder.GetValues("Company"));
				projectInfoList += (encoder.GetValues("Project"));
				authorInfoList += (encoder.GetValues("Author"));
				categoryInfoList += (encoder.GetValues("Category"));
				tagInfoList += (encoder.GetValues("Tag"));
				keywordInfoList += (encoder.GetUnassignedKeywords());
			}
			else{
				companyInfoList += tr("<unknown>");
				projectInfoList += tr("<unknown>");
				authorInfoList += tr("<unknown>");
				categoryInfoList += tr("<unknown>");
				tagInfoList += tr("<unknown>");
				keywordInfoList += tr("<unknown>");
			}
		}

		if (!commonAddress.IsValid()){
			commonAddress = address;
		}
		else if (commonAddress != address){
			isMultiType = true;
		}
	}

	if (!description.isEmpty() && !isMultiType){
		DescriptionLabel->setText(description);
		DescriptionLabel->setVisible(true);
	}
	else{
		DescriptionLabel->setVisible(false);
	}

	QIcon componentIcon;
	if (commonAddress.IsValid()){
		if (isMultiType){
			AddressLabel->setText(tr("<multiple component types>"));
		}
		else{
			AddressLabel->setText(commonAddress.ToString());
			if (m_consistInfoCompPtr.IsValid()){
				componentIcon = m_consistInfoCompPtr->GetComponentIcon(commonAddress);
			}
		}

		AddressLabel->setVisible(true);
	}
	else{
		AddressLabel->setVisible(false);
	}
	if (!componentIcon.isNull()){
		IconLabel->setPixmap(componentIcon.pixmap(128));
		IconLabel->setVisible(true);
	}
	else{
		IconLabel->setVisible(false);
	}

	companyInfoList.removeDuplicates();
	CompanyLabel->setText(companyInfoList.join(", "));
	projectInfoList.removeDuplicates();
	ProjectLabel->setText(projectInfoList.join(", "));
	authorInfoList.removeDuplicates();
	AuthorLabel->setText(authorInfoList.join(", "));
	categoryInfoList.removeDuplicates();
	CategoryLabel->setText(categoryInfoList.join(", "));
	tagInfoList.removeDuplicates();
	TagsLabel->setText(tagInfoList.join(", "));
	keywordInfoList.removeDuplicates();
	KeywordsLabel->setText(keywordInfoList.join(", "));

	NameLabel->setText(names);

	MetaInfoFrame->setVisible(!selectedElements.isEmpty());
}


void CAttributeEditorComp::UpdateAttributesView()
{
	if (!IsGuiCreated()){
		return;
	}

	iqt::CSignalBlocker signalBlocker(AttributeTree);

	m_attrInfosMap.clear();

	bool hasError = false;
	bool hasWarning = false;
	bool hasImport = false;
	int itemIndex = 0;

	QFont normalFont = AttributeTree->font();
	QFont importantFont = normalFont;
	importantFont.setBold(true);

	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr != NULL){
		const icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();
		if (registryPtr != NULL){
			IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();
			for (		IElementSelectionInfo::Elements::ConstIterator iter = selectedElements.constBegin();
						iter != selectedElements.constEnd();
						++iter){
				const QByteArray& elementId = iter.key();

				// check general element consistency
				if (m_consistInfoCompPtr.IsValid()){
					hasError = hasError || !m_consistInfoCompPtr->IsElementValid(
								elementId,
								*registryPtr,
								true,
								false,
								NULL);
				}

				const icomp::IRegistry::ElementInfo* selectedInfoPtr = iter.value();
				Q_ASSERT(selectedInfoPtr != NULL);

				// creating map of attributes based on registry element data
				icomp::IRegistryElement* elementPtr = selectedInfoPtr->elementPtr.GetPtr();
				if (elementPtr != NULL){
					iattr::IAttributesProvider::AttributeIds attributeIds = elementPtr->GetAttributeIds();
					for (		iattr::IAttributesProvider::AttributeIds::ConstIterator attrIter = attributeIds.constBegin();
								attrIter != attributeIds.constEnd();
								++attrIter){
						const QByteArray& attributeId = *attrIter;

						icomp::IRegistryElement::AttributeInfo* attributeInfoPtr = const_cast<icomp::IRegistryElement::AttributeInfo*>(elementPtr->GetAttributeInfo(attributeId));
						if (attributeInfoPtr != NULL){
							AttrInfo& attrInfo = m_attrInfosMap[attributeId][elementId];

							attrInfo.elementPtr = elementPtr;
							attrInfo.infoPtr = attributeInfoPtr;
						}
					}
				}

				// creating map of attributes based on static meta info
				if (m_metaInfoManagerCompPtr.IsValid()){
					const icomp::IComponentStaticInfo* infoPtr = m_metaInfoManagerCompPtr->GetComponentMetaInfo(selectedInfoPtr->address);
					if (infoPtr != NULL){
						iattr::IAttributesProvider::AttributeIds attributeIds = infoPtr->GetAttributeMetaIds();
						for (		iattr::IAttributesProvider::AttributeIds::ConstIterator attrIter = attributeIds.constBegin();
									attrIter != attributeIds.constEnd();
									++attrIter){
							const QByteArray& attributeId = *attrIter;

							AttrInfo& attrInfo = m_attrInfosMap[attributeId][elementId];

							Q_ASSERT(!((attrInfo.elementPtr != NULL) && (elementPtr == NULL)));	// check if we dont reset existing element pointer, it shouldn't happen
							attrInfo.elementPtr = elementPtr;
							attrInfo.staticInfoPtr = infoPtr->GetAttributeInfo(attributeId);
							attrInfo.componentStaticInfoPtr = infoPtr;
						}
					}
				}
			}

			for (int groupType = 0; groupType <= AGT_LAST; ++groupType){
				for (		AttrInfosMap::ConstIterator treeIter = m_attrInfosMap.constBegin();
							treeIter != m_attrInfosMap.constEnd();
							++treeIter){
					const QByteArray& attributeId = treeIter.key();
					const ElementIdToAttrInfoMap& attrInfos = treeIter.value();

					bool errorFlag = false;
					bool warningFlag = false;
					bool exportFlag = false;
					SetAttributeToItem(
								AttributeGroupType(groupType),
								itemIndex,
								*registryPtr,
								attributeId,
								attrInfos,
								normalFont, importantFont,
								errorFlag, warningFlag, exportFlag);

					hasError = hasError || errorFlag;
					hasWarning = hasWarning || warningFlag;
					hasImport = hasImport || exportFlag;
				}
			}
		}
	}

	while (itemIndex < AttributeTree->topLevelItemCount()){
		delete AttributeTree->topLevelItem(itemIndex);
	}

	AttributeTree->resizeColumnToContents(AC_NAME);

	if (hasError){
		ElementInfoTab->setTabIcon(TI_ATTRIBUTES, m_invalidIcon);
	}
	else if (hasWarning){
		ElementInfoTab->setTabIcon(TI_ATTRIBUTES, m_warningIcon);
	}
	else if (hasImport){
		ElementInfoTab->setTabIcon(TI_ATTRIBUTES, m_importIcon);
	}
	else{
		ElementInfoTab->setTabIcon(TI_ATTRIBUTES, QIcon());
	}
}


void CAttributeEditorComp::UpdateInterfacesView()
{
	if (!IsGuiCreated()){
		return;
	}

	iqt::CSignalBlocker signalBlocker(InterfacesTree);

	bool hasWarning = false;
	bool hasExport = false;
	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr != NULL){
		const icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();
		if (registryPtr != NULL){
			icomp::IRegistry::ExportedInterfacesMap registryInterfaces = registryPtr->GetExportedInterfacesMap();

			IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();

			int itemIndex = 0;

			for (		IElementSelectionInfo::Elements::ConstIterator iter = selectedElements.constBegin();
						iter != selectedElements.constEnd();
						++iter){
				const icomp::IRegistry::ElementInfo* selectedInfoPtr = iter.value();
				Q_ASSERT(selectedInfoPtr != NULL);
				Q_ASSERT(selectedInfoPtr->elementPtr.IsValid());

				const QByteArray& elementId = iter.key();

				QTreeWidgetItem* itemPtr = NULL;
				if (selectedElements.size() > 1){
					if (itemIndex < InterfacesTree->topLevelItemCount()){
						itemPtr = InterfacesTree->topLevelItem(itemIndex);
					}
					else{
						itemPtr = new QTreeWidgetItem();
						InterfacesTree->addTopLevelItem(itemPtr);
					}
					itemIndex++;
					Q_ASSERT(itemPtr != NULL);

					ResetItem(*itemPtr);
					itemPtr->setText(0, elementId);
				}

				if (m_metaInfoManagerCompPtr.IsValid()){
					const icomp::IComponentStaticInfo* staticInfoPtr = m_metaInfoManagerCompPtr->GetComponentMetaInfo(selectedInfoPtr->address);
					bool readOnly = ((selectedInfoPtr->elementPtr->GetElementFlags() & icomp::IRegistryElement::EF_IS_DETACHED) != 0);

					bool warningFlag = false;
					bool exportFlag = false;
					CreateInterfacesTree(
								elementId,
								staticInfoPtr,
								registryInterfaces,
								itemPtr,
								warningFlag,
								exportFlag,
								true,
								readOnly);
					hasWarning = hasWarning || warningFlag;
					hasExport = hasExport || exportFlag;
				}
			}

			if (selectedElements.size() != 1){
				while (itemIndex < InterfacesTree->topLevelItemCount()){
					delete InterfacesTree->topLevelItem(itemIndex);
				}
			}
		}
	}

	if (hasWarning){
		ElementInfoTab->setTabIcon(TI_INTERFACES, m_warningIcon);
	}
	else if (hasExport){
		ElementInfoTab->setTabIcon(TI_INTERFACES, m_exportIcon);
	}
	else{
		ElementInfoTab->setTabIcon(TI_INTERFACES, QIcon());
	}
}


void CAttributeEditorComp::UpdateFlagsView()
{
	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr == NULL){
		return;
	}

	const icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();
	if (registryPtr == NULL){
		return;
	}

	IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();
	bool autoInstanceOn = false;
	bool autoInstanceOff = false;

	bool isDetachedOn = false;
	bool isDetachedOff = false;

	for (		IElementSelectionInfo::Elements::ConstIterator iter = selectedElements.constBegin();
				iter != selectedElements.constEnd();
				++iter){
		const icomp::IRegistry::ElementInfo* selectedInfoPtr = iter.value();
		Q_ASSERT(selectedInfoPtr != NULL);

		const icomp::IRegistryElement* elementPtr = selectedInfoPtr->elementPtr.GetPtr();
		if (elementPtr != NULL){
			quint32 elementFlags = elementPtr->GetElementFlags();

			if ((elementFlags & icomp::IRegistryElement::EF_AUTO_INSTANCE) != 0){
				autoInstanceOn = true;
			}
			else{
				autoInstanceOff = true;
			}

			if ((elementFlags & icomp::IRegistryElement::EF_IS_DETACHED) != 0){
				isDetachedOn = true;
			}
			else{
				isDetachedOff = true;
			}
		}
	}

	if (autoInstanceOn){
		if (autoInstanceOff){
			AutoInstanceCB->setCheckState(Qt::PartiallyChecked);
		}
		else{
			AutoInstanceCB->setTristate(false);
			AutoInstanceCB->setCheckState(Qt::Checked);
		}
		AutoInstanceCB->setEnabled(true);
	}
	else{
		if (autoInstanceOff){
			AutoInstanceCB->setTristate(false);
			AutoInstanceCB->setCheckState(Qt::Unchecked);
			AutoInstanceCB->setEnabled(true);
		}
		else{
			AutoInstanceCB->setCheckState(Qt::PartiallyChecked);
			AutoInstanceCB->setEnabled(false);
		}
	}

	if (isDetachedOn){
		if (isDetachedOff){
			IsDetachedCB->setCheckState(Qt::PartiallyChecked);
		}
		else{
			IsDetachedCB->setTristate(false);
			IsDetachedCB->setCheckState(Qt::Checked);
		}
		IsDetachedCB->setEnabled(true);
	}
	else{
		if (isDetachedOff){
			IsDetachedCB->setTristate(false);
			IsDetachedCB->setCheckState(Qt::Unchecked);
			IsDetachedCB->setEnabled(true);
		}
		else{
			IsDetachedCB->setCheckState(Qt::PartiallyChecked);
			IsDetachedCB->setEnabled(false);
		}
	}
}


void CAttributeEditorComp::UpdateSubcomponentsView()
{
	if (!IsGuiCreated()){
		return;
	}

	iqt::CSignalBlocker signalBlocker(ComponentsTree);

	bool hasWarning = false;
	bool hasExport = false;
	int itemIndex = 0;

	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr != NULL){
		const icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();
		if (registryPtr != NULL){
			IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();
			for (		IElementSelectionInfo::Elements::ConstIterator iter = selectedElements.constBegin();
						iter != selectedElements.constEnd();
						++iter){
				const QByteArray& elementId = iter.key();
				const icomp::IRegistry::ElementInfo* selectedInfoPtr = iter.value();
				Q_ASSERT(selectedInfoPtr != NULL);
				Q_ASSERT(selectedInfoPtr->elementPtr.IsValid());

				const icomp::IRegistryElement* elementPtr = selectedInfoPtr->elementPtr.GetPtr();
				if (elementPtr != NULL){
					QTreeWidgetItem* componentRootPtr = NULL;
					if (itemIndex < ComponentsTree->topLevelItemCount()){
						componentRootPtr = ComponentsTree->topLevelItem(itemIndex);
					}
					else{
						componentRootPtr = new QTreeWidgetItem();
						ComponentsTree->addTopLevelItem(componentRootPtr);
					}
					Q_ASSERT(componentRootPtr != NULL);

					if (m_metaInfoManagerCompPtr.IsValid()){
						const icomp::IComponentStaticInfo* infoPtr = m_metaInfoManagerCompPtr->GetComponentMetaInfo(selectedInfoPtr->address);

						bool readOnly = ((selectedInfoPtr->elementPtr->GetElementFlags() & icomp::IRegistryElement::EF_IS_DETACHED) != 0);

						bool warningFlag = false;
						bool exportFlag = false;
						CreateExportedComponentsTree(elementId, elementId, infoPtr, *componentRootPtr, warningFlag, exportFlag, readOnly);
						hasWarning = hasWarning || warningFlag;
						hasExport = hasExport || exportFlag;
					}

					++itemIndex;
				}
			}
		}
	}

	while (itemIndex < ComponentsTree->topLevelItemCount()){
		delete ComponentsTree->topLevelItem(itemIndex);
	}

	ComponentsTree->resizeColumnToContents(AC_NAME);

	if (hasWarning){
		ElementInfoTab->setTabIcon(TI_EXPORTS, m_warningIcon);
	}
	else if (hasExport){
		ElementInfoTab->setTabIcon(TI_EXPORTS, m_exportIcon);
	}
	else{
		ElementInfoTab->setTabIcon(TI_EXPORTS, QIcon());
	}
}


// protected methods

bool CAttributeEditorComp::SetAttributeToItem(
			AttributeGroupType groupType,
			int& itemIndex,
			const icomp::IRegistry& registry,
			const QByteArray& attributeId,
			const ElementIdToAttrInfoMap& infos,
			const QFont& normalFont,
			const QFont& importantFont,
			bool& hasError,
			bool& hasWarning,
			bool& hasExport) const
{
	bool isAttributeEditable = true;
	bool isAttributeEnabled = false;
	bool isAttributeObligatory = false;
	bool isAttributeUsed = false;
	bool isAttributeWarning = false;
	bool isAttributeError = false;
	QString attributeValueText;
	int attributeStatMeaning = AM_NONE;
	int attributeValueMeaning = AM_NONE;
	QString attributeImportText;
	QByteArray attributeExportValue;
	QString attributeValueTip;
	QString attributeDescription;

	QByteArray attributeValueTypeId;
	QByteArray attributeStatTypeId;
	icomp::IElementStaticInfo::Ids obligatoryInterfaces;
	icomp::IElementStaticInfo::Ids optionalInterfaces;

	for (		ElementIdToAttrInfoMap::ConstIterator attrsIter = infos.constBegin();
				attrsIter != infos.constEnd();
				++attrsIter){
		const AttrInfo& attrInfo = attrsIter.value();

		if (attrInfo.infoPtr != NULL){
			if (!attributeExportValue.isEmpty() && (attrInfo.infoPtr->exportId != attributeExportValue)){
				attributeImportText = tr("<multi selection>");
			}
			else{
				attributeExportValue = attrInfo.infoPtr->exportId;
				attributeImportText = attributeExportValue;
			}
		}

		const iser::IObject* attributePtr = NULL;
		if ((attrInfo.infoPtr != NULL) && attrInfo.infoPtr->attributePtr.IsValid()){
			isAttributeEnabled = true;
			isAttributeUsed = true;

			attributePtr = attrInfo.infoPtr->attributePtr.GetPtr();
		}
		else if (attrInfo.staticInfoPtr != NULL){
			attributePtr = attrInfo.staticInfoPtr->GetAttributeDefaultValue();
		}
		QString text;

		QByteArray currentAttrTypeId;
		if (attrInfo.infoPtr != NULL){
			currentAttrTypeId = attrInfo.infoPtr->attributeTypeName;
			if (!currentAttrTypeId.isEmpty()){
				if (attributeValueTypeId.isEmpty()){
					attributeValueTypeId = currentAttrTypeId;
				}
			}
		}

		if (attrInfo.staticInfoPtr != NULL){
			QByteArray statAttrTypeId = attrInfo.staticInfoPtr->GetAttributeTypeId();
			if (attributeStatTypeId.isEmpty()){
				attributeStatTypeId = statAttrTypeId;
			}
			else if (attributeStatTypeId != statAttrTypeId){
				if (!attributeValueTip.isEmpty()){
					attributeValueTip += "\n";
				}
				attributeValueTip += tr("More elements selected with the same attribute name and different type");
				isAttributeEditable = false;
				break;
			}

			int attributeFlags = attrInfo.staticInfoPtr->GetAttributeFlags();

			int meaning = AM_NONE;

			if ((attributeFlags & icomp::IAttributeStaticInfo::AF_REFERENCE) != 0){
				if ((attributeFlags & icomp::IAttributeStaticInfo::AF_MULTIPLE) != 0){
					meaning = AM_MULTI_REFERENCE;
				}
				else{
					meaning = AM_REFERENCE;
				}
			}
			else if ((attributeFlags & icomp::IAttributeStaticInfo::AF_FACTORY) != 0){
				if ((attributeFlags & icomp::IAttributeStaticInfo::AF_MULTIPLE) != 0){
					meaning = AM_MULTI_FACTORY;
				}
				else{
					meaning = AM_FACTORY;
				}
			}
			else{
				if ((attributeFlags & icomp::IAttributeStaticInfo::AF_MULTIPLE) != 0){
					meaning = AM_MULTI_ATTRIBUTE;
				}
				else{
					if (statAttrTypeId == iattr::TAttribute<bool>::GetTypeName()){
						meaning = AM_BOOL_ATTRIBUTE;
					}
					else{
						meaning = AM_ATTRIBUTE;
					}
				}
			}

			if ((attributeStatMeaning != AM_NONE) && (meaning != attributeStatMeaning)){
				attributeStatMeaning = AM_MULTI;
			}
			else{
				attributeStatMeaning = meaning;
			}

			if ((attributeFlags & icomp::IAttributeStaticInfo::AF_OBLIGATORY) != 0){
				isAttributeObligatory = true;

				if ((attributeFlags & icomp::IAttributeStaticInfo::AF_NULLABLE) != 0){
					isAttributeUsed = true;
				}
			}
			QString description = attrInfo.staticInfoPtr->GetAttributeDescription();
			if (!description.isEmpty()){
				if (!attributeDescription.isEmpty()){
					attributeDescription += "\n";
				}
				attributeDescription = description;
			}

			icomp::IElementStaticInfo::Ids interfaces = attrInfo.staticInfoPtr->GetRelatedMetaIds(
						icomp::IComponentStaticInfo::MGI_INTERFACES,
						icomp::IAttributeStaticInfo::AF_OBLIGATORY,
						icomp::IAttributeStaticInfo::AF_OBLIGATORY);	// Names of obligatory interfaces
			obligatoryInterfaces += interfaces;
			interfaces = attrInfo.staticInfoPtr->GetRelatedMetaIds(
						icomp::IComponentStaticInfo::MGI_INTERFACES,
						0,
						icomp::IAttributeStaticInfo::AF_OBLIGATORY);	// Names of optional interfaces
			optionalInterfaces += interfaces;
		}

		int valueMeaning;
		if ((attributePtr != NULL) && DecodeAttribute(*attributePtr, text, valueMeaning)){
			if (!attributeValueText.isEmpty() && (text != attributeValueText)){
				attributeValueText = tr("<multi selection>");
			}
			else{
				attributeValueText = text;
			}

			if ((attributeValueMeaning != AM_NONE) && (valueMeaning != attributeValueMeaning)){
				attributeValueMeaning = AM_MULTI;
			}
			else{
				attributeValueMeaning = valueMeaning;
			}
		}
	}

	// get the attribute meaning from registry if it is not defined in packages
	if (attributeStatMeaning == AM_NONE){
		attributeStatMeaning = attributeValueMeaning;

		if (!attributeValueTip.isEmpty()){
			attributeValueTip += "\n";
		}
		attributeValueTip += tr("Attribute doesn't exist in package (was removed?)");
		isAttributeWarning = true;
	}

	// get the attribute type from registry if it is not defined in packages
	if (attributeStatTypeId.isEmpty()){
		attributeStatTypeId = attributeValueTypeId;
	}
	else if (!attributeValueTypeId.isEmpty() && !CRegistryConsistInfoComp::AreTypesCompatible(attributeStatTypeId, attributeValueTypeId)){
		attributeValueTip += tr("Attribute type in package differs from registry");
		isAttributeError = true;
	}

	AttributeGroupType elementGroupType = AGT_ATTRIBUTE;
	QString attributeTypeDescription;
	AttributeTypesMap::ConstIterator foundTypeNameIter = m_attributeTypesMap.constFind(attributeStatTypeId);
	if (foundTypeNameIter != m_attributeTypesMap.constEnd()){
		attributeTypeDescription = foundTypeNameIter.value().first;
		elementGroupType = foundTypeNameIter.value().second;
	}
	else{
		attributeTypeDescription = tr("unsupported attribute of type '%1'").arg(QString(attributeStatTypeId));
	}

	if (elementGroupType != groupType){
		return false;
	}

	QString consistencyInfoText;

	if (m_consistInfoCompPtr.IsValid()){
		for (		ElementIdToAttrInfoMap::ConstIterator attrsIter = infos.constBegin();
					attrsIter != infos.constEnd();
					++attrsIter){
			const QByteArray& elementId = attrsIter.key();
			const AttrInfo& attrInfo = attrsIter.value();

			ilog::CMessageContainer messageContainer;

			if (!m_consistInfoCompPtr->IsAttributeValid(
							attributeId,
							elementId,
							registry,
							true,
							false,
							&messageContainer,
							attrInfo.componentStaticInfoPtr)){
				isAttributeError = true;
			}

			ilog::IMessageContainer::Messages messages = messageContainer.GetMessages();
			for (		ilog::IMessageContainer::Messages::const_iterator messageIter = messages.cbegin();
						messageIter != messages.cend();
						++messageIter){
				const ilog::IMessageConsumer::MessagePtr messagePtr = *messageIter;
				if (messagePtr.IsValid()){
					consistencyInfoText += "\n";

					switch (messagePtr->GetInformationCategory()){
					case istd::IInformationProvider::IC_INFO:
						consistencyInfoText += tr("Information: ");
						break;

					case istd::IInformationProvider::IC_WARNING:
						consistencyInfoText += tr("Warning: ");
						break;

					case istd::IInformationProvider::IC_ERROR:
						consistencyInfoText += tr("Error: ");
						break;

					case istd::IInformationProvider::IC_CRITICAL:
						consistencyInfoText += tr("Critical error: ");
						break;

					default:
						break;
					}

					consistencyInfoText += messagePtr->GetInformationDescription();
				}
			}
		}
	}

	if (!isAttributeObligatory){
		attributeTypeDescription = tr("Optional %1").arg(attributeTypeDescription);
	}

	if (attributeValueTip.isEmpty()){
		attributeValueTip = (attributeTypeDescription.isEmpty() || attributeDescription.isEmpty())?
					attributeDescription + attributeTypeDescription:
					tr("%1\nType: %2").arg(attributeDescription, attributeTypeDescription);

		if (!obligatoryInterfaces.isEmpty() || !optionalInterfaces.isEmpty()){
			attributeValueTip += tr("\nInterfaces:");
			for (		icomp::IElementStaticInfo::Ids::ConstIterator obligIter = obligatoryInterfaces.constBegin();
						obligIter != obligatoryInterfaces.constEnd();
						++obligIter){
				attributeValueTip += tr("\n - %1").arg(QString(*obligIter));
			}
			for (		icomp::IElementStaticInfo::Ids::ConstIterator optIter = optionalInterfaces.constBegin();
						optIter != optionalInterfaces.constEnd();
						++optIter){
				attributeValueTip += tr("\n - %1 (optional)").arg(QString(*optIter));
			}
		}
	}

	attributeValueTip += consistencyInfoText;

	QTreeWidgetItem* attributeItemPtr = NULL;
	if (itemIndex < AttributeTree->topLevelItemCount()){
		attributeItemPtr = AttributeTree->topLevelItem(itemIndex);
	}
	else{
		attributeItemPtr = new QTreeWidgetItem();
		AttributeTree->addTopLevelItem(attributeItemPtr);
	}

	attributeItemPtr->setText(AC_NAME, attributeId);
	attributeItemPtr->setData(AC_VALUE, AttributeId, attributeId);

	attributeItemPtr->setText(AC_VALUE, attributeValueText);
	attributeItemPtr->setData(AC_VALUE, AttributeTypeId, attributeStatTypeId);
	attributeItemPtr->setData(AC_VALUE, AttributeMining, attributeStatMeaning);

	attributeItemPtr->setToolTip(AC_NAME, attributeValueTip);
	attributeItemPtr->setToolTip(AC_VALUE, attributeValueTip);

	attributeItemPtr->setFlags(Qt::ItemIsSelectable);
	attributeItemPtr->setData(AC_NAME, Qt::CheckStateRole, QVariant());
	if (isAttributeEditable){
		attributeItemPtr->setFlags(attributeItemPtr->flags() | Qt::ItemIsEnabled | Qt::ItemIsEditable);
	}

	if (isAttributeEnabled){
		attributeItemPtr->setFlags(attributeItemPtr->flags() | Qt::ItemIsUserCheckable);
		attributeItemPtr->setCheckState(AC_NAME, Qt::Checked);
	}

	QColor backgroundColor = Qt::white;
	if (groupType == AGT_REFERENCE){
		backgroundColor = QColor(240, 240, 255);
	}
	else if (groupType == AGT_FACTORY){
		backgroundColor = QColor(255, 255, 240);
	}

	attributeItemPtr->setBackground(AC_NAME, backgroundColor);

	attributeItemPtr->setFont(AC_NAME, isAttributeEnabled? importantFont: normalFont);
	attributeItemPtr->setFont(AC_VALUE, isAttributeUsed? importantFont: normalFont);

	if (isAttributeError){
		attributeItemPtr->setBackground(AC_VALUE, Qt::red);
		hasError = true;
	}
	else if (isAttributeWarning){
		attributeItemPtr->setBackground(AC_VALUE, Qt::yellow);
		hasWarning = true;
	}
	else{
		attributeItemPtr->setBackground(AC_VALUE, backgroundColor);
	}

	QTreeWidgetItem* importItemPtr = NULL;
	if (attributeItemPtr->childCount() < 1){
		importItemPtr = new QTreeWidgetItem(attributeItemPtr);
		importItemPtr->setText(AC_NAME, tr("<import>"));
		importItemPtr->setData(AC_VALUE, AttributeMining, AM_EXPORTED_ATTR);
	}
	else{
		importItemPtr = attributeItemPtr->child(0);
	}

	importItemPtr->setData(AC_VALUE, AttributeId, attributeId);
	importItemPtr->setData(AC_VALUE, AttributeTypeId, attributeValueTypeId);
	importItemPtr->setText(AC_VALUE, attributeImportText);
	importItemPtr->setData(AC_VALUE, AttributeValue, attributeExportValue);

	if (!attributeImportText.isEmpty()){
		importItemPtr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
		importItemPtr->setCheckState(AC_NAME, Qt::Checked);

		attributeItemPtr->setIcon(AC_NAME, m_importIcon);

		hasExport = true;
	}
	else{
		importItemPtr->setData(AC_NAME, Qt::CheckStateRole, QVariant());
		importItemPtr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		attributeItemPtr->setIcon(AC_NAME, QIcon());
	}

	importItemPtr->setHidden(false);

	++itemIndex;

	return true;
}


bool CAttributeEditorComp::SetInterfaceToItem(
			QTreeWidgetItem& item,
			icomp::IRegistry::ExportedInterfacesMap* interfacesMapPtr,
			const QByteArray& elementId,
			const QByteArray& interfaceName,
			bool& hasWarning,
			bool& hasExport,
			bool readOnly) const
{
	item.setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);

	item.setData(0, InterfaceName, interfaceName);
	item.setData(0, ElementId, elementId);

	if (interfacesMapPtr != NULL){
		hasExport = false;
		icomp::IRegistry::ExportedInterfacesMap::Iterator foundExportIter = interfacesMapPtr->find(interfaceName);
		if ((foundExportIter != interfacesMapPtr->end()) && (foundExportIter.value() == elementId)){
			hasExport = true;
			interfacesMapPtr->erase(foundExportIter);
		}

		item.setText(0, interfaceName);
		item.setBackground(0, Qt::transparent);
		if (readOnly){
			if (hasExport){
				item.setToolTip(0, tr("Export of interfaces from detached objects is not allowed"));
				item.setBackground(0, Qt::yellow);

				hasWarning = true;
			}
			else{
				item.setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			}
		}

		item.setCheckState(0, hasExport? Qt::Checked: Qt::Unchecked);
		item.setToolTip(0, "");
	}
	else{
		item.setText(0, elementId + ": " + interfaceName);
		item.setCheckState(0, Qt::Checked);
		item.setToolTip(0, tr("Interface doesn't implemented by this element (was removed?)"));
		item.setBackground(0, Qt::yellow);
	}
	item.setIcon(0, QIcon());

	while (item.childCount() > 0){
		delete item.child(0);
	}

	return true;
}


bool CAttributeEditorComp::ResetItem(QTreeWidgetItem& item)
{
	item.setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item.setData(0, InterfaceName, QVariant());
	item.setData(0, ElementId, QVariant());
	item.setText(0, "");
	item.setData(0, Qt::CheckStateRole, QVariant());
	item.setToolTip(0, "");
	item.setBackground(0, Qt::transparent);
	item.setIcon(0, QIcon());

	return true;
}


bool CAttributeEditorComp::DecodeAttribute(
			const iser::ISerializable& attribute,
			QString& text,
			int& meaning) const
{
	const iattr::CIntegerAttribute* intAttribute = dynamic_cast<const iattr::CIntegerAttribute*>(&attribute);
	if (intAttribute != NULL){
		text = QString::number(intAttribute->GetValue());
		meaning = AM_ATTRIBUTE;

		return true;
	}

	const iattr::CRealAttribute* doubleAttribute = dynamic_cast<const iattr::CRealAttribute*>(&attribute);
	if (doubleAttribute != NULL){
		text = QString::number(doubleAttribute->GetValue(), 'g', 12);
		meaning = AM_ATTRIBUTE;

		return true;
	}

	const iattr::CBooleanAttribute* boolAttribute = dynamic_cast<const iattr::CBooleanAttribute*>(&attribute);
	if (boolAttribute != NULL){
		text = boolAttribute->GetValue()? "true": "false";
		meaning = AM_BOOL_ATTRIBUTE;

		return true;
	}

	const iattr::CStringAttribute* stringAttribute = dynamic_cast<const iattr::CStringAttribute*>(&attribute);
	if (stringAttribute != NULL){
		text = EncodeToEdit(stringAttribute->GetValue());
		meaning = AM_ATTRIBUTE;

		return true;
	}

	const iattr::CIdAttribute* idPtr = dynamic_cast<const iattr::CIdAttribute*>(&attribute);
	if (idPtr != NULL){
		text = EncodeToEdit(idPtr->GetValue());

		if (dynamic_cast<const icomp::CFactoryAttribute*>(idPtr) != NULL){
			meaning = AM_FACTORY;
		}
		else if (dynamic_cast<const icomp::CReferenceAttribute*>(idPtr) != NULL){
			meaning = AM_REFERENCE;
		}
		else{
			meaning = AM_ATTRIBUTE;
		}

		return true;
	}

	const iattr::CStringListAttribute* stringListAttribute = dynamic_cast<const iattr::CStringListAttribute*>(&attribute);
	if (stringListAttribute != NULL){
		text.clear();

		for (int index = 0; index < stringListAttribute->GetValuesCount(); index++){
			if (index != 0){
				text += ";";
			}

			text += EncodeToEdit(stringListAttribute->GetValueAt(index));
		}

		meaning = AM_MULTI_ATTRIBUTE;

		return true;
	}

	const iattr::CIntegerListAttribute* intListAttribute = dynamic_cast<const iattr::CIntegerListAttribute*>(&attribute);
	if (intListAttribute != NULL){
		text.clear();

		for (int index = 0; index < intListAttribute->GetValuesCount(); index++){
			if (index != 0){
				text += ";";
			}

			text += QString::number(intListAttribute->GetValueAt(index));
		}

		meaning = AM_MULTI_ATTRIBUTE;

		return true;
	}

	const iattr::CRealListAttribute* doubleListAttribute = dynamic_cast<const iattr::CRealListAttribute*>(&attribute);
	if (doubleListAttribute != NULL){
		text.clear();

		for (int index = 0; index < doubleListAttribute->GetValuesCount(); index++){
			if (index != 0){
				text += ";";
			}

			text += QString::number(doubleListAttribute->GetValueAt(index), 'g', 12);
		}

		meaning = AM_MULTI_ATTRIBUTE;

		return true;
	}

	const iattr::CBooleanListAttribute* boolListAttribute = dynamic_cast<const iattr::CBooleanListAttribute*>(&attribute);
	if (boolListAttribute != NULL){
		text.clear();

		for (int index = 0; index < boolListAttribute->GetValuesCount(); index++){
			if (index != 0){
				text += ";";
			}

			text += boolListAttribute->GetValueAt(index)? "true": "false";
		}

		meaning = AM_MULTI_ATTRIBUTE;

		return true;
	}

	const iattr::CIdListAttribute* multiIdPtr = dynamic_cast<const iattr::CIdListAttribute*>(&attribute);
	if (multiIdPtr != NULL){
		QString dependecyString;

		for (int index = 0; index < multiIdPtr->GetValuesCount(); index++){
			if (index != 0){
				text += ";";
			}

			QString componentId = multiIdPtr->GetValueAt(index);

			text += EncodeToEdit(componentId);
		}

		if (dynamic_cast<const icomp::CMultiFactoryAttribute*>(multiIdPtr) != NULL){
			meaning = AM_MULTI_FACTORY;
		}
		else if (dynamic_cast<const icomp::CMultiReferenceAttribute*>(multiIdPtr) != NULL){
			meaning = AM_MULTI_REFERENCE;
		}
		else{
			meaning = AM_MULTI_ATTRIBUTE;
		}

		return true;
	}

	return false;
}


bool CAttributeEditorComp::EncodeAttribute(
			const QString& text,
			int attributeStatMeaning,
			iser::ISerializable& result) const
{
	// set single reference of factory data
	if ((attributeStatMeaning == AM_REFERENCE) || (attributeStatMeaning == AM_FACTORY)){
		iattr::TAttribute<QByteArray>* referenceAttributePtr = dynamic_cast<iattr::TAttribute<QByteArray>*>(&result);
		if (referenceAttributePtr != NULL){
			referenceAttributePtr->SetValue(DecodeFromEdit(text).toLocal8Bit());

			return true;
		}
	}
	// set multiple reference data
	else if ((attributeStatMeaning == AM_MULTI_REFERENCE) || (attributeStatMeaning == AM_MULTI_FACTORY)){

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
		QStringList references = text.split(';', Qt::SkipEmptyParts);
#else
		QStringList references = text.split(';', QString::SkipEmptyParts);
#endif
		iattr::TMultiAttribute<QByteArray>* multiReferenceAttributePtr = dynamic_cast<iattr::TMultiAttribute<QByteArray>*>(&result);

		if (multiReferenceAttributePtr != NULL){
			multiReferenceAttributePtr->Reset();
			for (int index = 0; index < references.count(); index++){
				multiReferenceAttributePtr->InsertValue(DecodeFromEdit(references.at(index)).toLocal8Bit());
			}

			return true;
		}
	}
	// set attribute data:
	else if (attributeStatMeaning == AM_ATTRIBUTE){
		iattr::CIntegerAttribute* intAttributePtr = dynamic_cast<iattr::CIntegerAttribute*>(&result);
		if (intAttributePtr != NULL){
			intAttributePtr->SetValue(text.toInt());

			return true;
		}

		iattr::CRealAttribute* doubleAttributePtr = dynamic_cast<iattr::CRealAttribute*>(&result);
		if (doubleAttributePtr != NULL){
			doubleAttributePtr->SetValue(text.toDouble());

			return true;
		}

		iattr::CStringAttribute* stringAttributePtr = dynamic_cast<iattr::CStringAttribute*>(&result);
		if (stringAttributePtr != NULL){
			stringAttributePtr->SetValue(DecodeFromEdit(text));

			return true;
		}

		iattr::CIdAttribute* idAttributePtr = dynamic_cast<iattr::CIdAttribute*>(&result);
		if (idAttributePtr != NULL){
			idAttributePtr->SetValue(DecodeFromEdit(text).toLocal8Bit());

			return true;
		}
	}
	else if (attributeStatMeaning == AM_BOOL_ATTRIBUTE){
		iattr::CBooleanAttribute* boolAttributePtr = dynamic_cast<iattr::CBooleanAttribute*>(&result);
		if (boolAttributePtr != NULL){
			boolAttributePtr->SetValue(text == "true");

			return true;
		}
	}
	else if (attributeStatMeaning == AM_MULTI_ATTRIBUTE){
		QStringList values = text.split(';');

		iattr::CIntegerListAttribute* intListAttributePtr = dynamic_cast<iattr::CIntegerListAttribute*>(&result);
		if (intListAttributePtr != NULL){
			intListAttributePtr->Reset();
			for (int index = 0; index < values.count(); index++){
				intListAttributePtr->InsertValue(values.at(index).toInt());
			}

			return true;
		}

		iattr::CRealListAttribute* doubleListAttributePtr = dynamic_cast<iattr::CRealListAttribute*>(&result);
		if (doubleListAttributePtr != NULL){
			doubleListAttributePtr->Reset();
			for (int index = 0; index < values.count(); index++){
				doubleListAttributePtr->InsertValue(values.at(index).toDouble());
			}

			return true;
		}

		iattr::CBooleanListAttribute* boolListAttributePtr = dynamic_cast<iattr::CBooleanListAttribute*>(&result);
		if (boolListAttributePtr != NULL){
			boolListAttributePtr->Reset();
			for (int index = 0; index < values.count(); index++){
				boolListAttributePtr->InsertValue(values.at(index) == "true");
			}

			return true;
		}

		iattr::CStringListAttribute* stringListAttributePtr = dynamic_cast<iattr::CStringListAttribute*>(&result);
		if (stringListAttributePtr != NULL){
			stringListAttributePtr->Reset();
			for (int index = 0; index < values.count(); index++){
				stringListAttributePtr->InsertValue(DecodeFromEdit(values.at(index)));
			}

			return true;
		}

		iattr::CIdListAttribute* idListAttributePtr = dynamic_cast<iattr::CIdListAttribute*>(&result);
		if (idListAttributePtr != NULL){
			idListAttributePtr->Reset();
			for (int index = 0; index < values.count(); index++){
				idListAttributePtr->InsertValue(DecodeFromEdit(values.at(index)).toLocal8Bit());
			}

			return true;
		}
	}

	return false;
}


void CAttributeEditorComp::CreateInterfacesTree(
			const QByteArray& elementId,
			const icomp::IElementStaticInfo* infoPtr,
			icomp::IRegistry::ExportedInterfacesMap& registryInterfaces,
			QTreeWidgetItem* parentItemPtr,
			bool& hasWarning,
			bool& hasExport,
			bool includeSubelement,
			bool readOnly)
{
	hasWarning = false;
	hasExport = false;

	int itemIndex = 0;

	// display all interfaces based on meta information
	if (infoPtr != NULL){
		const icomp::IElementStaticInfo::Ids& interfaceIds = infoPtr->GetMetaIds(icomp::IComponentStaticInfo::MGI_INTERFACES);
		for (		icomp::IElementStaticInfo::Ids::ConstIterator interfaceIter = interfaceIds.constBegin();
					interfaceIter != interfaceIds.constEnd();
					interfaceIter++){
			const QByteArray& interfaceName = *interfaceIter;

			QTreeWidgetItem* itemPtr = NULL;
			if (parentItemPtr != NULL){
				if (itemIndex < parentItemPtr->childCount()){
					itemPtr = parentItemPtr->child(itemIndex);
				}
				else{
					itemPtr = new QTreeWidgetItem(parentItemPtr);
					parentItemPtr->addChild(itemPtr);
				}
			}
			else{
				if (itemIndex < InterfacesTree->topLevelItemCount()){
					itemPtr = InterfacesTree->topLevelItem(itemIndex);
				}
				else{
					itemPtr = new QTreeWidgetItem();
					InterfacesTree->addTopLevelItem(itemPtr);
				}
			}
			itemIndex++;
			Q_ASSERT(itemPtr != NULL);

			bool warningFlag = false;
			bool exportFlag = false;
			SetInterfaceToItem(
						*itemPtr,
						&registryInterfaces,
						elementId,
						interfaceName,
						warningFlag,
						exportFlag,
						readOnly);
			hasWarning = hasWarning || warningFlag;
			hasExport = hasExport || exportFlag;
		}

		if (includeSubelement){
			const icomp::IElementStaticInfo::Ids subcomponentIds = infoPtr->GetMetaIds(icomp::IComponentStaticInfo::MGI_SUBELEMENTS);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
			QList< QByteArray> sortedSubcomponentIds(subcomponentIds.begin(), subcomponentIds.end());
#else
			QList<QByteArray> sortedSubcomponentIds = subcomponentIds.toList();
#endif
			std::sort(sortedSubcomponentIds.begin(), sortedSubcomponentIds.end());

			for (		QList<QByteArray>::ConstIterator subIter = sortedSubcomponentIds.constBegin();
						subIter != sortedSubcomponentIds.constEnd();
						++subIter){
				const QByteArray& subelementId = *subIter;

				QByteArray globalSubelementId = istd::CIdManipBase::JoinId(elementId, subelementId);

				QTreeWidgetItem* itemPtr = NULL;
				if (parentItemPtr != NULL){
					if (itemIndex < parentItemPtr->childCount()){
						itemPtr = parentItemPtr->child(itemIndex);
					}
					else{
						itemPtr = new QTreeWidgetItem(parentItemPtr);
						parentItemPtr->addChild(itemPtr);
					}
				}
				else{
					if (itemIndex < InterfacesTree->topLevelItemCount()){
						itemPtr = InterfacesTree->topLevelItem(itemIndex);
					}
					else{
						itemPtr = new QTreeWidgetItem();
						itemPtr->setExpanded(true);
						InterfacesTree->addTopLevelItem(itemPtr);
					}
				}
				itemIndex++;
				Q_ASSERT(itemPtr != NULL);

				ResetItem(*itemPtr);
				itemPtr->setText(AC_NAME, subelementId);

				const icomp::IElementStaticInfo* subelementInfoPtr = infoPtr->GetSubelementInfo(subelementId);

				bool warningFlag = false;
				bool exportFlag = false;
				CreateInterfacesTree(
							globalSubelementId,
							subelementInfoPtr,
							registryInterfaces,
							itemPtr,
							warningFlag,
							exportFlag,
							false,
							readOnly);
				hasWarning = hasWarning || warningFlag;
				hasExport = hasExport || exportFlag;
			}
		}
	}

	// display all interfaces assigned to this element, but not existing in element meta information
	for (		icomp::IRegistry::ExportedInterfacesMap::ConstIterator interfaceIter = registryInterfaces.begin();
				interfaceIter != registryInterfaces.end();
				interfaceIter++){
		const QByteArray& interfaceName = interfaceIter.key();
		if (interfaceIter.value() == elementId){
			QTreeWidgetItem* itemPtr = NULL;
			if (itemIndex < InterfacesTree->topLevelItemCount()){
				itemPtr = InterfacesTree->topLevelItem(itemIndex);
			}
			else{
				itemPtr = new QTreeWidgetItem();
				InterfacesTree->addTopLevelItem(itemPtr);
			}
			itemIndex++;
			Q_ASSERT(itemPtr != NULL);

			bool warningFlag = false;
			bool exportFlag = false;
			SetInterfaceToItem(*itemPtr, NULL, elementId, interfaceName, warningFlag, exportFlag, readOnly);
			hasWarning = true;
		}
	}

	if (parentItemPtr != NULL){
		while (parentItemPtr->childCount() > itemIndex){
			delete parentItemPtr->child(itemIndex);
		}

		if (hasWarning){
			parentItemPtr->setIcon(AC_NAME, m_warningIcon);
		}
		else if (hasExport){
			parentItemPtr->setIcon(AC_NAME, m_exportIcon);
		}
		else{
			parentItemPtr->setIcon(AC_NAME, QIcon());
		}
	}
	else{
		while (InterfacesTree->topLevelItemCount() > itemIndex){
			delete InterfacesTree->topLevelItem(itemIndex);
		}
	}
}


void CAttributeEditorComp::CreateExportedComponentsTree(
			const QByteArray& elementId,
			const QByteArray& globalElementId,
			const icomp::IElementStaticInfo* elementMetaInfoPtr,
			QTreeWidgetItem& item,
			bool& hasWarning,
			bool& hasExport,
			bool readOnly) const
{
	int itemIndex = 0;

	QStringList exportedAliases = GetExportAliases(globalElementId);
	if (!exportedAliases.isEmpty()){
		hasExport = true;
	}

	item.setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item.setText(AC_NAME, elementId);
	item.setData(AC_VALUE, AttributeId, QString(globalElementId));
	item.setData(AC_VALUE, AttributeValue, QString(elementId));
	item.setData(AC_VALUE, AttributeMining, AM_EXPORTED_COMP);
	item.setText(AC_VALUE, exportedAliases.join(";"));
	item.setExpanded(true);

	if (elementMetaInfoPtr != NULL){
		const icomp::IElementStaticInfo::Ids subcomponentIds = elementMetaInfoPtr->GetMetaIds(icomp::IComponentStaticInfo::MGI_SUBELEMENTS);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
		QList< QByteArray> sortedSubcomponentIds(subcomponentIds.begin(), subcomponentIds.end());
#else
		QList<QByteArray> sortedSubcomponentIds = subcomponentIds.toList();
#endif
		std::sort(sortedSubcomponentIds.begin(), sortedSubcomponentIds.end());

		for (		QList<QByteArray>::ConstIterator subIter = sortedSubcomponentIds.constBegin();
					subIter != sortedSubcomponentIds.constEnd();
					++subIter, ++itemIndex){
			const QByteArray& subelementId = *subIter;

			QByteArray globalSubelementId = istd::CIdManipBase::JoinId(globalElementId, subelementId);

			QStringList subExportedAliases = GetExportAliases(globalSubelementId);
			if (!subExportedAliases.isEmpty()){
				hasExport = true;
			}

			QTreeWidgetItem* itemPtr = NULL;
			if (itemIndex < item.childCount()){
				itemPtr = item.child(itemIndex);
			}
			else{
				itemPtr = new QTreeWidgetItem(&item);
			}

			itemPtr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
			itemPtr->setBackground(AC_VALUE, Qt::transparent);
			if (readOnly){
				if (subExportedAliases.isEmpty()){
					itemPtr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				}
				else{
					itemPtr->setBackground(AC_VALUE, Qt::yellow);
					hasWarning = true;
				}
			}
			itemPtr->setText(AC_NAME, subelementId);
			itemPtr->setData(AC_VALUE, AttributeId, QString(globalSubelementId));
			itemPtr->setData(AC_VALUE, AttributeValue, QString(subelementId));
			itemPtr->setData(AC_VALUE, AttributeMining, AM_EXPORTED_COMP);
			itemPtr->setText(AC_VALUE, subExportedAliases.join(";"));
			itemPtr->setExpanded(true);

/*
			Q_ASSERT(itemPtr != NULL);

			const icomp::IElementStaticInfo* subcomponentInfoPtr = elementMetaInfoPtr->GetSubelementInfo(subelementId);

			bool warningFlag = false;
			bool exportFlag = false;
			CreateExportedComponentsTree(subelementId, globalSubelementId, subcomponentInfoPtr, *itemPtr, &warningFlag, &exportFlag);
			hasWarning = hasWarning || warningFlag;
			hasExport = hasExport || exportFlag;
*/
		}
		item.setDisabled(false);
	}
	else{
		item.setDisabled(true);
	}

	while (itemIndex < item.childCount()){
		delete item.child(itemIndex);
	}
}


// reimplemented (TGuiObserverWrap)

void CAttributeEditorComp::OnGuiModelDetached()
{
	if (!IsUpdateBlocked()){
		UpdateBlocker updateBlocker(this);

		ElementInfoTab->setVisible(false);

		AttributeTree->clear();
		InterfacesTree->clear();
		ComponentsTree->clear();
	}

	BaseClass::OnGuiModelDetached();
}


void CAttributeEditorComp::UpdateGui(const istd::IChangeable::ChangeSet& changeSet)
{
	Q_ASSERT(IsGuiCreated());

	if (changeSet.Contains(IElementSelectionInfo::CF_SELECTION)){
		AttributeTree->reset();
	}

	const IElementSelectionInfo* objectPtr = GetObservedObject();
	if (objectPtr != NULL){
		icomp::IRegistry* registryPtr = objectPtr->GetSelectedRegistry();

		imod::IModel* registryModelPtr = dynamic_cast<imod::IModel*>(registryPtr);
		if (registryModelPtr != m_lastRegistryModelPtr){
			m_registryObserver.EnsureModelDetached();

			if (m_registryPropObserverCompPtr.IsValid()){
				if (m_registryPropObserverCompPtr->IsModelAttached(m_lastRegistryModelPtr)){
					m_lastRegistryModelPtr->DetachObserver(m_registryPropObserverCompPtr.GetPtr());
				}
			}

			if (registryModelPtr != NULL){
				registryModelPtr->AttachObserver(&m_registryObserver);

				if (m_registryPropObserverCompPtr.IsValid()){
					registryModelPtr->AttachObserver(m_registryPropObserverCompPtr.GetPtr());
				}
			}

			m_lastRegistryModelPtr = registryModelPtr;
		}

		if (registryPtr == NULL){
			ElementInfoTab->setVisible(false);
			RegistryPropertiesFrame->setVisible(false);

			return;
		}

		IElementSelectionInfo::Elements selectedElements = objectPtr->GetSelectedElements();

		if (selectedElements.isEmpty()){
			ElementInfoTab->setVisible(false);

			RegistryPropertiesFrame->setVisible(true);
		}
		else{
			RegistryPropertiesFrame->setVisible(false);

			ElementInfoTab->setVisible(true);
		}
	}

	UpdateGeneralView();
	UpdateAttributesView();
	UpdateInterfacesView();
	UpdateFlagsView();
	UpdateSubcomponentsView();
}


// reimplemented (CGuiComponentBase)

void CAttributeEditorComp::OnGuiCreated()
{
	BaseClass::OnGuiCreated();

	ElementInfoTab->setVisible(false);
	RegistryPropertiesFrame->setVisible(false);

	m_attributesTreeFilter.SetPtr(new iwidgets::CTreeWidgetFilter(AttributeTree));
	m_interfacesTreeFilter.SetPtr(new iwidgets::CTreeWidgetFilter(InterfacesTree));
	m_subcomponentsTreeFilter.SetPtr(new iwidgets::CTreeWidgetFilter(ComponentsTree));

	AttributeTree->setItemDelegate(&m_attributeItemDelegate);
	ComponentsTree->setItemDelegate(&m_attributeItemDelegate);

	m_invalidIcon.addFile(":/Icons/StateInvalid");
	m_warningIcon.addFile(":/Icons/Warning");
	m_exportIcon.addFile(":/Icons/Export");
	m_importIcon.addFile(":/Icons/Import");

	if (m_registryPropGuiCompPtr.IsValid()){
		m_registryPropGuiCompPtr->CreateGui(RegistryPropertiesFrame);
	}
}


void CAttributeEditorComp::OnGuiDestroyed()
{
	if (m_registryPropGuiCompPtr.IsValid()){
		m_registryPropGuiCompPtr->DestroyGui();
	}

	m_attributesTreeFilter.Reset();
	m_interfacesTreeFilter.Reset();
	m_subcomponentsTreeFilter.Reset();


	BaseClass::OnGuiDestroyed();
}


// static methods

QString CAttributeEditorComp::DecodeFromEdit(const QString& text)
{
	QString retVal = text;
	retVal.replace("\\:", ";");
	retVal.replace("\\\\", "\\");

	return retVal;
}


QString CAttributeEditorComp::EncodeToEdit(const QString& text)
{
	QString retVal = text;
	retVal.replace("\\", "\\\\");
	retVal.replace(";", "\\:");

	return retVal;
}


// public methods of embedded class AttributeItemDelegate

CAttributeEditorComp::AttributeItemDelegate::AttributeItemDelegate(CAttributeEditorComp* parentPtr)
:	m_parent(*parentPtr)
{
	Q_ASSERT(parentPtr != NULL);
}


// reimplemented (QItemDelegate)

QWidget* CAttributeEditorComp::AttributeItemDelegate::createEditor(QWidget* parentWidget, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.column() != AC_VALUE){
		return NULL;
	}

	int propertyMining = index.data(AttributeMining).toInt();

	if (		(propertyMining == AM_MULTI_ATTRIBUTE) ||
				(propertyMining == AM_MULTI_REFERENCE) ||
				(propertyMining == AM_MULTI_FACTORY)){
		QByteArray attributeId = index.data(AttributeId).toString().toLocal8Bit();

		int attributeFlags = 0;
		if (propertyMining == AM_MULTI_REFERENCE){
			attributeFlags = icomp::IAttributeStaticInfo::AF_REFERENCE;
		}
		else if (propertyMining == AM_MULTI_FACTORY){
			attributeFlags = icomp::IAttributeStaticInfo::AF_FACTORY;
		}
		else{
			attributeFlags = icomp::IAttributeStaticInfo::AF_VALUE;
		}

		return new CMultiAttributeDelegateWidget(
					const_cast<AttributeItemDelegate&>(*this),
					m_parent,
					parentWidget,
					attributeId,
					attributeFlags);
	}

	if (		(propertyMining == AM_REFERENCE) ||
				(propertyMining == AM_FACTORY) ||
				(propertyMining == AM_BOOL_ATTRIBUTE)){
		QByteArray attributeType = index.data(AttributeTypeId).toString().toLocal8Bit();
		QComboBox* comboEditor = new QComboBox(parentWidget);
		if (propertyMining == AM_BOOL_ATTRIBUTE){
			comboEditor->addItem("true");
			comboEditor->addItem("false");
		}
		else{
			QString text = index.data().toString();
			comboEditor->setEditable(true);
			comboEditor->setEditText(text);
		}

		return comboEditor;
	}

	return BaseClass::createEditor(parentWidget, option, index);
}


void CAttributeEditorComp::AttributeItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	if (index.column() != AC_VALUE){
		BaseClass::setEditorData(editor, index);
	}

	int propertyMining = index.data(AttributeMining).toInt();

	QByteArray attributeName = index.data(AttributeId).toString().toLocal8Bit();
	QByteArray attributeValue = index.data(AttributeValue).toString().toLocal8Bit();
	if (attributeValue.isEmpty()){
		attributeValue = attributeName;
	}

	if (propertyMining == AM_EXPORTED_COMP){
		SetComponentExportEditor(attributeName, attributeValue, *editor);
	}
	else if (propertyMining == AM_EXPORTED_ATTR){
		SetAttributeExportEditor(attributeName, attributeValue, *editor);
	}
	else{
		SetAttributeValueEditor(attributeName, propertyMining, *editor);
	}
}


void CAttributeEditorComp::AttributeItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	UpdateBlocker updateBlocker(&m_parent);

	if (index.column() != AC_VALUE){
		BaseClass::setModelData(editor, model, index);
	}

	QByteArray attributeId = index.data(AttributeId).toString().toLocal8Bit();
	QString newValue;

	const QComboBox* comboEditorPtr = dynamic_cast<const QComboBox*>(editor);
	if (comboEditorPtr != NULL){
		newValue = comboEditorPtr->currentText();
	}
	else{
		const CMultiAttributeDelegateWidget* multiAttributeEditorPtr = dynamic_cast<const CMultiAttributeDelegateWidget*>(editor);
		if (multiAttributeEditorPtr != NULL){
			newValue = multiAttributeEditorPtr->GetText();
		}
		else{
			newValue = editor->property("text").toString();
		}
	}

	int propertyMining = index.data(AttributeMining).toInt();
	if (propertyMining == AM_EXPORTED_COMP){
		// set attribute export for each selected component (if has the attribute)
		SetComponentExportData(attributeId, newValue);
	}
	else if ((propertyMining >= AM_REFERENCE) && (propertyMining <= AM_EXPORTED_ATTR)){
		// set attribute value for each selected component (if has the attribute)
		SetComponentValue(attributeId, propertyMining, newValue);
	}
}


// protected methods of embedded class AttributeItemDelegate

bool CAttributeEditorComp::AttributeItemDelegate::SetComponentExportEditor(const QByteArray& attributeId, const QString& defaultValue, QWidget& editor) const
{
	QString editorValue = m_parent.GetExportAliases(attributeId).join(";");

	if (editorValue.isEmpty()){
		editorValue = defaultValue;
	}

	return editor.setProperty("text", QVariant(editorValue));
}


bool CAttributeEditorComp::AttributeItemDelegate::SetAttributeExportEditor(const QByteArray& /*id*/, const QByteArray& exportId, QWidget& editor) const
{
	return editor.setProperty("text", QVariant(exportId));
}


bool CAttributeEditorComp::AttributeItemDelegate::SetAttributeValueEditor(
			const QByteArray& id,
			int propertyMining,
			QWidget& editor) const
{
	if (!m_parent.m_metaInfoManagerCompPtr.IsValid()){
		return false;
	}

	const IElementSelectionInfo* selectionInfoPtr = m_parent.GetObservedObject();
	if (selectionInfoPtr == NULL){
		return false;
	}

	IElementSelectionInfo::Elements selectedElements = selectionInfoPtr->GetSelectedElements();
	if (selectedElements.isEmpty()){
		return false;
	}

	for (		IElementSelectionInfo::Elements::ConstIterator elemIter = selectedElements.constBegin();
				elemIter != selectedElements.constEnd();
				++elemIter){
		const icomp::IRegistry::ElementInfo* elementInfoPtr = elemIter.value();
		Q_ASSERT(elementInfoPtr != NULL);
		if (!elementInfoPtr->elementPtr.IsValid()){
			continue;
		}

		const iser::IObject* attributePtr = m_parent.GetAttributeObject(id, *elementInfoPtr);
		if (attributePtr == NULL){
			continue;
		}

		if ((propertyMining == AM_MULTI_REFERENCE) || (propertyMining == AM_MULTI_FACTORY)){
			CMultiAttributeDelegateWidget* multiEditorPtr = dynamic_cast<CMultiAttributeDelegateWidget*>(&editor);
			if (multiEditorPtr == NULL){
				return false;
			}

			const icomp::CMultiReferenceAttribute* multiReferenceAttributePtr = dynamic_cast<const icomp::CMultiReferenceAttribute*>(attributePtr);
			if (multiReferenceAttributePtr != NULL){
				QString valuesString = GetMultiAttributeValueAsString(*multiReferenceAttributePtr);

				multiEditorPtr->SetText(valuesString);

				return true;
			}
		}

		QComboBox* comboEditor = dynamic_cast<QComboBox*>(&editor);
		if ((propertyMining == AM_REFERENCE) || (propertyMining == AM_FACTORY)){
			if (comboEditor == NULL){
				return false;
			}

			const icomp::IRegistry* registryPtr = m_parent.GetRegistry();
			const icomp::IAttributeStaticInfo* staticInfoPtr = m_parent.GetAttributeStaticInfo(id, *elementInfoPtr);
			if ((registryPtr != NULL) && (staticInfoPtr != NULL) && m_parent.m_consistInfoCompPtr.IsValid()){
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

				int attributeFlags = staticInfoPtr->GetAttributeFlags();
				if ((attributeFlags & icomp::IAttributeStaticInfo::AF_REFERENCE) != 0){
					queryFlags = queryFlags | IRegistryConsistInfo::QF_INCLUDE_SUBELEMENTS;
				}
				else if ((attributeFlags & icomp::IAttributeStaticInfo::AF_FACTORY) != 0){
					queryFlags = queryFlags | IRegistryConsistInfo::QF_DETACHED_FROM_CONTAINER;
				}

				icomp::IRegistry::Ids compatIds = m_parent.m_consistInfoCompPtr->GetCompatibleElements(
							obligatoryInterfaces,
							*registryPtr,
							queryFlags);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
				QList< QByteArray> compatIdList(compatIds.begin(), compatIds.end());
#else
				QList<QByteArray> compatIdList = compatIds.toList();
#endif
				std::sort(compatIdList.begin(), compatIdList.end());

				for(		QList< QByteArray>::ConstIterator iter = compatIdList.constBegin();
							iter != compatIdList.constEnd();
							++iter){
					comboEditor->addItem(*iter);
				}
			}

			const icomp::CReferenceAttribute* referenceAttributePtr = dynamic_cast<const icomp::CReferenceAttribute*>(attributePtr);
			if (referenceAttributePtr != NULL){
				comboEditor->lineEdit()->setText(referenceAttributePtr->GetValue());

				return true;
			}

			const icomp::CFactoryAttribute* factoryAttributePtr = dynamic_cast<const icomp::CFactoryAttribute*>(attributePtr);
			if (factoryAttributePtr != NULL){
				comboEditor->lineEdit()->setText(factoryAttributePtr->GetValue());

				return true;
			}
		}
		else if (propertyMining == AM_ATTRIBUTE){
			const iattr::CIntegerAttribute* intAttributePtr = dynamic_cast<const iattr::CIntegerAttribute*>(attributePtr);
			if (intAttributePtr != NULL){
				int value = intAttributePtr->GetValue();
				editor.setProperty("text", QVariant(value));

				return true;
			}

			const iattr::CRealAttribute* doubleAttributePtr = dynamic_cast<const iattr::CRealAttribute*>(attributePtr);
			if (doubleAttributePtr != NULL){
				double value = doubleAttributePtr->GetValue();
				editor.setProperty("text", QString::number(value, 'g', 12));

				return true;
			}

			const iattr::CStringAttribute* stringAttributePtr = dynamic_cast<const iattr::CStringAttribute*>(attributePtr);
			if (stringAttributePtr != NULL){
				const QString& value = stringAttributePtr->GetValue();
				editor.setProperty("text", QVariant(value));

				return true;
			}

			const iattr::CIdAttribute* idAttributePtr = dynamic_cast<const iattr::CIdAttribute*>(attributePtr);
			if (idAttributePtr != NULL){
				const QByteArray& value = idAttributePtr->GetValue();
				editor.setProperty("text", QVariant(value));

				return true;
			}
		}
		else if (propertyMining == AM_BOOL_ATTRIBUTE){
			const iattr::CBooleanAttribute* boolAttributePtr = dynamic_cast<const iattr::CBooleanAttribute*>(attributePtr);
			if (boolAttributePtr != NULL){
				Q_ASSERT(comboEditor != NULL);

				bool value = boolAttributePtr->GetValue();
				comboEditor->setCurrentIndex(value? 0: 1);

				return true;
			}
		}
		else if (propertyMining == AM_MULTI_ATTRIBUTE){
			CMultiAttributeDelegateWidget* multiEditorPtr = dynamic_cast<CMultiAttributeDelegateWidget*>(&editor);
			if (multiEditorPtr == NULL){
				return false;
			}

			const iattr::CIntegerListAttribute* intListAttributePtr = dynamic_cast<const iattr::CIntegerListAttribute*>(attributePtr);
			if (intListAttributePtr != NULL){
				QString outputValue;
				for (int index = 0; index < intListAttributePtr->GetValuesCount(); index++){
					if (index != 0){
						outputValue += ";";
					}

					outputValue += QString::number(intListAttributePtr->GetValueAt(index));
				}

				multiEditorPtr->SetText(outputValue);

				return true;
			}

			const iattr::CRealListAttribute* doubleListAttributePtr = dynamic_cast<const iattr::CRealListAttribute*>(attributePtr);
			if (doubleListAttributePtr != NULL){
				QString outputValue;
				for (int index = 0; index < doubleListAttributePtr->GetValuesCount(); index++){
					if (index != 0){
						outputValue += ";";
					}

					outputValue += QString::number(doubleListAttributePtr->GetValueAt(index), 'g', 12);
				}

				multiEditorPtr->SetText(outputValue);

				return true;
			}

			const iattr::CBooleanListAttribute* boolListAttributePtr = dynamic_cast<const iattr::CBooleanListAttribute*>(attributePtr);
			if (boolListAttributePtr != NULL){
				QString outputValue;
				for (int index = 0; index < boolListAttributePtr->GetValuesCount(); index++){
					if (index != 0){
						outputValue += ";";
					}

					outputValue += boolListAttributePtr->GetValueAt(index)? "true": "false";
				}

				multiEditorPtr->SetText(outputValue);

				return true;
			}

			const iattr::CStringListAttribute* stringListAttributePtr = dynamic_cast<const iattr::CStringListAttribute*>(attributePtr);
			if (stringListAttributePtr != NULL){
				QString outputValue;
				for (int index = 0; index < stringListAttributePtr->GetValuesCount(); index++){
					if (index != 0){
						outputValue += ";";
					}

					outputValue += stringListAttributePtr->GetValueAt(index);
				}

				multiEditorPtr->SetText(outputValue);

				return true;
			}

			const iattr::CIdListAttribute* idListAttributePtr = dynamic_cast<const iattr::CIdListAttribute*>(attributePtr);
			if (idListAttributePtr != NULL){
				QString outputValue;
				for (int index = 0; index < idListAttributePtr->GetValuesCount(); index++){
					if (index != 0){
						outputValue += ";";
					}

					outputValue += idListAttributePtr->GetValueAt(index);
				}

				multiEditorPtr->SetText(outputValue);

				return true;
			}
		}
	}

	return false;
}


bool CAttributeEditorComp::AttributeItemDelegate::SetComponentValue(
			const QByteArray& attributeId,
			int propertyMining,
			const QString& value) const
{
	icomp::IRegistry* registryPtr = m_parent.GetRegistry();
	if (registryPtr == NULL){
		return false;
	}

	AttrInfosMap::ConstIterator attributeInfoMapIter = m_parent.m_attrInfosMap.constFind(attributeId);
	if (attributeInfoMapIter == m_parent.m_attrInfosMap.constEnd()){
		return false;
	}

	istd::CChangeNotifier registryNotifier(registryPtr, &s_setAttributeChangeSet);
	Q_UNUSED(registryNotifier);

	bool retVal = false;

	const ElementIdToAttrInfoMap& elementsMap = attributeInfoMapIter.value();
	for (		ElementIdToAttrInfoMap::ConstIterator elemIter = elementsMap.constBegin();
				elemIter != elementsMap.constEnd();
				++elemIter){
		const AttrInfo& attributeInfo = elemIter.value();

		icomp::IRegistryElement::AttributeInfo* attributeInfoPtr = attributeInfo.infoPtr;
		if (attributeInfo.elementPtr == NULL){
			continue;
		}

		static const istd::IChangeable::ChangeSet elementChangeSet(icomp::IRegistryElement::CF_ATTRIBUTE_CHANGED);
		istd::CChangeNotifier elementNotifier(attributeInfo.elementPtr, &elementChangeSet);
		Q_UNUSED(elementNotifier);

		if ((attributeInfoPtr == NULL) && !value.isEmpty()){
			Q_ASSERT(attributeInfo.staticInfoPtr != NULL);	// attributeInfo.infoPtr or attributeInfo.staticInfoPtr must be valid for attribute!

			QByteArray attributeValueTypeId = attributeInfo.staticInfoPtr->GetAttributeTypeId();

			attributeInfoPtr = attributeInfo.elementPtr->InsertAttributeInfo(attributeId, attributeValueTypeId);
		}

		if (attributeInfoPtr != NULL){
			if (propertyMining == AM_EXPORTED_ATTR){
				attributeInfoPtr->exportId = value.toLocal8Bit();

				retVal = true;
			}
			else{
				if (!attributeInfoPtr->attributePtr.IsValid()){
					attributeInfoPtr->attributePtr.SetPtr(attributeInfo.elementPtr->CreateAttribute(attributeInfoPtr->attributeTypeName));
				}

				if (attributeInfoPtr->attributePtr.IsValid()){
					retVal = m_parent.EncodeAttribute(value, propertyMining, *attributeInfoPtr->attributePtr) || retVal;
				}
			}
		}

		if ((attributeInfoPtr != NULL) && !attributeInfoPtr->attributePtr.IsValid() && attributeInfoPtr->exportId.isEmpty()){
			attributeInfo.elementPtr->RemoveAttribute(attributeId);
		}
 	}

	Q_EMIT m_parent.AfterAttributesChange();

	return retVal;
}


bool CAttributeEditorComp::AttributeItemDelegate::SetComponentExportData(const QByteArray& attributeId, const QString& value) const
{
	icomp::IRegistry* registryPtr = m_parent.GetRegistry();
	if (registryPtr == NULL){
		return false;
	}

	istd::CChangeNotifier registryNotifier(registryPtr, &s_setExportChangeSet);
	Q_UNUSED(registryNotifier);

	icomp::IRegistry::ExportedElementsMap exportedMap = registryPtr->GetExportedElementsMap();
	for (icomp::IRegistry::ExportedElementsMap::ConstIterator iter = exportedMap.constBegin();
				iter != exportedMap.constEnd();
				++iter){
		if (iter.value() == attributeId){
			registryPtr->SetElementExported(iter.key(), "");
		}
	}

	if (!value.isEmpty()){
		registryPtr->SetElementExported(value.toLocal8Bit(), attributeId);
	}

	Q_EMIT m_parent.AfterSubcomponentsChange();

	return true;
}


// public methods of embedded class RegistryObserver

CAttributeEditorComp::RegistryObserver::RegistryObserver(CAttributeEditorComp* parentPtr)
:	m_parent(*parentPtr)
{
	Q_ASSERT(parentPtr != NULL);
}


// protected methods of embedded class RegistryObserver

// reimplemented (imod::CSingleModelObserverBase)

void CAttributeEditorComp::RegistryObserver::OnUpdate(const istd::IChangeable::ChangeSet& changeSet)
{
	if (!changeSet.IsEmpty()){
		m_parent.UpdateEditor(changeSet);
	}
}


} // namespace icmpstr


