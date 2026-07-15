// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CBitmapJoinerCompBase.h>


namespace icam
{


// protected members

bool CBitmapJoinerCompBase::PrepareSnapResult(int imageCount, istd::IChangeable* outputPtr)
{
	m_resultPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (imageCount <= 0 || m_resultPtr == NULL){
		return false;
	}

	m_snapList.clear();
	m_snapList.resize(imageCount);

	for(int listIndex = 0; listIndex < m_snapList.size(); listIndex++){
		iimg::IBitmapUniquePtr bitmapInstancePtr = m_bitmapFactoryPtr.CreateInstance();

		m_snapList[listIndex].bitmapPtr.FromUnique(std::move(bitmapInstancePtr));
	}

	return true;
}


bool CBitmapJoinerCompBase::FinalizeSnapResult()
{
	int height = 0, rowHeight = 0;
	int width = 0, columnWidth = 0;

	if (m_snapList.count() < 1){
		return true;
	}

	iimg::IBitmap::PixelFormat outputPixelFormat = m_snapList[0].bitmapPtr->GetPixelFormat();

	for(int listIndex = 1; listIndex < m_snapList.size(); listIndex++){
		if (outputPixelFormat != m_snapList[listIndex].bitmapPtr->GetPixelFormat()){
			SendErrorMessage(0, "Pixel format mismatch", "Bitmap joiner");

			return false;
		}
	}

	int columnCount = *m_numberOfColumnsAttrPtr > 0 ? *m_numberOfColumnsAttrPtr : m_snapList.size();
	int rowCount = m_snapList.size() / columnCount + int(m_snapList.size() % columnCount > 0);

	// find the biggest image
	for (int i = 0; i < m_snapList.count(); i++){
		if (m_snapList.at(i).isSnapOk){
			istd::CIndex2d imageSize = m_snapList.at(i).bitmapPtr.GetPtr()->GetImageSize();
			columnWidth = qMax(columnWidth, imageSize.GetX());
			rowHeight = qMax(rowHeight, imageSize.GetY());
		}
	}

	// make joined image
	width = columnWidth * columnCount;
	height = rowHeight * rowCount;

	if (!m_resultPtr->CreateBitmap(outputPixelFormat, istd::CIndex2d(width, height))){
		SendErrorMessage(0, "Resulting bitmap could not be created", "Bitmap joiner");

		return false;
	}

	int index = 0;
	int outputLineBytes = m_resultPtr->GetLineBytesCount();

	// vertical joining
	for (int row = 0; row < rowCount; row++){
		// horizontal joining
		for (int col = 0; col < columnCount; col++){
			if (m_snapList.at(index).isSnapOk){
				const iimg::IBitmap* inputSingleBitmap = m_snapList.at(index).bitmapPtr.GetPtr();
				if (inputSingleBitmap != NULL){
					int singleBitmapHeight = inputSingleBitmap->GetImageSize().GetY();
					int lineBytes = inputSingleBitmap->GetLineBytesCount();

					quint8* outputLinePtr = (quint8*)(m_resultPtr->GetLinePtr(row * rowHeight)) + col * columnWidth;
					for (int y = 0; y < singleBitmapHeight; y++, outputLinePtr += outputLineBytes){
						const void* inputLinePtr = inputSingleBitmap->GetLinePtr(y);

						memcpy((void*)outputLinePtr, inputLinePtr, lineBytes);
					}
				}
			}

			// ++ and break
			if (++index == m_snapList.count()){
				return true;
			}
		}
	}

	return true;
}


// reimplemented (icomp::CComponentBase)

void CBitmapJoinerCompBase::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	m_resultPtr = NULL;
}


} // namespace icam
