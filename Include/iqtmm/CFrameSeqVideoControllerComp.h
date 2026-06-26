// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iqtmm_CFrameSeqVideoControllerComp_included
#define iqtmm_CFrameSeqVideoControllerComp_included


// Qt includes
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QFileInfoList>

// ACF includes
#include <ifile/IFilePersistence.h>
#include <imod/CSingleModelObserverBase.h>
#include <icomp/CComponentBase.h>
#include <iproc/TSyncProcessorCompBase.h>

// ACF-Solutions includes
#include <icam/IBitmapAcquisition.h>
#include <imm/IVideoController.h>


namespace iqtmm
{


/**
	Component to play some image (frame) sequence from a directory.
*/
class CFrameSeqVideoControllerComp:
			public QObject,
			public iproc::TSyncProcessorCompBase<icam::IBitmapAcquisition>,
			virtual public imm::IVideoController,
			protected imod::CSingleModelObserverBase
{
	Q_OBJECT

public:
	typedef iproc::TSyncProcessorCompBase<icam::IBitmapAcquisition> BaseClass;

	I_BEGIN_COMPONENT(CFrameSeqVideoControllerComp);
		I_REGISTER_INTERFACE(istd::IChangeable);
		I_REGISTER_INTERFACE(imm::IMediaController);
		I_REGISTER_INTERFACE(imm::IVideoInfo);
		I_REGISTER_INTERFACE(imm::IVideoController);
		I_ASSIGN(m_framesPerSecondAttrPtr, "FramesPerSecond", "Default number of frames per second if this info is unavailable from video", true, 25.0);
		I_ASSIGN(m_frameLoaderCompPtr, "FrameLoader", "Loader for the single frame", true, "FrameLoader");
		I_ASSIGN(m_frameDataCompPtr, "FrameData", "Frame instance will be loaded with loader", true, "FrameData");
	I_END_COMPONENT;

	CFrameSeqVideoControllerComp();

	// reimplemented (icam::IBitmapAcquisition)
	virtual istd::CIndex2d GetBitmapSize(const iprm::IParamsSet* paramsPtr) const override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (imm::IMediaController)
	virtual QString GetOpenedMediumUrl() const override;
	virtual bool OpenMediumUrl(const QString& url, bool autoPlay = true) override;
	virtual void CloseMedium() override;
	virtual bool IsPlaying() const override;
	virtual bool SetPlaying(bool state = true) override;
	virtual double GetMediumLength() const override;
	virtual double GetCurrentPosition() const override;
	virtual bool SetCurrentPosition(double position) override;
	virtual int GetSupportedFeatures() const override;

	// reimplemented (imm::IVideoInfo)
	virtual int GetFramesCount() const override;
	virtual double GetFrameInterval() const override;
	virtual istd::CIndex2d GetFrameSize() const override;
	virtual double GetPixelAspectRatio() const override;

	// reimplemented (imm::IVideoController)
	virtual int GetCurrentFrame() const override;
	virtual bool SetCurrentFrame(int frameIndex) override;

	// reimplemented (ifile::IFileTypeInfo)
	virtual bool GetFileExtensions(QStringList& result, const istd::IChangeable* dataObjectPtr = NULL, int flags = -1, bool doAppend = false) const override;
	virtual QString GetTypeDescription(const QString* extensionPtr = NULL) const override;

protected:
	bool LoadCurrentFrame();

private:
	I_ATTR(QString, m_defaultSequenceDirectoryAttrPtr);
	I_ATTR(double, m_framesPerSecondAttrPtr);
	I_REF(ifile::IFilePersistence, m_frameLoaderCompPtr);
	I_REF(istd::IChangeable, m_frameDataCompPtr);

	int m_currentFrameIndex;

	QString m_mediumUrl;

	QFileInfoList m_fileList;
	bool m_isPlaying;

	bool m_isFrameLoaded;
};


} // namespace iqtmm


#endif // !iqtmm_CFrameSeqVideoControllerComp_included

