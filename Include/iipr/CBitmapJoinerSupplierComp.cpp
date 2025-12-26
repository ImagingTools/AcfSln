#include <iipr/CBitmapJoinerSupplierComp.h>


#include <iipr/CBitmapOperations.h>


namespace iipr
{


// reimplemented (iimg::IBitmapProvider)

const iimg::IBitmap* CBitmapJoinerSupplierComp::GetBitmap() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr){
		return productPtr->second.GetPtr();
	}

	return nullptr;
}


// reimplemented (i2d::ICalibrationProvider)

const i2d::ITransformation2d* CBitmapJoinerSupplierComp::GetLogTransform() const
{
	const ProductType* productPtr = GetWorkProduct();
	if (productPtr != nullptr){
		return productPtr->first;
	}
	else{
		return nullptr;
	}
}

// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CBitmapJoinerSupplierComp::ProduceObject(ProductType& result) const
{
	if (!m_bitmapsProviderCompPtr.IsValid()){
		return WS_FAILED;
	}

	if (!EnsureBitmapCreated(result)){
		return WS_FAILED;
	}
	
	Q_ASSERT(result.second.IsValid());

	result.first = nullptr;

	int inputBitmapsCount = m_bitmapsProviderCompPtr->GetBitmapsCount();

	if (inputBitmapsCount <= 0){
		return WS_FAILED;
	}

	QVector<const iimg::IBitmap*> inputBitmaps;

	for (int inputBitmapIndex = 0; inputBitmapIndex < inputBitmapsCount; inputBitmapIndex++){
			const iimg::IBitmap* bitmapPtr = m_bitmapsProviderCompPtr->GetBitmap(inputBitmapIndex);
		if (bitmapPtr == nullptr){
			SendErrorMessage(0, "Input image could not be provided");

			return WS_FAILED;
		}
	
		inputBitmaps.push_back(bitmapPtr);
	}

	CBitmapOperations::JoinMode joinMode = *m_useHorizontalJoinAttrPtr ? CBitmapOperations::JM_HORIZONTAL : CBitmapOperations::JM_VERTICAL;

	return CBitmapOperations::JoinBitmaps(
				inputBitmaps,
				joinMode,
				*result.second.GetPtr(),
				const_cast<CBitmapJoinerSupplierComp*>(this)) ? WS_OK : WS_FAILED;
}


bool CBitmapJoinerSupplierComp::EnsureBitmapCreated(ProductType& result) const
{
	if (!m_bitmapCompFact.IsValid()){
		return false;
	}

	if (!result.second.IsValid()){
		result.second.FromUnique(m_bitmapCompFact.CreateInstance());
	}

	return result.second.IsValid();
}


// reimplemented (icomp::CComponentBase)

void CBitmapJoinerSupplierComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	if (m_bitmapsProviderModelCompPtr.IsValid()){
		RegisterSupplierInput(m_bitmapsProviderModelCompPtr.GetPtr(), m_bitmapsSupplierCompPtr.GetPtr());
	}
}


} // namespace iipr


