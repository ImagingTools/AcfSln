// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef CompositorPck_included
#define CompositorPck_included


#include <icomp/TModelCompWrap.h>
#include <icomp/TMakeComponentWrap.h>
#include <ibase/TModelObserverCompWrap.h>

#include <icmpstr/CAttributeEditorComp.h>
#include <icmpstr/CPackageOverviewComp.h>
#include <icmpstr/CRegistryLoaderComp.h>
#include <icmpstr/CRegistryPreviewComp.h>
#include <icmpstr/CComponentHelpFileProviderComp.h>
#include <icmpstr/CVisualRegistry.h>
#include <icmpstr/CVisualRegistryEditorComp.h>
#include <icmpstr/CRegistryConsistInfoComp.h>
#include <icmpstr/CRegistryPropEditorComp.h>
#include <icmpstr/CQuickHelpGuiComp.h>
#include <icmpstr/CPackagesLoaderExtComp.h>
#include <icmpstr/CComponentNoteEditorComp.h>
#include <icmpstr/CSceneProviderGuiComp.h>
#include <icmpstr/CXpcEditorComp.h>
#include <icmpstr/CRegistryTreeViewComp.h>
#include <icmpstr/CComponentTreeComp.h>
#include <icmpstr/CConvertComponentCommandComp.h>


/**
	Component manipulation package.
	This package is used to contruct Compositor tool.
*/
namespace CompositorPck
{


typedef ibase::TModelObserverCompWrap<icmpstr::CAttributeEditorComp> AttributeEditor;
typedef icomp::TModelCompWrap<icmpstr::CPackageOverviewComp> PackageOverview;
typedef icmpstr::CRegistryLoaderComp RegistryLoader;
typedef icmpstr::CRegistryPreviewComp RegistryPreview;
typedef icmpstr::CComponentHelpFileProviderComp ComponentHelpFileProvider;
typedef icomp::TMakeComponentWrap<
			imod::TModelWrap<icmpstr::CVisualRegistry>,
			icomp::IRegistry,
			iser::ISerializable,
			istd::IChangeable,
			imod::IModel,
			icmpstr::IComponentNoteController> VisualRegistry;
typedef icomp::TModelCompWrap<icmpstr::CVisualRegistryEditorComp> VisualRegistryScenographer;
typedef icmpstr::CRegistryConsistInfoComp RegistryConsistInfo;
typedef icmpstr::CRegistryPropEditorComp RegistryPropEditor;
typedef icmpstr::CQuickHelpGuiComp QuickHelpGui;
typedef icomp::TModelCompWrap<icmpstr::CPackagesLoaderExtComp> PackagesLoaderExt;
typedef icmpstr::CComponentNoteEditorComp ComponentNoteEditor;
typedef icomp::TModelCompWrap<icmpstr::CSceneProviderGuiComp> SceneProvider;
typedef icmpstr::CXpcEditorComp XpcEditorGui;
typedef icmpstr::CRegistryTreeViewComp RegistryTreeView;
typedef icmpstr::CComponentTreeComp ComponentTree;
typedef ibase::TModelObserverCompWrap<icmpstr::CConvertComponentCommandComp> ConvertComponentCommand;


} // namespace CompositorPck


#endif // !CompositorPck_included


