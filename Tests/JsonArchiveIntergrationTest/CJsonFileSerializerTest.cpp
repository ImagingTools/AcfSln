// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include "CJsonFileSerializerTest.h"


// Qt includes
#include <QtTest/QtTest>

// ACF includes
#include <istd/CChangeGroup.h>
#include <iprm/ISelectionParam.h>
#include <iprm/IIdParam.h>
#include <iprm/ITextParam.h>
#include <iprm/IOptionsList.h>
#include <iprm/IParamsSet.h>
#include <iprm/IParamsManager.h>
#include <ifile/IFilePersistence.h>
#include <itest/CStandardTestExecutor.h>
#include <imeas/INumericValue.h>


CJsonFileSerializerTest::CJsonFileSerializerTest()
	: QObject(nullptr),
	m_jsonFilePersistancePtr(nullptr),
	m_originalSerializableObjectPtr(nullptr),
	m_serializableObjectPtr(nullptr),
	m_jsonTempFilePtr(new QTemporaryFile)
{
}


void CJsonFileSerializerTest::initTestCase()
{
	m_testPartituraInstanceCompPtr.SetPtr(new CJsonArchiveIntergrationTest);
	QVERIFY2(m_testPartituraInstanceCompPtr.IsValid(), "Unable to initialize partitura component");

	ilog::IMessageConsumer* messageConsumerPtr = m_testPartituraInstanceCompPtr->GetInterface<ilog::IMessageConsumer>();
	if (messageConsumerPtr != nullptr){
		BaseClass2::SetLogPtr(messageConsumerPtr);
	}

	m_jsonFilePersistancePtr = m_testPartituraInstanceCompPtr->GetInterface<ifile::IFilePersistence>(QByteArrayLiteral("JsonFileSerializer"));
	QVERIFY2((m_jsonFilePersistancePtr != nullptr), "Unable to find JSON file persistance ('JsonFileSerializer')");

	m_originalSerializableObjectPtr = m_testPartituraInstanceCompPtr->GetInterface<iser::ISerializable>(QByteArrayLiteral("OriginalReceipt"));
	QVERIFY2((m_originalSerializableObjectPtr != nullptr), "Unable to find original object ('OriginalReceipt')");

	m_serializableObjectPtr = m_testPartituraInstanceCompPtr->GetInterface<iser::ISerializable>(QByteArrayLiteral("Receipt"));
	QVERIFY2((m_serializableObjectPtr != nullptr), "Unable to find object ('Receipt')");

	// Create file
	QVERIFY(m_jsonTempFilePtr->open());

	// close it
	m_jsonTempFilePtr->close();

	QVERIFY(QFile::exists(m_jsonTempFilePtr->fileName()));

	SendInfoMessage(0, QString("Created temp json file '%1'").arg(m_jsonTempFilePtr->fileName()), __func__);
}


