// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iedge_IEdgeLinesProcessor_included
#define iedge_IEdgeLinesProcessor_included


// ACF includes
#include <iproc/IProcessor.h>

// ACF-Solutions includes
#include <iedge/CEdgeLineContainer.h>


namespace iedge
{


/**
	Interface for edge lines processing.
*/
class IEdgeLinesProcessor: virtual public iproc::IProcessor
{
public:
	/**
		Process set of edge lines to another set of edge lines.
		\param	edgeLines	set of edge lines used as input.
		\param	result		container of result edges lines.
		\return				true if success.
	*/
	virtual bool DoLinesProcessing(
				const iprm::IParamsSet* paramsPtr,
				const CEdgeLineContainer& edgeLines,
				CEdgeLineContainer& result,
				istd::IChangeable* processingReportPtr = NULL) const = 0;
};


} // namespace iedge


#endif // !iedge_IEdgeLinesProcessor_included


