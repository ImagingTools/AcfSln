// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iqtcam/CFileAcquisitionComp.h>


// Qt includes
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>
#include <QtGui/QImageReader>
#include <QtCore/QStringList>
#include <QtCore/QRegularExpression>

// ACF includes
#include <istd/CChangeNotifier.h>
#include <iprm/IParamsSet.h>
#include <iimg/IRasterImage.h>
#include <iimg/IQImageProvider.h>
#include <iprm/TParamsPtr.h>


namespace iqtcam
{


CFileAcquisitionComp::CFileAcquisitionComp()
:	m_lastIdStamp(0),
	m_lastImageSize(-1, -1)
{
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CFileAcquisitionComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* /*inputPtr*/,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	QMutexLocker lock(&m_lock);

	Q_ASSERT(m_defaultDirAttrPtr.IsValid());	// obligatory attribute
	Q_ASSERT(m_pathParamIdAttrPtr.IsValid());	// obligatory attribute
	Q_ASSERT(m_maxCachedDirectoriesAttrPtr.IsValid());	// obligatory attribute

	if (!m_bitmapLoaderCompPtr.IsValid()){
		SendVerboseMessage("Bitmap loader was not set");

		return TS_INVALID;
	}

	QString inputPath = *m_defaultDirAttrPtr;
	iprm::TParamsPtr<ifile::IFileNameParam> pathParamsPtr(paramsPtr, m_pathParamIdAttrPtr, m_defaultPathParamCompPtr, false);
	if (pathParamsPtr.IsValid()){
		inputPath = pathParamsPtr->GetPath();
	}

	QString imageFileName;

	QFileInfo inputPathInfo(inputPath);
	if (inputPathInfo.isDir()){
		QDir directory(inputPath);

		QStringList extensions;
		m_bitmapLoaderCompPtr->GetFileExtensions(extensions, outputPtr, ifile::IFilePersistence::QF_LOAD);

		ParamsInfo& info = m_dirInfos[inputPath];

		if (info.filesIter == info.files.end()){
			QStringList nameFilters;

			if (!extensions.isEmpty()){
				for (		QStringList::iterator iter = extensions.begin();
							iter != extensions.end();
							++iter){
					nameFilters << (QString("*.") + *iter);
				}
			}
			else{
				nameFilters << "*.bmp";
				nameFilters << "*.png";
				nameFilters << "*.jpg";
			}

			info.files = directory.entryList(nameFilters, QDir::Files | QDir::Readable);

			if (m_acceptedFileNamePatternAttrPtr.IsValid() && !(*m_acceptedFileNamePatternAttrPtr).isEmpty()){
				QMutableListIterator<QString> fileListIter(info.files);
				while (fileListIter.hasNext()){
					const QString& file = fileListIter.next();
					if (!file.contains(QRegularExpression(*m_acceptedFileNamePatternAttrPtr))){
						fileListIter.remove();
					}
				}
			}

			info.filesIter = info.files.end();
		}

		info.idStamp = ++m_lastIdStamp;

		info.filesIter++;
		if (info.filesIter >= info.files.end()){
			info.filesIter = info.files.begin();
		}

		if (info.filesIter != info.files.end()){
			imageFileName = directory.absoluteFilePath(*info.filesIter);
		}
	}
	else if (inputPathInfo.isFile()){
		imageFileName = inputPath;
	}

	iproc::IProcessor::TaskState retVal = TS_INVALID;

	if (!imageFileName.isEmpty()){
		if (outputPtr != NULL){
			int loadState = m_bitmapLoaderCompPtr->LoadFromFile(*outputPtr, imageFileName);
			retVal = (loadState == ifile::IFilePersistence::OS_OK)? TS_OK: TS_INVALID;

			iimg::IRasterImage* imagePtr = dynamic_cast<iimg::IRasterImage*>(outputPtr);
			if (imagePtr != NULL){
				m_lastImageSize = imagePtr->GetImageSize();
			}

			// #9125
			if (m_lastFileNameCompPtr.IsValid()){
				m_lastFileNameCompPtr->SetPath(imageFileName);
			}
		}
		else{
			retVal = TS_OK;
		}
	}
	else {
		// #9125
		if (m_lastFileNameCompPtr.IsValid()) {
			m_lastFileNameCompPtr->SetPath("");
		}
	}

	if (m_dirInfos.count() > *m_maxCachedDirectoriesAttrPtr){
		DirInfos::iterator maxIdStampDiffIter = m_dirInfos.end();
		quint32 maxIdStampDiff = 0;

		for (		DirInfos::iterator iter = m_dirInfos.begin();
					iter != m_dirInfos.end();
					++iter){
			quint32 idStampDiff = m_lastIdStamp - iter.value().idStamp;
			if (idStampDiff >= maxIdStampDiff){
				maxIdStampDiff = idStampDiff;
				maxIdStampDiffIter = iter;
			}
		}

		if (maxIdStampDiffIter != m_dirInfos.end()){
			m_dirInfos.erase(maxIdStampDiffIter);	// remove element recently not used
		}
	}

	return retVal;
}


// reimplemented (icam::IBitmapAcquisition)

istd::CIndex2d CFileAcquisitionComp::GetBitmapSize(const iprm::IParamsSet* /*paramsPtr*/) const
{
	QMutexLocker lock(&m_lock);

	return m_lastImageSize;
}


// reimplemented (ilog::CLoggerComponentBase)

void CFileAcquisitionComp::OnComponentCreated()
{
	QMutexLocker lock(&m_lock);

	BaseClass::OnComponentCreated();

	// preinitialize components
	m_bitmapLoaderCompPtr.EnsureInitialized();
	m_defaultPathParamCompPtr.EnsureInitialized();
	m_lastFileNameCompPtr.EnsureInitialized();
}


// public methods of embedded class ParamsInfo

CFileAcquisitionComp::ParamsInfo::ParamsInfo()
:	idStamp(0)
{
	filesIter = files.end();
}


} // namespace iqtcam


