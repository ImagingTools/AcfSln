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
#include <icomp/IComponentEnvironmentManager.h>
#include <icomp/IRegistryLoader.h>
#include <idoc/IHelpViewer.h>
#include <idoc/IDocumentManager.h>
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
		I_ASSIGN(m_envManagerCompPtr, "EnvironmentManager", "Component environment manager for resolving exported attributes", false, "EnvironmentManager");
		I_ASSIGN_TO(m_registryLoaderCompPtr, m_envManagerCompPtr, false);
		I_ASSIGN(m_documentManagerCompPtr, "DocumentManager", "Document manager for navigating to attribute resolution", false, "DocumentManager");
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
	void on_AttributeTree_itemDoubleClicked(QTreeWidgetItem* item, int column);
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
	/**
		Information about where a delegated attribute is resolved.
	*/
	struct ExportResolutionInfo
	{
		QString filePath;             ///< ACC file where the attribute is resolved
		QString elementId;            ///< Element ID in that registry
		QString value;                ///< The resolved attribute value
		bool resolved = false;        ///< Whether the attribute was found and resolved
		QByteArray embeddedRegistryId;///< Embedded registry ID if resolved inside embedded composition
		int attributeMeaning = AM_NONE; ///< Meaning of the resolved value (AM_REFERENCE, AM_ATTRIBUTE, etc.)
	};

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

	/**
		Find where a delegated attribute (with given exportId) is resolved.
		For each root registry (from GetProjectTargets()), builds a full component
		tree (recursing into embedded sub-registries and external package components,
		like CRegistryTreeViewComp::AddSubcomponents) and searches for the exportId.
		Returns the first match found. For user-interactive disambiguation when
		multiple matches exist, use FindExportResolutionInteractive().

		\param exportId The export name of the delegated attribute
		\param currentRegistryPtr Currently unused, kept for interface compatibility
		\return Resolution information including file path, element ID, and value
	*/
	ExportResolutionInfo FindExportResolution(const QByteArray& exportId, const icomp::IRegistry* currentRegistryPtr) const;

	/**
		Find export resolution with interactive disambiguation.
		Like FindExportResolution, but when multiple matches are found across
		different root registries, the user is asked to choose via a dropdown dialog.

		\param exportId The export name of the delegated attribute
		\return Resolution information for the user-chosen match
	*/
	ExportResolutionInfo FindExportResolutionInteractive(const QByteArray& exportId) const;

	/**
		Internal implementation that searches within a specific root registry tree
		for the given exportId. Tracks visited exportIds to prevent cycles.

		\param rootRegistry The root registry to search within
		\param exportId The export name to search for
		\param visitedExportIds Set of exportIds already being searched (cycle detection)
		\return Resolution information
	*/
	ExportResolutionInfo FindExportResolutionImpl(
				const icomp::IRegistry& rootRegistry,
				const QByteArray& exportId,
				QSet<QByteArray>& visitedExportIds) const;

	/**
		Search a registry and its sub-component trees for where an attribute
		with the given ID is resolved. Recurses into both embedded sub-registries
		and external package components (like CRegistryTreeViewComp::AddSubcomponents).
		When a further export is found, follows the chain within the same root tree
		via FindExportResolutionImpl.

		\param rootRegistry The root registry for re-export chain searches
		\param registry The registry to search
		\param attributeId The attribute ID to search for
		\param depth Current depth in registry traversal (prevents infinite recursion)
		\param visitedExportIds Set of exportIds already being searched (cycle detection)
		\return Resolution information
	*/
	ExportResolutionInfo SearchRegistryForResolution(
				const icomp::IRegistry& rootRegistry,
				const icomp::IRegistry& registry,
				const QByteArray& attributeId,
				int depth,
				QSet<QByteArray>& visitedExportIds) const;

	/**
		Find all resolutions for a given exportId across all root registries.
		Uses GetProjectTargets() to get root registry file paths from the XPC model,
		then loads each root registry via IRegistryLoader::GetRegistryFromFile().
		Builds full component trees for each root registry and collects all matches.

		\param exportId The export name to search for
		\return List of all found resolutions
	*/
	QList<ExportResolutionInfo> FindAllExportResolutions(const QByteArray& exportId) const;

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
	I_REF(icomp::IComponentEnvironmentManager, m_envManagerCompPtr);
	I_REF(icomp::IRegistryLoader, m_registryLoaderCompPtr);
	I_REF(idoc::IDocumentManager, m_documentManagerCompPtr);

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

	QByteArray m_pendingAttributeId;	// attribute to select after next selection change

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

