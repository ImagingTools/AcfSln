// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iqtmm/CSvgBitmapLoaderComp.h>


// Qt includes
#include <QtCore/QFileInfo>
#include <QtSvg/QSvgRenderer>
#include <QtGui/QPainter>


// ACF includes
#include <iimg/IBitmap.h>
#include <iimg/IQImageProvider.h>


namespace iqtmm
{


// public methods


// reimplemented (ifile::IFilePersistence)

bool CSvgBitmapLoaderComp::IsOperationSupported(
			const istd::IChangeable* dataObjectPtr,
			const QString* filePathPtr,
			int flags,
			bool beQuiet) const
{
	if (		(dataObjectPtr != NULL) &&
				(dynamic_cast<const iimg::IRasterImage*>(dataObjectPtr) == NULL)){
		if (!beQuiet){
			SendInfoMessage(MI_BAD_OBJECT_TYPE, tr("Object is not a valid"));
		}

		return false;
	}

	if (filePathPtr != NULL){
		if (filePathPtr->isEmpty()){
			return false;
		}

		QFileInfo info(*filePathPtr);

		QStringList extensions;
		if (GetFileExtensions(extensions, dataObjectPtr, flags)){
			QStringList extensionsList = (extensions);

			if (!extensionsList.contains(info.suffix(), Qt::CaseInsensitive)){
				if (!beQuiet){
					SendInfoMessage(MI_BAD_EXTENSION, tr("Bad SVG file extension %1").arg(info.suffix()));
				}

				return false;
			}
		}
	}

	return ((flags & QF_FILE) != 0) && ((flags & QF_LOAD) != 0);
}


ifile::IFilePersistence::OperationState CSvgBitmapLoaderComp::LoadFromFile(
			istd::IChangeable& data,
			const QString& filePath,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	if (IsOperationSupported(&data, &filePath, QF_LOAD | QF_FILE, false)){
		Q_ASSERT(!filePath.isEmpty());	// should be checked by IsOperationSupported

		iimg::IRasterImage* imagePtr = dynamic_cast<iimg::IBitmap*>(&data);
		Q_ASSERT(imagePtr != NULL);	// should be checked by IsOperationSupported
		if (filePath != m_lastFilePath){
			QSvgRenderer renderer;
			if (renderer.load(filePath)){
				if (!m_lastBitmap.CreateBitmap(iimg::IBitmap::PF_RGBA, istd::CIndex2d(*m_bitmapWidthAttrPtr, *m_bitmapHeightAttrPtr))){
					m_lastFilePath.clear();

					SendInfoMessage(MI_BITMAP_TYPE, tr("Cannot create bitmap"));

					return OS_FAILED;
				}

				QPainter painter(&m_lastBitmap.GetQImageRef());

				renderer.render(&painter);

				m_lastFilePath = filePath;
			}
			else{
				SendInfoMessage(MI_CANNOT_LOAD, tr("Cannot load SVG file '%1'").arg(filePath));

				return OS_FAILED;
			}
		}

		if (imagePtr->CopyFrom(m_lastBitmap)){
			return OS_OK;
		}
		else{
			SendInfoMessage(MI_BITMAP_TYPE, tr("Cannot copy bitmap from Qt bitmap"));
		}
	}

	return OS_FAILED;
}


ifile::IFilePersistence::OperationState CSvgBitmapLoaderComp::SaveToFile(
			const istd::IChangeable&/* data*/,
			const QString&/* filePath*/,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	return OS_FAILED;
}


// reimplemented (ifile::IFileTypeInfo)

bool CSvgBitmapLoaderComp::GetFileExtensions(QStringList& result, const istd::IChangeable* /*dataObjectPtr*/, int flags, bool doAppend) const
{
	if (!doAppend){
		result.clear();
	}

	if ((flags & QF_LOAD) != 0){
		result.push_back("svg");
	}

	return true;
}


QString CSvgBitmapLoaderComp::GetTypeDescription(const QString* extensionPtr) const
{
	bool isKnown = (extensionPtr == NULL);

	if (!isKnown){
		QStringList extensions;
		if (GetFileExtensions(extensions)){
			QStringList extensionsList = (extensions);
			if (!extensionsList.contains(*extensionPtr, Qt::CaseInsensitive)){
				isKnown = true;
			}
		}
	}

	if (isKnown){
		return tr("SVG image");
	}

	return "";
}


} // namespace iqtmm


