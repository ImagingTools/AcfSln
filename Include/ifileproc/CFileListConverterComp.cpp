// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <ifileproc/CFileListConverterComp.h>


// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QDir>

// ACF-Solutions includes
#include <ifileproc/CFilePathesContainer.h>


namespace ifileproc
{


// public methods

// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CFileListConverterComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (!m_outputFileNameCompPtr.IsValid()){
		return TS_INVALID;
	}

	const ifile::IFileListProvider* inputFileListProviderPtr = dynamic_cast<const ifile::IFileListProvider*>(inputPtr);
	if (inputFileListProviderPtr == NULL){
		return TS_INVALID;
	}

	auto progressLoggerPtr = StartProgressLogger(progressManagerPtr, true);
	
	TaskState retVal = TS_OK;

	CFilePathesContainer* convertedFileListPtr = dynamic_cast<CFilePathesContainer*>(outputPtr);

	const QFileInfoList& fileList = inputFileListProviderPtr->GetFileList();
	int filesCount = fileList.count();

	double progressStep = 1.0 / (double)filesCount;
	
	if (convertedFileListPtr != NULL){
		convertedFileListPtr->ResetFiles();
	}

	QStringList filesToRemove;

	for (int inputFileIndex = 0; inputFileIndex < filesCount; inputFileIndex++){
		const QString& inputFile = fileList[inputFileIndex].absoluteFilePath();

		QString outputFileName = m_outputFileNameCompPtr->CalculateFileName(inputFile, QString(), NULL);

		QFileInfo outputFileInfo(outputFileName);

		QDir outputDirectory(outputFileInfo.absoluteDir());
		if (!outputDirectory.mkpath(outputFileInfo.absolutePath())){
			SendErrorMessage(0, QObject::tr("Output directory doesn't exist").arg(outputFileInfo.absolutePath()));			
		}

		bool isConverted = false;

		if (m_fileConvertCompPtr.IsValid()){
			isConverted = (m_fileConvertCompPtr->ConvertFiles(inputFile, outputFileName, paramsPtr) == iproc::IProcessor::TS_OK);
		}
		else{
			isConverted = QFile::copy(inputFile, outputFileName);
		}

		if (!isConverted){
			SendErrorMessage(0, QObject::tr("Processing of %1 failed").arg(inputFile));
		}
		else{
			if (convertedFileListPtr != NULL){
				convertedFileListPtr->InsertFile(outputFileName);
			}
		}

		if (*m_inputFilesRemovingEnabledAttrPtr){
			filesToRemove.push_back(inputFile);
		}

		if (progressLoggerPtr != NULL){
			progressLoggerPtr->OnProgress(inputFileIndex * progressStep);

			if (progressLoggerPtr->IsCanceled()){
				return iproc::IProcessor::TS_CANCELED;
			}
		}
	}

	if (progressLoggerPtr != NULL){
		progressLoggerPtr->OnProgress(1.0);
	}

	int removeFilesCount = filesToRemove.size();
	for (int fileIndex = 0; fileIndex < removeFilesCount; fileIndex++){
		QString fileToRemove = filesToRemove[fileIndex];
		if (!QFile::remove(fileToRemove)){
			SendErrorMessage(0, QObject::tr("File %1 could not be removed").arg(fileToRemove));
		}
	}

	return retVal;
}


} // namespace ifileproc


