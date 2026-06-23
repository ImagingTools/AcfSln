// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <ifileproc/CRenderedObjectFileLoaderComp.h>


// Qt includes
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QUuid>
#if QT_VERSION >= 0x050000
#include <QtCore/QStandardPaths>
#else
#include <QtGui/QDesktopServices>
#endif

// ACF includes
#include <istd/CSystem.h>
#include <istd/CChangeNotifier.h>
#include <iser/IArchive.h>
#include <iser/CArchiveTag.h>
#include <iser/CPrimitiveTypesSerializer.h>
#include <iprm/CParamsSet.h>
#include <i2d/CRectangle.h>


namespace ifileproc
{


// public methods

// reimplemented (ifile::IFilePersistence)

bool CRenderedObjectFileLoaderComp::IsOperationSupported(
				const istd::IChangeable* dataObjectPtr,
				const QString* filePathPtr,
				int flags,
				bool beQuiet) const
{
	if ((flags & QF_SAVE) != 0){
		return false;
	}

	int loadersCount = m_fileLoadersCompPtr.GetCount();
	for (int loaderIndex = 0; loaderIndex < loadersCount; ++loaderIndex){
		const ifile::IFilePersistence* loaderPtr = m_fileLoadersCompPtr[loaderIndex];
		if (		(loaderPtr != NULL) &&
					loaderPtr->IsOperationSupported(dataObjectPtr, filePathPtr, flags, beQuiet)){
			return true;
		}
	}

	return false;
}


ifile::IFilePersistence::OperationState CRenderedObjectFileLoaderComp::LoadFromFile(
			istd::IChangeable& data,
			const QString& filePath,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	iimg::IBitmap* bitmapPtr = dynamic_cast<iimg::IBitmap*>(&data);
	if (bitmapPtr == NULL){
		return OS_FAILED;
	}

	ifile::IFilePersistence* fileLoaderPtr = GetLoaderForFile(filePath);
	if (fileLoaderPtr == NULL){
		return OS_FAILED;
	}

	istd::IChangeable* dataObjectPtr = GetDataObjectForFile(filePath);
	if (dataObjectPtr == NULL){
		return OS_FAILED;
	}

	iproc::IProcessor* previewGeneratorPtr = GetPreviewGeneratorForFile(filePath);
	if (previewGeneratorPtr == NULL){
		return OS_FAILED;
	}

	if ((dataObjectPtr != NULL) && (fileLoaderPtr != NULL) && (previewGeneratorPtr != NULL)){
		QFileInfo fileInfo(filePath);
		if (fileInfo.exists()){
			QDateTime fileTimeStamp = fileInfo.lastModified();

			PreviewCache::iterator foundCacheIter = m_previewCache.find(filePath);
			if (foundCacheIter != m_previewCache.end()){
				if (fileTimeStamp == foundCacheIter.value().fileTimeStamp){
					if (bitmapPtr->CopyFrom(*foundCacheIter.value().fileBitmapPtr.GetPtr())){
						return OS_OK;
					}

					return OS_FAILED;
				}
				else{
					m_previewCache.erase(foundCacheIter);
				}
			}

			QString renderingFilePath = filePath;
			if (*m_useTempFileAttrPtr){
				QString tempPath;

#if QT_VERSION < 0x050000
				tempPath = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
#else
				tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#endif
				renderingFilePath = tempPath + "/" + QUuid::createUuid().toString() + "_" + QFileInfo(filePath).fileName();

				istd::CSystem::FileCopy(filePath, renderingFilePath);
			}

			int loadResult = fileLoaderPtr->LoadFromFile(*dataObjectPtr, renderingFilePath);
			if (loadResult == OS_OK){
				istd::CChangeNotifier changePtr(bitmapPtr);

				iprm::CParamsSet previewGenerationParams;
				i2d::CRectangle previewRect(0, 0, *m_widthAttrPtr, *m_heightAttrPtr);

				previewGenerationParams.SetEditableParameter("PreviewRect", &previewRect);

				if (previewGeneratorPtr->DoProcessing(&previewGenerationParams, dataObjectPtr, bitmapPtr)){
					FileInfo fileObjectInfo;
					fileObjectInfo.fileTimeStamp = fileTimeStamp;

					istd::TSmartPtr<iimg::CBitmap> cachedBitmapPtr(new iimg::CBitmap);

					if (cachedBitmapPtr->CopyFrom(*bitmapPtr)){
						fileObjectInfo.fileBitmapPtr = cachedBitmapPtr;

						if (m_maxCacheSizeAttrPtr.IsValid() && (int(m_previewCache.size()) >= *m_maxCacheSizeAttrPtr)){
							m_previewCache.erase(m_previewCache.begin());
						}

						m_previewCache[filePath] = fileObjectInfo;
					}

					return OS_OK;
				}
			}
		}
	}

	return OS_FAILED;
}


ifile::IFilePersistence::OperationState CRenderedObjectFileLoaderComp::SaveToFile(
			const istd::IChangeable& /*data*/,
			const QString& /*filePath*/,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	return ifile::IFilePersistence::OS_FAILED;
}


// reimplemented (ifile::IFileTypeInfo)

bool CRenderedObjectFileLoaderComp::GetFileExtensions(QStringList& result, const istd::IChangeable* dataObjectPtr, int flags, bool doAppend) const
{
	if (!doAppend){
		result.clear();
	}

	int loadersCount = m_fileLoadersCompPtr.GetCount();
	for (int loaderIndex = 0; loaderIndex < loadersCount; ++loaderIndex){
		ifile::IFilePersistence* loaderPtr = m_fileLoadersCompPtr[loaderIndex];
		if (loaderPtr != NULL){
			loaderPtr->GetFileExtensions(result, dataObjectPtr, flags, true);
		}
	}

	return true;
}


QString CRenderedObjectFileLoaderComp::GetTypeDescription(const QString* extensionPtr) const
{
	QString retVal;

	int loadersCount = m_fileLoadersCompPtr.GetCount();
	for (int loaderIndex = 0; loaderIndex < loadersCount; ++loaderIndex){
		ifile::IFilePersistence* loaderPtr = m_fileLoadersCompPtr[loaderIndex];
		if (loaderPtr != NULL){
			QString description = loaderPtr->GetTypeDescription(extensionPtr);
			if (!description.isEmpty()){
				if (!retVal.isEmpty() && (description != retVal)){	// check if got the different descriptions
					return QString();
				}

				retVal = description;
			}
		}
	}

	return retVal;
}

	
// reimplemented (iser::ISerializable)

bool CRenderedObjectFileLoaderComp::Serialize(iser::IArchive& archive)
{
	static iser::CArchiveTag previewCacheTag("PreviewCacheElements", "Elements list in the preview cache", iser::CArchiveTag::TT_MULTIPLE);
	static iser::CArchiveTag previewCacheElementTag("PreviewCacheElement", "An element in the preview cache", iser::CArchiveTag::TT_GROUP, &previewCacheTag);
	static iser::CArchiveTag filePathTag("FilePath", "Path of the rendered file", iser::CArchiveTag::TT_LEAF, &previewCacheElementTag);
	static iser::CArchiveTag fileTimeStampTag("TimeStamp", "Time stamp of the last file update", iser::CArchiveTag::TT_GROUP, &previewCacheElementTag);
	static iser::CArchiveTag bitmapTag("Bitmap", "Rendered bitmap", iser::CArchiveTag::TT_GROUP, &previewCacheElementTag);

	int cacheSize = m_previewCache.size();

	bool retVal = archive.BeginMultiTag(previewCacheTag, previewCacheElementTag, cacheSize);

	if (archive.IsStoring()){
		for (		PreviewCache::const_iterator index = m_previewCache.begin();
					index != m_previewCache.end();
					++index){
			retVal = retVal && archive.BeginTag(previewCacheElementTag);
	
			retVal = retVal && archive.BeginTag(filePathTag);
			QString filePath = index.key();
			retVal = retVal && archive.Process(filePath);
			retVal = retVal && archive.EndTag(filePathTag);

			retVal = retVal && archive.BeginTag(fileTimeStampTag);
			QDateTime fileTimeStamp = index.value().fileTimeStamp;
			retVal = retVal && iser::CPrimitiveTypesSerializer::SerializeDateTime(archive, fileTimeStamp);
			retVal = retVal && archive.EndTag(fileTimeStampTag);

			retVal = retVal && archive.BeginTag(bitmapTag);
			retVal = retVal && index.value().fileBitmapPtr->Serialize(archive);
			retVal = retVal && archive.EndTag(bitmapTag);

			retVal = retVal && archive.EndTag(previewCacheElementTag);
		}
	}
	else{
		istd::CChangeNotifier notifier(this);
		Q_UNUSED(notifier);

		for (int elementIndex = 0; elementIndex < cacheSize; elementIndex++){
			FileInfo fileInfo;

			retVal = retVal && archive.BeginTag(previewCacheElementTag);
	
			retVal = retVal && archive.BeginTag(filePathTag);
			QString filePath;
			retVal = retVal && archive.Process(filePath);
			retVal = retVal && archive.EndTag(filePathTag);

			retVal = retVal && archive.BeginTag(fileTimeStampTag);
			retVal = retVal && iser::CPrimitiveTypesSerializer::SerializeDateTime(archive, fileInfo.fileTimeStamp);
			retVal = retVal && archive.EndTag(fileTimeStampTag);

			istd::TSmartPtr<iimg::CBitmap> bitmapPtr(new iimg::CBitmap);
			retVal = retVal && archive.BeginTag(bitmapTag);
			retVal = retVal && bitmapPtr->Serialize(archive);
			retVal = retVal && archive.EndTag(bitmapTag);

			retVal = retVal && archive.EndTag(previewCacheElementTag);

			if (retVal){
				fileInfo.fileBitmapPtr = bitmapPtr;
				m_previewCache[filePath] = fileInfo;
			}
		}
	}

	retVal = retVal && archive.EndTag(previewCacheTag);

	return retVal;
}


// protected methods

// reimplemented (icomp::CComponentBase)

void CRenderedObjectFileLoaderComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	int loadersCount = m_fileLoadersCompPtr.GetCount();
	for (int loaderIndex = 0; loaderIndex < loadersCount; ++loaderIndex){
		ifile::IFilePersistence* loaderPtr = m_fileLoadersCompPtr[loaderIndex];
		if (loaderPtr != NULL){
			if ((loaderIndex < m_previewGenerationProcessorsCompPtr.GetCount()) && (loaderIndex < m_dataObjectFactCompPtr.GetCount())){
				Configuration configuration;

				configuration.filePersistencePtr = loaderPtr;
				configuration.dataObjectPtr.FromUnique(m_dataObjectFactCompPtr.CreateInstance(loaderIndex));
				configuration.previewGenerationProcessorPtr = m_previewGenerationProcessorsCompPtr[loaderIndex];

				if (configuration.IsValid()){
					m_workingConfigurations.push_back(configuration);
				}
			}
		}
	}
}


