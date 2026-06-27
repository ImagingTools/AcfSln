// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <ifileproc/CFileConvertOverProcessorComp.h>


// Qt includes
#include <QtCore/QMutexLocker>


namespace ifileproc
{


// public methods

// reimplemented (ifileproc::IFileConversion)

int CFileConvertOverProcessorComp::ConvertFiles(
			const QString& inputPath,
			QString& outputPath,
			const iprm::IParamsSet* paramsSetPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* processingReportPtr) const
{
	QMutexLocker blocker(&m_mutex);

	if (!m_inputFileLoaderCompPtr.IsValid()){
		SendErrorMessage(0, "No file loader defined", "File processing component");

		return iproc::IProcessor::TS_INVALID;
	}

	if (!m_inputDataCompPtr.IsValid()){
		SendErrorMessage(0, "Input data definition was not set", "File processing component");

		return iproc::IProcessor::TS_INVALID;
	}

	if (!m_outputDataCompPtr.IsValid()){
		SendErrorMessage(0, "Output data definition was not set", "File processing component");

		return iproc::IProcessor::TS_INVALID;
	}

	if (!m_inputFileLoaderCompPtr->IsOperationSupported(
					m_inputDataCompPtr.GetPtr(),
					&inputPath,
					ifile::IFilePersistence::QF_LOAD | ifile::IFilePersistence::QF_FILE)){
		SendErrorMessage(0, "File could not be loaded", "File processing component");
		
		return iproc::IProcessor::TS_INVALID;
	}

	if (m_inputFileLoaderCompPtr->LoadFromFile(*m_inputDataCompPtr.GetPtr(), inputPath) != ifile::IFilePersistence::OS_OK){
		SendErrorMessage(0, "File could not be loaded", "File processing component");
		
		return iproc::IProcessor::TS_INVALID;
	}

	if (!m_processorCompPtr.IsValid()){
		SendErrorMessage(0, "File processor not set", "File processing component");

		return iproc::IProcessor::TS_INVALID;
	}

	const iprm::IParamsSet* processingParamsPtr = (paramsSetPtr != NULL) ? paramsSetPtr : m_processingParamsSetCompPtr.GetPtr();
	ibase::IProgressManager* processingProgressManagerPtr = (progressManagerPtr != NULL) ? progressManagerPtr : m_progressManagerCompPtr.GetPtr();

	int processingResult = m_processorCompPtr->DoProcessing(
				processingParamsPtr,
				m_inputDataCompPtr.GetPtr(),
				m_outputDataCompPtr.GetPtr(),
				processingProgressManagerPtr,
				processingReportPtr);
	if (processingResult != iproc::IProcessor::TS_OK){
		SendErrorMessage(0, "File conversion failed", "File processing component");

		return processingResult;
	}

	ifile::IFilePersistence* outputFileLoader = m_inputFileLoaderCompPtr.GetPtr();
	if (m_outputFileLoaderCompPtr.IsValid()){
		outputFileLoader = m_outputFileLoaderCompPtr.GetPtr(); 
	}

	Q_ASSERT(outputFileLoader != NULL);

	return (outputFileLoader->SaveToFile(*m_outputDataCompPtr.GetPtr(), outputPath) == ifile::IFilePersistence::OS_OK) ? iproc::IProcessor::TS_OK : iproc::IProcessor::TS_INVALID;
}


} // namespace ifileproc