void CJsonFileSerializerTest::JsonArchiveParamsSerializeTest()
{
	QVERIFY2((m_jsonFilePersistancePtr != nullptr), "Unable to find JSON file persistance ('JsonFileSerializer')");
	QVERIFY2((m_serializableObjectPtr != nullptr), "Unable to find object factory ('Receipt')");

	SendInfoMessage(1, "Creating model...", __func__);

	iprm::IParamsSet* paramsPtr = dynamic_cast<iprm::IParamsSet*>(m_originalSerializableObjectPtr);

	istd::CChangeGroup paramsChangeGroup(m_originalSerializableObjectPtr);
	// set type
	const QByteArray receiptTypeId = "Sell";
	iprm::ISelectionParam* receiptTypeSelectionParamPtr = dynamic_cast<iprm::ISelectionParam*>(paramsPtr->GetEditableParameter("Type"));
	QVERIFY(receiptTypeSelectionParamPtr != nullptr);

	const iprm::IOptionsList* receiptTypeSelectionConstraintsPtr = receiptTypeSelectionParamPtr->GetSelectionConstraints();
	QVERIFY(receiptTypeSelectionConstraintsPtr != nullptr);

	const int indexOfType = iprm::FindOptionIndexById(receiptTypeId, *receiptTypeSelectionConstraintsPtr);
	QVERIFY(indexOfType >= 0);
	receiptTypeSelectionParamPtr->SetSelectedOptionIndex(indexOfType);

	// set place
	const QString paymentPlace = "Shop";
	iprm::ITextParam* paymentsPlaceParamPtr = dynamic_cast<iprm::ITextParam*>(paramsPtr->GetEditableParameter("PaymentsPlace"));
	if (paymentsPlaceParamPtr != nullptr){
		paymentsPlaceParamPtr->SetText(paymentPlace);
	}

	// set machine number
	const QString machineNumber = "YRF4WYFHTURFIJ3EI";
	iprm::ITextParam* machineNumberParamPtr = dynamic_cast<iprm::ITextParam*>(paramsPtr->GetEditableParameter("MachineNumber"));
	if (machineNumberParamPtr != nullptr){
		machineNumberParamPtr->SetText(machineNumber);
	}

	// set payments
	iprm::IParamsManager* paymentsManagerParamsPtr = dynamic_cast<iprm::IParamsManager*>(paramsPtr->GetEditableParameter("Payments"));
	if (paymentsManagerParamsPtr != nullptr){
		// 1
		int insertedParamsIndex = paymentsManagerParamsPtr->InsertParamsSet();
		QVERIFY(insertedParamsIndex >= 0);

		iprm::IParamsSet* paymentParamsPtr = paymentsManagerParamsPtr->GetParamsSet(insertedParamsIndex);
		QVERIFY(paymentParamsPtr != nullptr);

		// type
		iprm::ISelectionParam* typeSelectionParamPtr = dynamic_cast<iprm::ISelectionParam*>(paymentParamsPtr->GetEditableParameter("Type"));
		QVERIFY(typeSelectionParamPtr != nullptr);

		const iprm::IOptionsList* typeSelectionConstraints = typeSelectionParamPtr->GetSelectionConstraints();
		QVERIFY(typeSelectionConstraints != nullptr);

		QByteArray paymentTypeId = "cash";
		int paymentIndexOfType = iprm::FindOptionIndexById(paymentTypeId, *typeSelectionConstraints);
		QVERIFY(paymentIndexOfType >= 0);
		typeSelectionParamPtr->SetSelectedOptionIndex(paymentIndexOfType);

		// sum
		imeas::INumericValue* sumNumericParamPtr = dynamic_cast<imeas::INumericValue*>(paymentParamsPtr->GetEditableParameter("Sum"));
		QVERIFY(sumNumericParamPtr != nullptr);

		int sum = 658374;
		imath::CVarVector sumList(1, sum);
		sumNumericParamPtr->SetValues(sumList);
	}

	// set operator
	iprm::IParamsSet* operatorParamsPtr = dynamic_cast<iprm::IParamsSet*>(paramsPtr->GetEditableParameter("Operator"));
	if (operatorParamsPtr != nullptr){
		iprm::IIdParam* idParamPtr = dynamic_cast<iprm::IIdParam*>(paramsPtr->GetEditableParameter("Id"));
		if (idParamPtr != nullptr){
			idParamPtr->SetId("DI483U5GTRJ");
		}

		SetTextForParamsSet(*operatorParamsPtr, "Name", "Karen");
		SetTextForParamsSet(*operatorParamsPtr, "TaxNumber", "425748563947");
	}

	// set client
	iprm::IParamsSet* clientParamsPtr = dynamic_cast<iprm::IParamsSet*>(paramsPtr->GetEditableParameter("Client"));
	if (clientParamsPtr != nullptr){
		SetTextForParamsSet(*clientParamsPtr, "Address", "96 Willow Vale");
		SetTextForParamsSet(*clientParamsPtr, "EMail", "User@Company.com");
		SetTextForParamsSet(*clientParamsPtr, "PhoneNumber", "555-342578345-2345");
		SetTextForParamsSet(*clientParamsPtr, "Name", "Sam Smith");
		SetTextForParamsSet(*clientParamsPtr, "BirthDate", "2024-09-19");
		SetTextForParamsSet(*clientParamsPtr, "Citizenship", "London");
		SetTextForParamsSet(*clientParamsPtr, "IdentityDocumentCode", "43T8IUGJIRTFS");
		SetTextForParamsSet(*clientParamsPtr, "IdentityDocumentData", "UFGJV54U7GHYVTRU");

		iprm::IIdParam* idParamPtr = dynamic_cast<iprm::IIdParam*>(paramsPtr->GetEditableParameter("Id"));
		if (idParamPtr != nullptr){
			idParamPtr->SetId("SK329R8FUGJRE");
		}
	}

	iprm::IParamsManager* itemsParamsManagerPtr = dynamic_cast<iprm::IParamsManager*>(paramsPtr->GetEditableParameter("Items"));
	if ( itemsParamsManagerPtr != nullptr){
		int insertedParamsIndex = itemsParamsManagerPtr->InsertParamsSet();
		QVERIFY(insertedParamsIndex >= 0);

		iprm::IParamsSet* paymentItemParamsSetPtr = itemsParamsManagerPtr->GetParamsSet(insertedParamsIndex);
		QVERIFY(paymentItemParamsSetPtr != nullptr);

		SetTextForParamsSet(*paymentItemParamsSetPtr, "Name", "Cookies");
		SetDigitValueForParamsSet(*paymentItemParamsSetPtr, "Price", 748);
		SetDigitValueForParamsSet(*paymentItemParamsSetPtr, "Amount", 374);
		SetDigitValueForParamsSet(*paymentItemParamsSetPtr, "Quantity", 500);
		SelectOptionByIdForParamsSet(*paymentItemParamsSetPtr, "ItemType", QByteArray("commodity"));
		SelectOptionByIdForParamsSet(*paymentItemParamsSetPtr, "PaymentMethod", QByteArray("fullPrepayment"));
		SelectOptionByIdForParamsSet(*paymentItemParamsSetPtr, "MeasurementUnit", QByteArray("gram"));
	}

	SendInfoMessage(1, QString("Saving to file '%1'...").arg(m_jsonTempFilePtr->fileName()), __func__);

	const int saveStatus = m_jsonFilePersistancePtr->SaveToFile(*paramsPtr, m_jsonTempFilePtr->fileName());
	QVERIFY2(saveStatus == ifile::IFilePersistence::OS_OK, "Unable to save file");

	// close file if it is open, to read file from begin
	if (!m_jsonTempFilePtr->isOpen()){
		m_jsonTempFilePtr->close();
	}
	m_jsonTempFilePtr->open();
	const QByteArray fileData = m_jsonTempFilePtr->readAll();
	m_jsonTempFilePtr->close();

	QJsonParseError jsonParseError;
	QJsonObject savedJsonObject = QJsonDocument::fromJson(fileData, &jsonParseError).object();

	QVERIFY2(jsonParseError.error == QJsonParseError::NoError,
			 QString("Saved JSON is NOT valid. Error: '%1' at '%2'. \n DATA: \n %3").arg(jsonParseError.errorString(), QString::number(jsonParseError.offset), qPrintable(fileData)).toLocal8Bit());
}


