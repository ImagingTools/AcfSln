// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iqtex/CXslSerializerComp.h>


// Qt includes
#include <QtCore/QObject>

// ACF includes
#include <istd/CChangeNotifier.h>

// ACF-Solutions includes
#include <iqtex/CXslTransformationReadArchive.h>
#include <iqtex/CXslTransformationWriteArchive.h>


namespace iqtex
{


bool CXslSerializerComp::IsOperationSupported(
			const istd::IChangeable* dataObjectPtr,
			const QString* filePathPtr,
			int flags,
			bool beQuiet) const
{
#if QT_VERSION < 0x060000
	if (((flags & (QF_LOAD | QF_SAVE)) == QF_LOAD) && !m_xslReadFilePath.IsValid()){
		return false;
	}

	if (((flags & (QF_LOAD | QF_SAVE)) == QF_SAVE) && !m_xslWriteFilePath.IsValid()){
		return false;
	}

	return BaseClass::IsOperationSupported(dataObjectPtr, filePathPtr, flags, beQuiet);
#else
	return false;
#endif
}


ifile::IFilePersistence::OperationState CXslSerializerComp::LoadFromFile(
			istd::IChangeable& data,
			const QString& filePath,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
#if QT_VERSION < 0x060000
	if (IsOperationSupported(&data, &filePath, QF_LOAD | QF_FILE, false)){
		CXslTransformationReadArchive archive(filePath, m_xslReadFilePath->GetPath());

		Q_ASSERT(!archive.IsStoring());

		iser::ISerializable* serializablePtr = CompCastPtr<iser::ISerializable>(&data);
		Q_ASSERT(serializablePtr != NULL);

		if (serializablePtr->Serialize(archive)){
			return OS_OK;
		}
		else{
			OnReadError(archive, data, filePath);
		}
	}
#endif
	return OS_FAILED;
}


ifile::IFilePersistence::OperationState CXslSerializerComp::SaveToFile(
			const istd::IChangeable& data,
			const QString& filePath,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
#if QT_VERSION < 0x060000
	if (IsOperationSupported(&data, &filePath, QF_SAVE | QF_FILE, false)){
		CXslTransformationWriteArchive archive(filePath, m_xslWriteFilePath->GetPath(), GetVersionInfo(), true);
		Q_ASSERT(archive.IsStoring());

		const iser::ISerializable* serializablePtr = CompCastPtr<iser::ISerializable>(&data);
		Q_ASSERT(serializablePtr != NULL);

		if (!CheckMinimalVersion(*serializablePtr, archive.GetVersionInfo())){
			SendWarningMessage(MI_UNSUPPORTED_VERSION, QT_TR_NOOP("Archive version is not supported, possible lost of data"));
		}

		if ((const_cast<iser::ISerializable*>(serializablePtr))->Serialize(archive)){
			return OS_OK;
		}
		else{
			SendInfoMessage(MI_CANNOT_SAVE, QT_TR_NOOP("Cannot serialize object to file"));
		}
	}
#endif
	return OS_FAILED;
}


// protected methods


void CXslSerializerComp::OnReadError(const iser::IArchive& /*archive*/, const istd::IChangeable& /*data*/, const QString& filePath) const
{
	SendWarningMessage(MI_CANNOT_LOAD, QT_TR_NOOP("Cannot load object from file ") + filePath);
}


} // namespace iqtex


