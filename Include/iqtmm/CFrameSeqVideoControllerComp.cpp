// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iqtmm/CFrameSeqVideoControllerComp.h>


// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QDir>

// ACF includes
#include <istd/CChangeNotifier.h>


namespace iqtmm
{


const istd::IChangeable::ChangeSet s_openMediaChangeSet(imm::IMediaController::CF_STATUS, imm::IMediaController::CF_MEDIA_POSITION);
const istd::IChangeable::ChangeSet s_closeMediaChangeSet(imm::IMediaController::CF_STATUS);
const istd::IChangeable::ChangeSet s_startStopChangeSet(imm::IMediaController::CF_STATUS);
const istd::IChangeable::ChangeSet s_setPositionChangeSet(imm::IMediaController::CF_MEDIA_POSITION);


// public methods

CFrameSeqVideoControllerComp::CFrameSeqVideoControllerComp()
:	m_currentFrameIndex(-1),
	m_isPlaying(false),
	m_isFrameLoaded(false)
{
}


// reimplemented (icam::IBitmapAcquisition)

istd::CIndex2d CFrameSeqVideoControllerComp::GetBitmapSize(const iprm::IParamsSet* /*paramsPtr*/) const
{
	return GetFrameSize();
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CFrameSeqVideoControllerComp::DoProcessing(
			const iprm::IParamsSet* /*paramsPtr*/,
			const istd::IPolymorphic* /*inputPtr*/,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/)
{
	if (outputPtr == NULL){
		return TS_OK;
	}

	iimg::IBitmap* bitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (bitmapPtr != NULL){
		if (m_isFrameLoaded){
			const istd::IChangeable* imagePtr = m_frameDataCompPtr.GetPtr();
			if ((imagePtr != NULL) && bitmapPtr->CopyFrom(*imagePtr)){
				return TS_OK;
			}
		}

		if (m_frameLoaderCompPtr.IsValid() && (m_currentFrameIndex < m_fileList.count())){
			QString filePath = m_fileList.at(m_currentFrameIndex).absoluteFilePath();

			istd::CChangeNotifier notifier(bitmapPtr);

			if (m_frameLoaderCompPtr->LoadFromFile(*bitmapPtr, filePath) == ifile::IFilePersistence::OS_OK){
				return TS_OK;
			}
		}
	}

	return TS_INVALID;
}


// reimplemented (imm::IMediaController)

QString CFrameSeqVideoControllerComp::GetOpenedMediumUrl() const
{
	return m_mediumUrl;
}


bool CFrameSeqVideoControllerComp::OpenMediumUrl(const QString& url, bool autoPlay)
{
	istd::CChangeNotifier notifier(this, &s_openMediaChangeSet);
	Q_UNUSED(notifier);

	m_mediumUrl = url;

	QStringList fileExtensions;
	
	m_frameLoaderCompPtr->GetFileExtensions(fileExtensions, m_frameDataCompPtr.GetPtr(), ifile::IFileTypeInfo::QF_LOAD);

	QStringList fileFilter;
	for (int fileExtensionIndex = 0; fileExtensionIndex < int(fileExtensions.size()); fileExtensionIndex++){
		fileFilter.push_back(QString("*.") + fileExtensions[fileExtensionIndex]);
	}
	
	QDir mediumDir(m_mediumUrl);

	m_fileList = mediumDir.entryInfoList(fileFilter, QDir::Files);

	SetCurrentFrame(-1);

	SetPlaying(autoPlay);

	return true;
}


void CFrameSeqVideoControllerComp::CloseMedium()
{
	istd::CChangeNotifier notifier(this, &s_closeMediaChangeSet);
	Q_UNUSED(notifier);

	SetPlaying(false);
}


bool CFrameSeqVideoControllerComp::IsPlaying() const
{
	return m_isPlaying;
}


bool CFrameSeqVideoControllerComp::SetPlaying(bool state)
{
	istd::CChangeNotifier notifier(this, &s_startStopChangeSet);
	Q_UNUSED(notifier);

	if (m_isPlaying != state){
		m_isPlaying = state;
	}

	return true;
}


double CFrameSeqVideoControllerComp::GetMediumLength() const
{
	return 0;
}


double CFrameSeqVideoControllerComp::GetCurrentPosition() const
{
	return GetCurrentFrame() / *m_framesPerSecondAttrPtr;
}


bool CFrameSeqVideoControllerComp::SetCurrentPosition(double position)
{
	return SetCurrentFrame(position * *m_framesPerSecondAttrPtr + 0.5);
}


int CFrameSeqVideoControllerComp::GetSupportedFeatures() const
{
	return SF_OPEN_MEDIA | SF_PLAY | SF_SEEK;
}


// reimplemented (imm::IVideoInfo)

int CFrameSeqVideoControllerComp::GetFramesCount() const
{
	return m_fileList.count();
}


double CFrameSeqVideoControllerComp::GetFrameInterval() const
{
	return 1.0 / *m_framesPerSecondAttrPtr;
}


istd::CIndex2d CFrameSeqVideoControllerComp::GetFrameSize() const
{
	return istd::CIndex2d::GetInvalid();
}


double CFrameSeqVideoControllerComp::GetPixelAspectRatio() const
{
	return 1;
}


// reimplemented (imm::IVideoController)

int CFrameSeqVideoControllerComp::GetCurrentFrame() const
{
	return m_currentFrameIndex;
}


bool CFrameSeqVideoControllerComp::SetCurrentFrame(int frameIndex)
{
	if (frameIndex != m_currentFrameIndex){
		istd::CChangeNotifier notifier(this, &s_setPositionChangeSet);
		Q_UNUSED(notifier);

		m_currentFrameIndex = frameIndex;

		m_isFrameLoaded = LoadCurrentFrame();
	}

	return m_isFrameLoaded;
}


// reimplemented (ifile::IFileTypeInfo)

bool CFrameSeqVideoControllerComp::GetFileExtensions(QStringList& result, const istd::IChangeable* /*dataObjectPtr*/, int flags, bool doAppend) const
{
	if (!doAppend){
		result.clear();
	}

	if ((flags & QF_LOAD) != 0){
		result.push_back("");
	}

	return true;
}


QString CFrameSeqVideoControllerComp::GetTypeDescription(const QString* /*extensionPtr*/) const
{
	return "Video files";
}


// protected methods

bool CFrameSeqVideoControllerComp::LoadCurrentFrame()
{
	// reset start frame:
	if (m_currentFrameIndex < 0){
		iimg::IRasterImage* imagePtr = dynamic_cast<iimg::IRasterImage*>(m_frameDataCompPtr.GetPtr());
		if (imagePtr != NULL){
			istd::CChangeNotifier changePtr(imagePtr);

			imagePtr->ResetImage();
		}

		return true;
	}

	if (m_frameLoaderCompPtr.IsValid() && m_frameDataCompPtr.IsValid() && (m_currentFrameIndex < m_fileList.count())){
		QString currentFile = m_fileList[m_currentFrameIndex].absoluteFilePath();

		int loadState = m_frameLoaderCompPtr->LoadFromFile(*m_frameDataCompPtr, currentFile);

		return (loadState == ifile::IFilePersistence::OS_OK);
	}
	else{
		return false;
	}
}


} // namespace iqtmm