void CJsonFileSerializerTest::JsonArchiveParamsDeserializeTest()
{
	SendInfoMessage(2, QString("Loading from file'%1' ...").arg(m_jsonTempFilePtr->fileName()), __func__);

	const int loadedStatus = m_jsonFilePersistancePtr->LoadFromFile(*m_serializableObjectPtr, m_jsonTempFilePtr->fileName());
	QVERIFY2(loadedStatus == ifile::IFilePersistence::OS_OK, "Unable to load file");
}


void CJsonFileSerializerTest::JsonArchiveParamsCompareTest()
{
	SendInfoMessage(3, "Comparing objects...", __func__);

	const bool isEqual = m_serializableObjectPtr->IsEqual(*m_originalSerializableObjectPtr);
	QVERIFY2(isEqual, "The serialized data does not match the original data.");
}


void CJsonFileSerializerTest::cleanupTestCase()
{
	BaseClass2::SetLogPtr(nullptr);
	m_jsonFilePersistancePtr = nullptr;
	m_serializableObjectPtr = nullptr;
	m_jsonTempFilePtr.reset();
	m_testPartituraInstanceCompPtr.Reset();
}


// private methods


// comfort methods

bool CJsonFileSerializerTest::SetTextForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, const QString& text)
{
	iprm::ITextParam* textParamPtr = dynamic_cast<iprm::ITextParam*>(params.GetEditableParameter(paramId));
	if (textParamPtr == nullptr){
		qWarning() << "Unable to find text param with id: " << paramId;

		return false;
	}

	textParamPtr->SetText(text);

	return true;
}


bool CJsonFileSerializerTest::SetDigitValueForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, int value)
{
	imeas::INumericValue* sumNumericParamPtr = dynamic_cast<imeas::INumericValue*>(params.GetEditableParameter(paramId));
	if (sumNumericParamPtr == nullptr){
		qWarning() << "Unable to find numeric param with id: " << paramId;

		return false;
	}
	imath::CVarVector valueList(1, value);
	sumNumericParamPtr->SetValues(valueList);

	return true;
}


bool CJsonFileSerializerTest::SetDigitValueForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, double value)
{
	imeas::INumericValue* sumNumericParamPtr = dynamic_cast<imeas::INumericValue*>(params.GetEditableParameter(paramId));
	if (sumNumericParamPtr == nullptr){
		qWarning() << "Unable to find numeric param with id: " << paramId;

		return false;
	}
	imath::CVarVector valueList(1, value);
	sumNumericParamPtr->SetValues(valueList);

	return true;
}


bool CJsonFileSerializerTest::SelectOptionByIdForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, const QByteArray& selectionId)
{
	iprm::ISelectionParam* selectionParamPtr = dynamic_cast<iprm::ISelectionParam*>(params.GetEditableParameter(paramId));
	if (selectionParamPtr == nullptr){
		qWarning() << "Unable to find selection param with id: " << paramId;

		return false;
	}

	const iprm::IOptionsList* selectionConstraintsPtr = selectionParamPtr->GetSelectionConstraints();
	if (selectionConstraintsPtr == nullptr){
		qWarning() << "Selection param with id: " << paramId << "hasn't constraints";

		return false;
	}

	const int indexOfId = iprm::FindOptionIndexById(selectionId, *selectionConstraintsPtr);
	if (indexOfId < 0){
		qWarning() << "Selection param with id: " << paramId << "hasn't option with id: " << selectionId;

		return false;
	}
	selectionParamPtr->SetSelectedOptionIndex(indexOfId);

	return true;
}


bool CJsonFileSerializerTest::SelectOptionByIdForParamsSet(iprm::IParamsSet& params, const QByteArray& paramId, const QString& selectionId)
{
	return SelectOptionByIdForParamsSet(params, paramId, selectionId.toUtf8());
}


I_ADD_TEST(CJsonFileSerializerTest)
