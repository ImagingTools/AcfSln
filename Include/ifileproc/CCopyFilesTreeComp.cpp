// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <ifileproc/CCopyFilesTreeComp.h>


// Qt includes
#include <QtCore/QObject>
#include <QtCore/QRegularExpression>

// ACF includes
#include <istd/CSystem.h>


namespace ifileproc
{


// reimplemented (ifileproc::IFileConversion)

int CCopyFilesTreeComp::ConvertFiles(
			const QString& inputPath,
			QString& outputPath,
			const iprm::IParamsSet* paramsPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/) const
{
	if (!m_fileCopyCompPtr.IsValid()){
		SendErrorMessage(MI_END_STATUS, QObject::tr("File copy provider is not present"));

		return iproc::IProcessor::TS_INVALID;
	}

	QStringList filters;
	for (int filterIndex = 0; filterIndex < m_filtersAttrPtr.GetCount(); ++filterIndex){
		const QString& filter = m_filtersAttrPtr[filterIndex];

		filters << filter;
	}

	QStringList excludeFilters;
	for (int excludeIndex = 0; excludeIndex < m_excludeFiltersAttrPtr.GetCount(); ++excludeIndex){
		const QString& excludeFilter = m_excludeFiltersAttrPtr[excludeIndex];

		excludeFilters << excludeFilter;
	}

	QDir inputDir(istd::CSystem::GetEnrolledPath(inputPath));
	if (m_inputSubdirAttrPtr.IsValid() && !(*m_inputSubdirAttrPtr).isEmpty()){
		inputDir.setPath(inputDir.filePath(istd::CSystem::GetEnrolledPath(*m_inputSubdirAttrPtr)));
	}

	QDir outputDir(istd::CSystem::GetEnrolledPath(outputPath));
	if (m_outputSubdirAttrPtr.IsValid() && !(*m_outputSubdirAttrPtr).isEmpty()){
		outputDir.setPath(outputDir.filePath(istd::CSystem::GetEnrolledPath(*m_outputSubdirAttrPtr)));
	}

	SendVerboseMessage(QString("Copy files from %1 to %2").arg(inputPath).arg(outputPath));

	int counter = 0;
	if (CopyFileTree(
				inputDir,
				outputDir,
				paramsPtr,
				filters,
				excludeFilters,
				*m_recursionDepthAttrPtr,
				counter)){
		SendInfoMessage(MI_END_STATUS, QObject::tr("Success: %1 files copied").arg(counter));

		return iproc::IProcessor::TS_OK;
	}
	else{
		SendErrorMessage(MI_END_STATUS, QObject::tr("Failed: %1 files copied").arg(counter));

		return iproc::IProcessor::TS_INVALID;
	}
}


// protected methods

bool CCopyFilesTreeComp::CheckFileExistTree(
			const QDir& inputDir,
			const QStringList& filters,
			const QStringList& excludeFilters,
			int recursionDepth) const
{
	QFileInfoList fileList = inputDir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
	for (		QFileInfoList::const_iterator fileIter = fileList.begin();
				fileIter != fileList.end();
				++fileIter){
		const QFileInfo& fileInfo = *fileIter;
		QString fileName = fileInfo.fileName();

		if (!CheckIfExcluded(fileName, excludeFilters)){
			return true;
		}
	}

	if (recursionDepth > 0){
		QFileInfoList dirList = inputDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (		QFileInfoList::const_iterator dirIter = dirList.begin();
					dirIter != dirList.end();
					++dirIter){
			const QFileInfo& fileInfo = *dirIter;
			QString fileName = fileInfo.fileName();

			if (CheckIfExcluded(fileName, excludeFilters)){
				continue;
			}

			QDir inputSubDir(inputDir.absoluteFilePath(fileName));

			if (CheckFileExistTree(inputSubDir, filters, excludeFilters, recursionDepth - 1)){
				return true;
			}
		}
	}

	return false;
}


bool CCopyFilesTreeComp::CopyFileTree(
			const QDir& inputDir,
			const QDir& outputDir,
			const iprm::IParamsSet* paramsPtr,
			const QStringList& filters,
			const QStringList& excludeFilters,
			int recursionDepth,
			int& counter) const
{
	if (!inputDir.exists()){
		SendWarningMessage(MI_NO_INPUT, QObject::tr("No input directory %1").arg(inputDir.absolutePath()));

		return false;
	}

	outputDir.mkpath(outputDir.absolutePath());
	if (!outputDir.exists()){
		SendWarningMessage(MI_NO_OUTPUT, QObject::tr("No output directory %1").arg(outputDir.absolutePath()));

		return false;
	}

	bool retVal = true;

	QFileInfoList fileList = inputDir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
	for (		QFileInfoList::const_iterator fileIter = fileList.begin();
				fileIter != fileList.end();
				++fileIter){
		const QFileInfo& fileInfo = *fileIter;
		QString fileName = fileInfo.fileName();

		if (CheckIfExcluded(fileName, excludeFilters)){
			continue;
		}

		QString inputFilePath = inputDir.absoluteFilePath(fileName);
		QString outputPath = outputDir.absoluteFilePath(fileName);

		Q_ASSERT(m_fileCopyCompPtr.IsValid());	// it should be checked before whole process started

		if (m_fileCopyCompPtr->ConvertFiles(inputFilePath, outputPath, paramsPtr) == iproc::IProcessor::TS_OK){
			counter++;
		}
	}

	if (recursionDepth < 0){
		recursionDepth = INT_MAX;
	}

	if (recursionDepth > 0){
		QFileInfoList dirList = inputDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (		QFileInfoList::const_iterator dirIter = dirList.begin();
					dirIter != dirList.end();
					++dirIter){
			const QFileInfo& fileInfo = *dirIter;
			QString fileName = fileInfo.fileName();

			if (CheckIfExcluded(fileName, excludeFilters)){
				continue;
			}

			QDir inputSubDir(inputDir.absoluteFilePath(fileName));
			QDir outputSubDir(outputDir.absoluteFilePath(fileName));

			if (CheckFileExistTree(inputSubDir, filters, excludeFilters, recursionDepth - 1)){
				retVal = CopyFileTree(inputSubDir, outputSubDir, paramsPtr, filters, excludeFilters, recursionDepth - 1, counter) && retVal;
			}
		}
	}

	return retVal;
}


bool CCopyFilesTreeComp::CheckIfExcluded(const QString& fileName, const QStringList& excludeFilters) const
{
	for (		QStringList::const_iterator excludeIter = excludeFilters.begin();
				excludeIter != excludeFilters.end();
				++excludeIter){

#if QT_VERSION < 0x060000
		QRegExp rx(*excludeIter);
		rx.setPatternSyntax(QRegExp::Wildcard);
		rx.setCaseSensitivity(Qt::CaseInsensitive);
		if (rx.exactMatch(fileName)){
			return true;
		}
#else
		QRegularExpression rx = QRegularExpression(QRegularExpression::wildcardToRegularExpression(*excludeIter), QRegularExpression::CaseInsensitiveOption);

		if (rx.match(fileName).hasMatch()){
			return true;
		}
#endif
	}

	return false;
}


} // namespace ifileproc


