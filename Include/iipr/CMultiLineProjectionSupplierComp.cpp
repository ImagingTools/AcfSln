#include <iipr/CMultiLineProjectionSupplierComp.h>


// ACF includes
#include <ilog/IMessageConsumer.h>
#include <ilog/CExtMessage.h>


namespace iipr
{


// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CMultiLineProjectionSupplierComp::ProduceObject(ProductType& result) const
{
	iinsp::ISupplier::WorkStatus retVal = WS_OK;

	if (m_bitmapProviderCompPtr.IsValid() &&
		m_linesProviderCompPtr.IsValid()){
		const iimg::IBitmap* bitmapPtr = m_bitmapProviderCompPtr->GetBitmap();
		if (bitmapPtr == nullptr){
			AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("No input image"), "MultiLineProjection"));

			return WS_FAILED;
		}

		m_linesSupplierCompPtr->InvalidateSupplier();
		m_linesSupplierCompPtr->EnsureWorkInitialized();
		m_linesSupplierCompPtr->EnsureWorkFinished();

		int projectionCount = m_linesProviderCompPtr->GetFeaturesCount();
		if (projectionCount <= 0){
			AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("No projection lines found"), "MultiLineProjection"));

			return WS_FAILED;
		}

		result.resize(projectionCount);

		Timer performanceTimer(this, "Extraction of projections");

		for (int i = 0; i < projectionCount; i++){
			imeas::CGeneralDataSequence& lineResult = result[i];

			imath::CVarVector varVector = m_linesProviderCompPtr->GetFeature(i).GetComponentValue(imeas::INumericValue::VTI_2D_LINE);
			if (varVector.GetElementsCount() < 4){
				AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("No line at projection %1 found").arg(i + 1), "MultiLineProjection"));

				return WS_FAILED;
			}

			i2d::CLine2d line(
				varVector.GetElement(0),
				varVector.GetElement(1),
				varVector.GetElement(2),
				varVector.GetElement(3));

			bool isOk = m_projectionProcessorCompPtr->DoProjection(*bitmapPtr, line, nullptr, lineResult);
			if (!isOk){
				AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("Calculation of projection %1 failed").arg(i + 1), "MultiLineProjection"));

				retVal = WS_FAILED;
			}

			ilog::CExtMessage* pointMessagePtr = new ilog::CExtMessage(
						isOk ? istd::IInformationProvider::IC_INFO : istd::IInformationProvider::IC_ERROR,
						iinsp::CSupplierCompBase::MI_INTERMEDIATE,
						QString("Line %1").arg(i),
						"Projection generator");
			i2d::CLine2d* pointMessageObjectPtr = new imod::TModelWrap<i2d::CLine2d>();
			pointMessageObjectPtr->SetPoint1(line.GetPoint1());
			pointMessageObjectPtr->SetPoint2(line.GetPoint2());
			pointMessagePtr->InsertAttachedObject(pointMessageObjectPtr);

			AddMessage(pointMessagePtr, MCT_TEMP);
		}

		return retVal;
	}

	SendCriticalMessage(0, "Bad component archtecture. Bitmap provider or lines provider were not set");

	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CMultiLineProjectionSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr(), m_bitmapSupplierCompPtr.GetPtr());
	}

	if (m_linesProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_linesProviderModelCompPtr.GetPtr(), m_linesSupplierCompPtr.GetPtr());
	}
}


// reimplemented (imeas::IMultiDataSequenceProvider)

int CMultiLineProjectionSupplierComp::GetSequencesCount() const
{
	const ProductType* resultPtr = GetWorkProduct();
	if (resultPtr != nullptr){
		return resultPtr->count();
	}

	return 0;
}


const imeas::IDataSequence* CMultiLineProjectionSupplierComp::GetDataSequence(int index) const
{
	const ProductType* resultPtr = GetWorkProduct();
	if (resultPtr != nullptr){
		if (index >= 0 && index < resultPtr->count()){
			return &resultPtr->at(index);
		}
	}

	return nullptr;
}


// reimplemented (imeas::IDataSequenceProvider)

const imeas::IDataSequence* CMultiLineProjectionSupplierComp::GetDataSequence() const
{
	const ProductType* resultPtr = GetWorkProduct();
	if (resultPtr != nullptr){
		imeas::CGeneralDataSequence* averageDataSequence = new imeas::CGeneralDataSequence;

		int sequenceCount = resultPtr->count();
		int endChannelsCount = 0;
		for (int sequenceIndex = 0; sequenceIndex < sequenceCount; ++sequenceIndex){
			const imeas::CGeneralDataSequence singleSequence = resultPtr->at(sequenceIndex);
			int channelsCount = singleSequence.GetChannelsCount();
			if (endChannelsCount < channelsCount){
				endChannelsCount = channelsCount;
			}
		}
		averageDataSequence->CreateSequence(sequenceCount, endChannelsCount);
		for (int sequenceIndex = 0; sequenceIndex < sequenceCount; ++sequenceIndex){
			const imeas::CGeneralDataSequence singleSequence = resultPtr->at(sequenceIndex);
			int channelsCount = singleSequence.GetChannelsCount();
			int samplesCount = singleSequence.GetSamplesCount();
			for (int channelsIndex = 0; channelsIndex < channelsCount; ++channelsIndex){
				double samplesSum = 0;
				for (int samplesIndex = 0; samplesIndex < samplesCount; ++samplesIndex){
					samplesSum += singleSequence.GetSample(samplesIndex, channelsIndex);
				}
				double average = samplesSum / samplesCount;
				averageDataSequence->SetSample(sequenceIndex, channelsIndex, average);
			}
		}
		return averageDataSequence;
	}
	return nullptr;
}


} // namespace iipr


