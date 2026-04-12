// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// Qt includes
#include <QtCore/QObject>
#include <QtCore/QTemporaryFile>
#include <QtCore/QScopedPointer>

// ACF includes
#include <istd/TIFactory.h>
#include <istd/TDelPtr.h>
#include <ilog/CLoggerBase.h>


#include <GeneratedFiles/JsonArchiveIntergrationTest/CJsonArchiveIntergrationTest.h>

// forward declaration
namespace iprm
{
class IParamsSet;
} // namespace iprm

namespace iser
{
class ISerializable;
} // namespace iser

namespace ifile
{
class IFilePersistence;
} // namespace ifile



class CJsonFileSerializerTest: public QObject, public ilog::CLoggerBase
{
	Q_OBJECT
public:
	typedef QObject BaseClass;
	typedef ilog::CLoggerBase BaseClass2;
	CJsonFileSerializerTest();

private Q_SLOTS:
	void initTestCase();

	void JsonArchiveParamsSerializeTest();
	void JsonArchiveParamsDeserializeTest();
	void JsonArchiveParamsCompareTest();

	void cleanupTestCase();

private:
	// comfort methods
	static bool SetTextForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, const QString& text);
	static bool SetDigitValueForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, int value);
	static bool SetDigitValueForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, double value);
	static bool SelectOptionByIdForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, const QByteArray& selectionId);
	static bool SelectOptionByIdForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, const QString& selectionId);

private:
	istd::TDelPtr<CJsonArchiveIntergrationTest> m_testPartituraInstanceCompPtr;
	ifile::IFilePersistence* m_jsonFilePersistancePtr;
	iser::ISerializable* m_originalSerializableObjectPtr;
	iser::ISerializable* m_serializableObjectPtr;
	QScopedPointer<QTemporaryFile> m_jsonTempFilePtr;

};


