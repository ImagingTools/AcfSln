// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef icmpstr_IElementSelectionInfo_included
#define icmpstr_IElementSelectionInfo_included


// Qt includes
#include <QtCore/QSet>
#include <QtGui/QIcon>

// ACF includes
#include <istd/IChangeable.h>

#include <iser/ISerializable.h>

#include <icomp/IRegistry.h>
#include <icomp/CComponentAddress.h>

#include <icmpstr/icmpstr.h>


namespace icmpstr
{


/**
	Interface containing all information about registry selected element.
*/
class IElementSelectionInfo: virtual public istd::IChangeable
{
public:
	/**
		Data model change notification flags.
	*/
	enum ChangeFlags
	{
		CF_SELECTION = 0x282ce27
	};

	typedef QMap<QByteArray, const icomp::IRegistry::ElementInfo*> Elements;

	/**
		Get registry containing selected element.
		\return	pointer to registry or NULL if no registry is selected.
	*/
	virtual icomp::IRegistry* GetSelectedRegistry() const = 0;
	/**
		Get set of selected elements.
		\return	map from element name to element information object.
				This map doesn't include pointers to NULL element info.
	*/
	virtual Elements GetSelectedElements() const = 0;

	/**
		Request element selection in the visual editor.
		The implementation should select the specified element in the component diagram.
		If embeddedRegistryId is not empty, the embedded composition should be activated first.
		\param	elementId			the element to select.
		\param	embeddedRegistryId	optional ID of the embedded registry to switch to first.
		\return	true if selection request was handled, false otherwise.
	*/
	virtual bool RequestElementSelection(const QByteArray& elementId, const QByteArray& embeddedRegistryId = QByteArray()) const { Q_UNUSED(elementId); Q_UNUSED(embeddedRegistryId); return false; }
};


} // namespace icmpstr


#endif // !icmpstr_IElementSelectionInfo_included


