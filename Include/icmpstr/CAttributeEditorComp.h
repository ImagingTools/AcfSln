// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef icmpstr_CAttributeEditorComp_included
#define icmpstr_CAttributeEditorComp_included


// Qt includes
#include <QtGui/QIcon>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#include <QtWidgets/QItemDelegate>
#else
#include <QtGui/QWidget>
#include <QtGui/QItemDelegate>
#endif

// ACF includes
#include <istd/CClassInfo.h>
#include <idoc/IHelpViewer.h>
#include <iqtgui/TDesignerGuiObserverCompBase.h>
#include <iwidgets/CTreeWidgetFilter.h>
#include <iwidgets/CItemDelegate.h>

// ACF-Solutions includes
#include <icmpstr/CElementSelectionInfoManagerBase.h>
#include <GeneratedFiles/icmpstr/ui_CAttributeEditorComp.h>


namespace icmpstr
{


class CAttributeEditorComp:
			public iqtgui::TGuiObserverWrap<
						iqtgui::TDesignerGuiCompBase<Ui::CAttributeEditorComp>,
						CElementSelectionInfoManagerBase>
{
	Q_OBJECT

public:
	typedef iqtgui::TGuiObserverWrap<
				iqtgui::TDesignerGuiCompBase<Ui::CAttributeEditorComp>,
				CElementSelectionInfoManagerBase> BaseClass;

	I_BEGIN_COMPONENT(CAttributeEditorComp);
		I_REGISTER_INTERFACE(imod::IModelEditor);
		I_ASSIGN(m_metaInfoManagerCompPtr, "MetaInfoManager", "Allows access to component meta information", true, "MetaInfoManager");
		I_ASSIGN(m_attributeSelectionObserverCompPtr, "AttributeSelectionObserver", "Attribute selection observer", false, "AttributeSelectionObserver");
		I_ASSIGN(m_quickHelpViewerCompPtr, "QuickHelpGui", "Shows object info during selection using its type", false, "QuickHelpGui");
		I_ASSIGN(m_consistInfoCompPtr, "ConsistencyInfo", "Allows to check consistency of registries and attributes", false, "ConsistencyInfo");
		I_ASSIGN(m_registryPropGuiCompPtr, "RegistryPropGui", "Display and edit registry properties if no element is selected", false, "RegistryPropGui");
		I_ASSIGN_TO(m_registryPropObserverCompPtr, m_registryPropGuiCompPtr, false);
	I_END_COMPONENT;

	enum TabIndex
	{
		TI_GENERAL,
		TI_ATTRIBUTES,
		TI_INTERFACES,
		TI_EXPORTS,
		TI_FLAGS
	};

	enum AttrMeaning
	{
		AM_NONE = 0,
		AM_REFERENCE,
		AM_FACTORY,
		AM_MULTI_REFERENCE,
		AM_MULTI_FACTORY,
		AM_ATTRIBUTE,
		AM_BOOL_ATTRIBUTE,
		AM_MULTI_ATTRIBUTE,
		AM_EXPORTED_ATTR,
		AM_EXPORTED_COMP,
		AM_MULTI
	};

	enum AttributeGroupType
	{
		AGT_ATTRIBUTE,
		AGT_REFERENCE,
		AGT_FACTORY,
		AGT_LAST = AGT_FACTORY
	};

	enum AttributeColumns
	{
		AC_NAME = 0,
		AC_VALUE = 1
	};

	enum AttributeRole
	{
		AttributeMining = Qt::UserRole + 1,
		AttributeId,
		AttributeValue,
		AttributeTypeId,
		ElementId,
		InterfaceName
	};

public:
	CAttributeEditorComp();

	// reimplemented (CElementSelectionInfoManagerBase)
	virtual const icomp::IMetaInfoManager* GetMetaInfoManagerPtr() const;
	virtual const icmpstr::IRegistryConsistInfo* GetConsistencyInfoPtr() const;

protected Q_SLOTS:
	void on_AttributeTree_itemSelectionChanged();
	void on_AttributeTree_itemChanged(QTreeWidgetItem* item, int column);
	void on_InterfacesTree_itemSelectionChanged();
	void on_InterfacesTree_itemChanged(QTreeWidgetItem* item, int column);
	void on_AutoInstanceCB_toggled(bool checked);
	void on_IsDetachedCB_toggled(bool checked);
	void UpdateGeneralView();
	void UpdateAttributesView();
	void UpdateInterfacesView();
	void UpdateFlagsView();
	void UpdateSubcomponentsView();

Q_SIGNALS:
	void AfterAttributesChange();
	void AfterInterfacesChange();
	void AfterSubcomponentsChange();

protected:
	struct AttrInfo
	{
		icomp::IRegistryElement* elementPtr;
		icomp::IRegistryElement::AttributeInfo* infoPtr;
		const icomp::IAttributeStaticInfo* staticInfoPtr;
		const icomp::IComponentStaticInfo* componentStaticInfoPtr;
	};

	typedef QMap<QByteArray, AttrInfo> ElementIdToAttrInfoMap;
	typedef QMap<QByteArray, ElementIdToAttrInfoMap> AttrInfosMap;

	bool SetAttributeToItem(
				AttributeGroupType groupType,
				int& itemIndex,
				const icomp::IRegistry& registry,
				const QByteArray& attributeId,
				const ElementIdToAttrInfoMap& infos,
				const QFont& normalFont,
				const QFont& importantFont,
				bool& hasError,
				bool& hasWarning,
				bool& hasExport) const;
	bool SetInterfaceToItem(
				QTreeWidgetItem& item,
				icomp::IRegistry::ExportedInterfacesMap* interfacesMapPtr,
				const QByteArray& elementId,
				const QByteArray& interfaceName,
				bool& hasWarning,
				bool& hasExport,
				bool readOnly) const;
	bool ResetItem(QTreeWidgetItem& item);
	bool DecodeAttribute(
				const iser::ISerializable& attribute,
				QString& text,
				int& meaning) const;
	bool EncodeAttribute(
				const QString& text,
				int meaning,
				iser::ISerializable& result) const;

	void CreateInterfacesTree(
				const QByteArray& elementId,
				const icomp::IElementStaticInfo* infoPtr,
				icomp::IRegistry::ExportedInterfacesMap& registryInterfaces,
				QTreeWidgetItem* parentItemPtr,
				bool& hasWarning,
				bool& hasExport,
				bool includeSubelement,
				bool readOnly);
	void CreateExportedComponentsTree(
				const QByteArray& elementId,
				const QByteArray& globalElementId,
				const icomp::IElementStaticInfo* elementMetaInfoPtr,
				QTreeWidgetItem& item,
				bool& hasWarning,
				bool& hasExport,
				bool readOnly) const;

	// reimplemented (iqt::TGuiObserverWrap)
	virtual void OnGuiModelDetached() override;
	virtual void UpdateGui(const istd::IChangeable::ChangeSet& changeSet) override;

	// reimplemented (CGuiComponentBase)
	virtual void OnGuiCreated() override;
	virtual void OnGuiDestroyed() override;

	// static methods
	static QString DecodeFromEdit(const QString& text);
	static QString EncodeToEdit(const QString& text);

private:
	class AttributeItemDelegate: public iwidgets::CItemDelegate
	{
	public:
		typedef iwidgets::CItemDelegate BaseClass;

		AttributeItemDelegate(CAttributeEditorComp* parentPtr);

		template <class AttributeImpl>
		static QString GetMultiAttributeValueAsString(const AttributeImpl& attribute);

		// reimplemented (QItemDelegate)
		virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		virtual void setEditorData(QWidget* editor, const QModelIndex& index ) const override;
		virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

	protected:
		bool SetComponentExportEditor(const QByteArray& attributeId, const QString& defaultValue, QWidget& editor) const;
		bool SetAttributeExportEditor(const QByteArray& id, const QByteArray& exportId, QWidget& editor) const;
		bool SetAttributeValueEditor(const QByteArray& id, int propertyMining, QWidget& editor) const;

		bool SetComponentValue(const QByteArray& attributeId, int propertyMining, const QString& value) const;
		bool SetComponentExportData(const QByteArray& attributeId, const QString& value) const;

	private:
		CAttributeEditorComp& m_parent;
	};

	class RegistryObserver: public imod::CSingleModelObserverBase
	{
	public:
		RegistryObserver(CAttributeEditorComp* parentPtr);

	protected:
		// reimplemented (imod::CSingleModelObserverBase)
		virtual void OnUpdate(const istd::IChangeable::ChangeSet& changeSet) override;

	private:
		CAttributeEditorComp& m_parent;
	};

	I_REF(icomp::IMetaInfoManager, m_metaInfoManagerCompPtr);
	I_REF(IAttributeSelectionObserver, m_attributeSelectionObserverCompPtr);
	I_REF(idoc::IHelpViewer, m_quickHelpViewerCompPtr);
	I_REF(IRegistryConsistInfo, m_consistInfoCompPtr);
	I_REF(iqtgui::IGuiObject, m_registryPropGuiCompPtr);
	I_REF(imod::IObserver, m_registryPropObserverCompPtr);

	AttributeItemDelegate m_attributeItemDelegate;
	RegistryObserver m_registryObserver;

	typedef QPair<QString, AttributeGroupType> TypeDescr;
	typedef QMap<QByteArray, TypeDescr> AttributeTypesMap;
	AttributeTypesMap m_attributeTypesMap;

	AttrInfosMap m_attrInfosMap;	// all current displayed attributes

	istd::TDelPtr<iwidgets::CTreeWidgetFilter> m_attributesTreeFilter;
	istd::TDelPtr<iwidgets::CTreeWidgetFilter> m_interfacesTreeFilter;
	istd::TDelPtr<iwidgets::CTreeWidgetFilter> m_subcomponentsTreeFilter;

	imod::IModel* m_lastRegistryModelPtr;

	QIcon m_invalidIcon;
	QIcon m_warningIcon;
	QIcon m_exportIcon;
	QIcon m_importIcon;
};


// public static methods

template <class AttributeImpl>
QString CAttributeEditorComp::AttributeItemDelegate::GetMultiAttributeValueAsString(const AttributeImpl& attribute)
{
	QString valuesString;
	for (int index = 0; index < attribute.GetValuesCount(); index++){
		if (!valuesString.isEmpty()){
			valuesString += ";";
		}

		valuesString += attribute.GetValueAt(index);
	}

	return valuesString;
}


} // namespace icmpstr


#endif // !icmpstr_CAttributeEditorComp_included

