// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ifileproc_CFileListConverterComp_included
#define ifileproc_CFileListConverterComp_included


// ACF includes
#include <istd/TPointerVector.h>
#include <iproc/TSyncProcessorCompBase.h>
#include <ifile/IFileListProvider.h>
#include <ifileproc/IFileConversion.h>

// ACF-Solutions includes
#include <ifileproc/IFileNaming.h>


namespace ifileproc
{


class CFileListConverterComp: public iproc::CSyncProcessorCompBase
{
public:
	typedef iproc::CSyncProcessorCompBase BaseClass;

	I_BEGIN_COMPONENT(CFileListConverterComp);
		I_ASSIGN(m_fileConvertCompPtr, "FileCopy", "File conversion component used for execution. If not set, the files will be only copied", false, "FileCopy");
		I_ASSIGN(m_outputFileNameCompPtr, "OutputFileNaming", "File naming component for the processing output", true, "OutputFileNaming");
		I_ASSIGN(m_inputFilesRemovingEnabledAttrPtr, "InputFilesRemovingEnabled", "If set, the input files will be removed from the file system", true, false);
	I_END_COMPONENT;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL);

private:
	I_REF(ifileproc::IFileConversion, m_fileConvertCompPtr);
	I_REF(IFileNaming, m_outputFileNameCompPtr);
	I_ATTR(bool, m_inputFilesRemovingEnabledAttrPtr);
};


} // namespace ifileproc


#endif // !ifileproc_CFileListConverterComp_included


