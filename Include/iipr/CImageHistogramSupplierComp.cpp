#include <iipr/CImageHistogramSupplierComp.h>


// ACF includes
#include <ilog/CMessage.h>


namespace iipr
{


// public methods

// reimplemented (imeas::IDataSequenceProvider)

const imeas::IDataSequence* CImageHistogramSupplierComp::GetDataSequence() const
{
	return GetWorkProduct();
}


// protected methods

// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CImageHistogramSupplierComp::ProduceObject(imod::TModelWrap<imeas::CSimpleSamplesSequence32>& result) const
{
	if (		m_bitmapProviderCompPtr.IsValid() &&
				m_histogramProcessorCompPtr.IsValid()){
		const iimg::IBitmap* bitmapPtr = m_bitmapProviderCompPtr->GetBitmap();
		if (bitmapPtr == nullptr){
			AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("No input image"), "ImageHistogram"));

			return WS_FAILED;
		}

		Timer performanceTimer(this, "Histogram calculation");
		Q_UNUSED(performanceTimer);

		int processingState = m_histogramProcessorCompPtr->DoProcessing(
						GetModelParametersSet(),
						bitmapPtr,
						&result);

		if (processingState != iproc::IProcessor::TS_OK){
			return WS_FAILED;
		}

		return WS_OK;
	}

	return WS_FAILED;
}


// reimplemented (icomp::CComponentBase)

void CImageHistogramSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr(), m_bitmapSupplierCompPtr.GetPtr());
	}
}


void CImageHistogramSupplierComp::OnComponentDestroyed()
{
	BaseClass::OnComponentDestroyed();

	if (m_bitmapProviderModelCompPtr.IsValid()){
		UnregisterSupplierInput(m_bitmapProviderModelCompPtr.GetPtr());
	}
}


} // namespace iipr


