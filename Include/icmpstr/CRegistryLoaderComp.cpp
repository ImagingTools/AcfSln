// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icmpstr/CRegistryLoaderComp.h>


// Qt includes
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>


// ACF includes
#include <istd/CChangeNotifier.h>
#include <ifile/TFileSerializerComp.h>
#include <ifile/CSimpleXmlFileReadArchive.h>
#include <ifile/CSimpleXmlFileWriteArchive.h>
#include <ifile/CCompactXmlFileReadArchive.h>
#include <ifile/CCompactXmlFileWriteArchive.h>

// ACF-Solutions includes
#include <icmpstr/CVisualRegistry.h>


namespace icmpstr
{


// reimplemented (ifile::IFilePersistence)

bool CRegistryLoaderComp::IsOperationSupported(
			const istd::IChangeable* dataObjectPtr,
			const QString* filePathPtr,
			int flags,
			bool beQuiet) const
{
	const icomp::CRegistry* serializablePtr = CompCastPtr<icomp::CRegistry>(dataObjectPtr);
	if ((dataObjectPtr != NULL) && (serializablePtr == NULL)){
		if (!beQuiet){
			SendInfoMessage(MI_BAD_OBJECT_TYPE, QObject::tr("Only ACF registries can be serialized"));
		}

		return false;
	}

	if ((flags & (QF_FILE)) == 0){
		return false;
	}

	if (filePathPtr != NULL){
		QFileInfo fileInfo(*filePathPtr);

		if ((flags & (QF_LOAD | QF_SAVE)) == QF_LOAD){
			if (!fileInfo.exists()){
				if (!beQuiet){
					SendWarningMessage(MI_FILE_NOT_EXIST, QString("File %1 does not exist").arg(*filePathPtr));
				}

				return false;
			}
		}

		if ((flags & QF_SAVE) != 0){
			QDir directory = fileInfo.dir();
			if (!directory.exists()){
				if (!beQuiet){
					SendWarningMessage(MI_FILE_NOT_EXIST, QString("Save target directory %1 does not exist").arg(directory.canonicalPath()));
				}

				return false;
			}
		}

		if (		(!*m_supportOldFormatAttrPtr || (fileInfo.suffix().compare("arx", Qt::CaseInsensitive) != 0)) &&
					(fileInfo.suffix().compare("acc", Qt::CaseInsensitive) != 0)){
			if (!beQuiet){
				SendInfoMessage(MI_BAD_EXTENSION, QObject::tr("File extension is not supported"));
			}

			return false;
		}
	}

	return true;
}


ifile::IFilePersistence::OperationState CRegistryLoaderComp::LoadFromFile(
			istd::IChangeable& data,
			const QString& filePath,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	istd::CChangeNotifier notifier(&data);

	if (!IsOperationSupported(&data, &filePath, QF_LOAD | QF_FILE, false)){
		return OS_FAILED;
	}

	icomp::CRegistry* registryPtr = dynamic_cast<icomp::CRegistry*>(&data);
	CVisualRegistry* geometricalRegistryPtr = dynamic_cast<CVisualRegistry*>(registryPtr);

	if (registryPtr != NULL){
		if (QFileInfo(filePath).suffix().compare("acc", Qt::CaseInsensitive) == 0){
			ifile::TFileSerializerComp<ifile::CCompactXmlFileReadArchive, ifile::CCompactXmlFileWriteArchive>::ReadArchiveEx registryArchive(filePath, this);
			Q_ASSERT(!registryArchive.IsStoring());

			if (geometricalRegistryPtr != NULL){
				if (!geometricalRegistryPtr->SerializeRegistry(registryArchive)){
					registryArchive.SendLogMessage(istd::IInformationProvider::IC_ERROR, MI_CANNOT_LOAD, "Cannot load registry file", "RegistryLoader");

					return OS_FAILED;
				}

				ifile::TFileSerializerComp<ifile::CCompactXmlFileReadArchive, ifile::CCompactXmlFileWriteArchive>::ReadArchiveEx layoutArchive(GetLayoutPath(filePath, false), this);
				Q_ASSERT(!layoutArchive.IsStoring());

				if (!geometricalRegistryPtr->SerializeUserData(layoutArchive)){
					layoutArchive.SendLogMessage(istd::IInformationProvider::IC_INFO, MI_CANNOT_LOAD, "Layout information cannot be loaded", "RegistryLoader");
				}

				return OS_OK;
			}
			else{
				if (!registryPtr->Serialize(registryArchive)){
					registryArchive.SendLogMessage(istd::IInformationProvider::IC_ERROR, MI_CANNOT_LOAD, "Cannot load registry file", "RegistryLoader");

					return OS_FAILED;
				}

				return OS_OK;
			}
		}
		else if (*m_supportOldFormatAttrPtr && (QFileInfo(filePath).suffix().compare("arx", Qt::CaseInsensitive) == 0)){
			ifile::TFileSerializerComp<ifile::CSimpleXmlFileReadArchive, ifile::CSimpleXmlFileWriteArchive>::ReadArchiveEx registryArchive(filePath, this);
			Q_ASSERT(!registryArchive.IsStoring());

			if (geometricalRegistryPtr != NULL){
				if (!geometricalRegistryPtr->SerializeRegistry(registryArchive)){
					registryArchive.SendLogMessage(istd::IInformationProvider::IC_ERROR, MI_CANNOT_LOAD, "Cannot load registry file", "RegistryLoader");

					return OS_FAILED;
				}

				ifile::TFileSerializerComp<ifile::CSimpleXmlFileReadArchive, ifile::CSimpleXmlFileWriteArchive>::ReadArchiveEx layoutArchive(GetLayoutPath(filePath, true), this);
				Q_ASSERT(!layoutArchive.IsStoring());

				if (!geometricalRegistryPtr->SerializeUserData(layoutArchive)){
					layoutArchive.SendLogMessage(istd::IInformationProvider::IC_INFO, MI_CANNOT_LOAD, "Layout information cannot be loaded", "RegistryLoader");
				}

				return OS_OK;
			}
			else{
				if (!registryPtr->Serialize(registryArchive)){
					registryArchive.SendLogMessage(istd::IInformationProvider::IC_ERROR, MI_CANNOT_LOAD, "Cannot load registry file", "RegistryLoader");

					return OS_FAILED;
				}

				return OS_OK;
			}
		}
	}

	return OS_FAILED;
}


ifile::IFilePersistence::OperationState CRegistryLoaderComp::SaveToFile(
			const istd::IChangeable& data,
			const QString& filePath,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	if (!IsOperationSupported(&data, &filePath, QF_SAVE | QF_FILE, false)){
		return OS_FAILED;
	}

	const icomp::CRegistry* registryPtr = dynamic_cast<const icomp::CRegistry*>(&data);
	const CVisualRegistry* geometricalRegistryPtr = dynamic_cast<const CVisualRegistry*>(registryPtr);

	if (registryPtr != NULL){
		if (QFileInfo(filePath).suffix().compare("arx", Qt::CaseInsensitive) == 0){
			ifile::TFileSerializerComp<ifile::CSimpleXmlFileReadArchive, ifile::CSimpleXmlFileWriteArchive>::WriteArchiveEx registryArchive(filePath, m_versionInfoCompPtr.GetPtr(), this);
			Q_ASSERT(registryArchive.IsStoring());

			if (geometricalRegistryPtr != NULL){
				if (!const_cast<CVisualRegistry*>(geometricalRegistryPtr)->SerializeRegistry(registryArchive)){
					registryArchive.SendLogMessage(istd::IInformationProvider::IC_ERROR, MI_CANNOT_LOAD, "Cannot store registry to file", "RegistryLoader");

					return OS_FAILED;
				}

				QString layoutFilePath = GetLayoutPath(filePath, true);
				QFile(layoutFilePath).setPermissions(QFile::WriteUser);

				ifile::TFileSerializerComp<ifile::CSimpleXmlFileReadArchive, ifile::CSimpleXmlFileWriteArchive>::WriteArchiveEx layoutArchive(layoutFilePath, m_versionInfoCompPtr.GetPtr(), this);
				Q_ASSERT(layoutArchive.IsStoring());

				if (!const_cast<CVisualRegistry*>(geometricalRegistryPtr)->SerializeUserData(layoutArchive)){
					layoutArchive.SendLogMessage(istd::IInformationProvider::IC_INFO, MI_CANNOT_LOAD, "Layout information cannot be stored", "RegistryLoader");
				}

				return OS_OK;
			}
			else{
				if (!const_cast<icomp::CRegistry*>(registryPtr)->Serialize(registryArchive)){
					registryArchive.SendLogMessage(istd::IInformationProvider::IC_ERROR, MI_CANNOT_LOAD, "Cannot store registry to file", "RegistryLoader");

					return OS_FAILED;
				}

				return OS_OK;
			}
		}
		else{
			ifile::TFileSerializerComp<ifile::CCompactXmlFileReadArchive, ifile::CCompactXmlFileWriteArchive>::WriteArchiveEx registryArchive(filePath, m_versionInfoCompPtr.GetPtr(), this);
			Q_ASSERT(registryArchive.IsStoring());

			if (geometricalRegistryPtr != NULL){
				if (!const_cast<CVisualRegistry*>(geometricalRegistryPtr)->SerializeRegistry(registryArchive)){
					registryArchive.SendLogMessage(istd::IInformationProvider::IC_ERROR, MI_CANNOT_LOAD, "Cannot store registry to file", "RegistryLoader");

					return OS_FAILED;
				}

				QString layoutFilePath = GetLayoutPath(filePath, false);
				QFile(layoutFilePath).setPermissions(QFile::WriteUser);

#if defined (Q_OS_UNIX)
				// for *NIX systems it is required set read permission explicitly for correct reading
				QFileInfo layoutFileInfo(layoutFilePath);
				QFile::Permissions layoutFilePermissions = layoutFileInfo.permissions();
				layoutFilePermissions.setFlag(QFile::ReadUser);
				QFile(layoutFilePath).setPermissions(layoutFilePermissions);
#endif

				ifile::TFileSerializerComp<ifile::CCompactXmlFileReadArchive, ifile::CCompactXmlFileWriteArchive>::WriteArchiveEx layoutArchive(layoutFilePath, m_versionInfoCompPtr.GetPtr(), this);
				Q_ASSERT(layoutArchive.IsStoring());

				if (!const_cast<CVisualRegistry*>(geometricalRegistryPtr)->SerializeUserData(layoutArchive)){
					layoutArchive.SendLogMessage(istd::IInformationProvider::IC_INFO, MI_CANNOT_LOAD, "Layout information cannot be stored", "RegistryLoader");
				}

				return OS_OK;
			}
			else{
				if (!const_cast<icomp::CRegistry*>(registryPtr)->Serialize(registryArchive)){
					registryArchive.SendLogMessage(istd::IInformationProvider::IC_ERROR, MI_CANNOT_LOAD, "Cannot store registry to file", "RegistryLoader");

					return OS_FAILED;
				}

				return OS_OK;
			}
		}
	}

	return OS_FAILED;
}


// reimplemented (ifile::IFileTypeInfo)

bool CRegistryLoaderComp::GetFileExtensions(QStringList& result, const istd::IChangeable* /*dataObjectPtr*/, int /*flags*/, bool doAppend) const
{
	if (!doAppend){
		result.clear();
	}

	result.push_back("acc");
	if (*m_supportOldFormatAttrPtr){
		result.push_back("arx");
	}

	return true;
}


QString CRegistryLoaderComp::GetTypeDescription(const QString* extensionPtr) const
{
	if ((extensionPtr == NULL) || (extensionPtr->compare("acc", Qt::CaseInsensitive) == 0)){
		return tr("ACF composed component");
	}
	else if (*m_supportOldFormatAttrPtr && extensionPtr->compare("arx", Qt::CaseInsensitive) == 0){
		return tr("Old ACF registry file");
	}

	return "";
}


// protected methods

QString CRegistryLoaderComp::GetLayoutPath(const QString& registryPath, bool oldFormat) const
{
	QFileInfo fileInfo(registryPath);
	if (oldFormat){
		return fileInfo.dir().absoluteFilePath(fileInfo.completeBaseName() + ".alx");
	}
	else{
		return fileInfo.dir().absoluteFilePath(fileInfo.completeBaseName() + ".accl");
	}
}


} // namespace icmpstr


