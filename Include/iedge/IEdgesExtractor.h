// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iedge_IEdgesExtractor_included
#define iedge_IEdgesExtractor_included


// ACF includes
#include <iimg/IBitmap.h>
#include <iproc/IProcessor.h>

// ACF-Solutions includes
#include <iedge/CEdgeLineContainer.h>


namespace iedge
{


/**
	Interface for edge lines extraction from the image.
*/
class IEdgesExtractor: virtual public iproc::IProcessor
{
public:
	/**
		Do extraction of set of edge lines found on the image.
		\param	bitmap		image where the contour lines should be extracted.
		\param	result		container of result edges lines.
		\return				true if success.
	*/
	virtual bool DoContourExtraction(
				const iprm::IParamsSet* paramsPtr,
				const iimg::IBitmap& bitmap,
				CEdgeLineContainer& result,
				istd::IChangeable* processingReportPtr = NULL) const = 0;
};


} // namespace iedge


#endif // !iedge_IEdgesExtractor_included