// private methods

ifile::IFilePersistence* CRenderedObjectFileLoaderComp::GetLoaderForFile(const QString& filePath) const
{
	QString fileExtension = QFileInfo(filePath).suffix();

	int loadersCount = m_fileLoadersCompPtr.GetCount();
	for (int loaderIndex = 0; loaderIndex < loadersCount; ++loaderIndex){
		ifile::IFilePersistence* loaderPtr = m_fileLoadersCompPtr[loaderIndex];
		if (loaderPtr != NULL){
			QStringList extensions;
			if (loaderPtr->GetFileExtensions(extensions)){
				if (extensions.contains(fileExtension, Qt::CaseInsensitive)){
					if (loaderPtr->IsOperationSupported(NULL, &filePath, QF_FILE | QF_LOAD, true)){
						return loaderPtr;
					}				
				}
			}
		}
	}

	return NULL;
}


istd::IChangeable* CRenderedObjectFileLoaderComp::GetDataObjectForFile(const QString& filePath) const
{
	ifile::IFilePersistence* loaderPtr = GetLoaderForFile(filePath);
	for (int configIndex = 0; configIndex < m_workingConfigurations.count(); ++configIndex){
		if (m_workingConfigurations[configIndex].filePersistencePtr == loaderPtr){
			Q_ASSERT(m_workingConfigurations[configIndex].IsValid());

			return m_workingConfigurations[configIndex].dataObjectPtr.GetPtr();
		}
	}

	return NULL;
}


iproc::IProcessor* CRenderedObjectFileLoaderComp::GetPreviewGeneratorForFile(const QString& filePath) const
{
	ifile::IFilePersistence* loaderPtr = GetLoaderForFile(filePath);
	for (int configIndex = 0; configIndex < m_workingConfigurations.count(); ++configIndex){
		if (m_workingConfigurations[configIndex].filePersistencePtr == loaderPtr){
			Q_ASSERT(m_workingConfigurations[configIndex].IsValid());

			return m_workingConfigurations[configIndex].previewGenerationProcessorPtr;
		}
	}

	return NULL;
}


} // namespace ifileproc


