// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef icmpstr_CElementSelectionInfoManagerBase_included
#define icmpstr_CElementSelectionInfoManagerBase_included


// ACF includes
#include <istd/TDelPtr.h>
#include <istd/CClassInfo.h>

#include <imod/TSingleModelObserverBase.h>

#include <icomp/IMetaInfoManager.h>
#include <icomp/IRegistryElement.h>

#include <icmpstr/IAttributeSelectionObserver.h>
#include <icmpstr/IElementSelectionInfo.h>
#include <icmpstr/IRegistryConsistInfo.h>


namespace icmpstr
{


/**
	Base class for all components depending of meta information in current selected registry context.
*/
class CElementSelectionInfoManagerBase:
				public imod::TSingleModelObserverBase<IElementSelectionInfo>
{
public:
	typedef imod::TSingleModelObserverBase<IElementSelectionInfo> BaseClass;

	CElementSelectionInfoManagerBase();

	icomp::IRegistry* GetRegistry() const;
	QStringList GetExportAliases(const QByteArray& attributeName) const;
	const iser::IObject* GetAttributeObject(const QByteArray& attributeId, const icomp::IRegistry::ElementInfo& elementInfo) const;
	virtual const icomp::IAttributeStaticInfo* GetAttributeStaticInfo(const QByteArray& attributeId, const icomp::IRegistry::ElementInfo& elementInfo) const;

	// abstract methods
	virtual const icomp::IMetaInfoManager* GetMetaInfoManagerPtr() const = 0;
	virtual const icmpstr::IRegistryConsistInfo* GetConsistencyInfoPtr() const = 0;
};


} // namespace icmpstr


#endif // !icmpstr_CElementSelectionInfoManagerBase_included

