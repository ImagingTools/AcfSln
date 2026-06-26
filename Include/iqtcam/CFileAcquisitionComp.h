// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iqtcam_CFileAcquisitionComp_included
#define iqtcam_CFileAcquisitionComp_included


// Qt includes
#include <QtCore/QDir>
#include <QtCore/QStringList>
#include <QtCore/QMutex>

// ACF includes
#include <ifile/IFilePersistence.h>
#include <ifile/IFileNameParam.h>
#include <ilog/TLoggerCompWrap.h>
#include <iproc/TSyncProcessorWrap.h>

// ACF-Solutions includes
#include <icam/IBitmapAcquisition.h>

#include <iqtcam/iqtcam.h>


namespace iqtcam
{


/**
	Bitmap loader component implementing interface \c icam::IBitmapAcquisition over \c ifile::IFilePersistence.
*/
class CFileAcquisitionComp:
			public ilog::CLoggerComponentBase,
			virtual public iproc::TSyncProcessorWrap<icam::IBitmapAcquisition>
{
public:
	typedef ilog::CLoggerComponentBase BaseClass;

	I_BEGIN_COMPONENT(CFileAcquisitionComp);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_REGISTER_INTERFACE(icam::IBitmapAcquisition);
		I_ASSIGN(m_bitmapLoaderCompPtr, "BitmapLoader", "Load bitmap from file", true, "BitmapLoader");
		I_ASSIGN(m_defaultDirAttrPtr, "DefaultDir", "Default directory or image file path, that will be used if no parameters are specified", true, ".");
		I_ASSIGN(m_defaultPathParamCompPtr, "DefaultPathParam", "Path of file or directory will be used if no parameters are specified in paramter set", false, "DefaultDirParam");
		I_ASSIGN(m_pathParamIdAttrPtr, "DirParamId", "Id used to get directory parameter (ifile::IFileNameParam)", true, "FileBitmapAcquisition");
		I_ASSIGN(m_maxCachedDirectoriesAttrPtr, "MaxCachedDirs", "Maximum number of cached directories", true, 10);
		I_ASSIGN(m_lastFileNameCompPtr, "LastFileName", "Stores last processed file name here if set", false, "LastFileName");
		I_ASSIGN(m_acceptedFileNamePatternAttrPtr, "AcceptedFileNamePattern", "Text pattern to be matched for file acceptance", false, "");
	I_END_COMPONENT;

	CFileAcquisitionComp();

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL);

	// reimplemented (icam::IBitmapAcquisition)
	virtual istd::CIndex2d GetBitmapSize(const iprm::IParamsSet* paramsPtr) const override;

	// reimplemented (ilog::CLoggerComponentBase)
	virtual void OnComponentCreated() override;

protected:
	struct ParamsInfo
	{
		ParamsInfo();

		QStringList files;
		QStringList::Iterator filesIter;
		quint32 idStamp;
	};

	typedef QMap<QString, ParamsInfo> DirInfos;
	DirInfos m_dirInfos;

	quint32 m_lastIdStamp;

	istd::CIndex2d m_lastImageSize;

	mutable QMutex m_lock;

private:
	I_REF(ifile::IFilePersistence, m_bitmapLoaderCompPtr);
	I_REF(ifile::IFileNameParam, m_defaultPathParamCompPtr);
	I_REF(ifile::IFileNameParam, m_lastFileNameCompPtr);
	I_ATTR(QString, m_defaultDirAttrPtr);
	I_ATTR(QByteArray, m_pathParamIdAttrPtr);
	I_ATTR(int, m_maxCachedDirectoriesAttrPtr);
	I_ATTR(QString, m_acceptedFileNamePatternAttrPtr);
};


} // namespace iqtcam


#endif // !iqtcam_CFileAcquisitionComp_included


