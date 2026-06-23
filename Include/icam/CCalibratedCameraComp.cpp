// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <icam/CCalibratedCameraComp.h>


// STD includes
#include <cmath>

// ACF includes
#include <iprm/TParamsPtr.h>
#include <imeas/INumericValue.h>


using namespace std;


namespace icam
{


// public static methods

static const quint32 s_resolutionPattern = 0xff00ff00;
static const quint32 s_resolutionPatternXY = 0x00ff00ff;


bool CCalibratedCameraComp::ReadImageResolution(const iimg::IBitmap& bitmap, double& resolutionX, double& resolutionY)
{
	iimg::IBitmap::PixelFormat pixelFormat = bitmap.GetPixelFormat();

	istd::CIndex2d imageSize = bitmap.GetImageSize();
	if (!imageSize.IsSizeEmpty())
	{
		if (imageSize.GetX() >= int(2 * sizeof(double) + sizeof(quint32)))
		{
			if (pixelFormat != iimg::IBitmap::PF_RGB && pixelFormat != iimg::IBitmap::PF_RGBA)
			{
				const quint8* rawData = (quint8*)bitmap.GetLinePtr(0);
				Q_ASSERT(rawData != NULL);
				quint32& bitmapPattern = (quint32&)rawData[0];
				double& value = (double&)rawData[sizeof(quint32)];
				double& value2 = (double&)rawData[sizeof(quint32) + sizeof(double)];

				// old
				if (bitmapPattern == s_resolutionPattern && !qIsNaN(value) && (value > 0)){
					resolutionX = resolutionY = value;
					return true;
				}

				// new
				if (bitmapPattern == s_resolutionPatternXY && !qIsNaN(value) && (value > 0) && !qIsNaN(value2) && (value2 > 0)){
					resolutionX = value;
					resolutionY = value2;
					return true;
				}
			}
			else
			{
				const quint8* rawData = (quint8*)bitmap.GetLinePtr(0);
				quint32 bitmapPattern = rawData[1] << 24;
				bitmapPattern |= (rawData[2] & 0xff) << 16; 
				bitmapPattern |= (rawData[5] & 0xff) << 8; 
				bitmapPattern |= rawData[6] & 0xff;

				if (bitmapPattern == s_resolutionPattern){
					union{
						quint8 data[8];
						double value;
					} res;
					res.value = 0;

					res.data[0]= rawData[9];
					res.data[1]= rawData[10];
					res.data[2]= rawData[13];
					res.data[3]= rawData[14];
					res.data[4]= rawData[17];
					res.data[5]= rawData[18];
					res.data[6]= rawData[21];
					res.data[7]= rawData[22];

					if (!qIsNaN(res.value) && (res.value > 0)){
						resolutionX = resolutionY = res.value;

						return true;
					}
				}

				if (bitmapPattern == s_resolutionPatternXY){
					union {
						quint8 data[8];
						double value;
					} res, res2;

					res.value = 0;
					res2.value = 0;

					res.data[0] = rawData[9];
					res.data[1] = rawData[10];
					res.data[2] = rawData[13];
					res.data[3] = rawData[14];
					res.data[4] = rawData[17];
					res.data[5] = rawData[18];
					res.data[6] = rawData[21];
					res.data[7] = rawData[22];

					res2.data[0] = rawData[25];
					res2.data[1] = rawData[26];
					res2.data[2] = rawData[29];
					res2.data[3] = rawData[30];
					res2.data[4] = rawData[33];
					res2.data[5] = rawData[34];
					res2.data[6] = rawData[37];
					res2.data[7] = rawData[38];

					if (!qIsNaN(res.value) && (res.value > 0) && !qIsNaN(res2.value) && (res2.value > 0)) 
					{
						resolutionX = res.value;
						resolutionY = res2.value;
						return true;
					}
				}
			}
		}
	}

	return false;
}


bool CCalibratedCameraComp::WriteImageResolution(iimg::IBitmap& bitmap, double resolutionX, double resolutionY)
{
	iimg::IBitmap::PixelFormat pixelFormat = bitmap.GetPixelFormat();

	istd::CIndex2d imageSize = bitmap.GetImageSize();
	if (!imageSize.IsSizeEmpty()) 
	{
		if (imageSize.GetX() >= int(2 * sizeof(double) + sizeof(quint32))) 
		{
			quint8* rawData = (quint8*)bitmap.GetLinePtr(0);
			Q_ASSERT(rawData != NULL);

			if (pixelFormat != iimg::IBitmap::PF_RGB && pixelFormat != iimg::IBitmap::PF_RGBA) 
			{
				quint32& bitmapPattern = (quint32&)rawData[0];
				double& value1 = (double&)rawData[sizeof(quint32)];
				double& value2 = (double&)rawData[sizeof(double) + sizeof(quint32)];

				bitmapPattern = quint32(s_resolutionPatternXY);
				value1 = resolutionX;
				value2 = resolutionY;

				return true;
			}
			else {
				rawData[1] = 0x00;
				rawData[2] = 0xff;
				rawData[5] = 0x00;
				rawData[6] = 0xff;

				quint8* resolutionDataX = (quint8*)&resolutionX;
				quint8* resolutionDataY = (quint8*)&resolutionY;

				rawData[9] = resolutionDataX[0];
				rawData[10] = resolutionDataX[1];
				rawData[13] = resolutionDataX[2];
				rawData[14] = resolutionDataX[3];
				rawData[17] = resolutionDataX[4];
				rawData[18] = resolutionDataX[5];
				rawData[21] = resolutionDataX[6];
				rawData[22] = resolutionDataX[7];

				rawData[25] = resolutionDataY[0];
				rawData[26] = resolutionDataY[1];
				rawData[29] = resolutionDataY[2];
				rawData[30] = resolutionDataY[3];
				rawData[33] = resolutionDataY[4];
				rawData[34] = resolutionDataY[5];
				rawData[37] = resolutionDataY[6];
				rawData[38] = resolutionDataY[7];

				return true;
			}
		}
	}

	return false;
}


// public methods

// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CCalibratedCameraComp::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* /*processingReportPtr*/)
{
	iproc::IProcessor::TaskState retVal = BaseClass::DoProcessing(paramsPtr, inputPtr, outputPtr, progressManagerPtr);
	if (retVal == TS_OK)
	{
		iprm::TParamsPtr<imeas::INumericValue> scaleParamPtr(paramsPtr, *m_scaleParamIdAttrPtr);
		if (scaleParamPtr.IsValid()){
			iimg::IBitmap* bitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
			if (bitmapPtr != NULL){
				double targetResolutionX = 1.0;
				double targetResolutionY = 1.0;
				if (!ReadImageResolution(*bitmapPtr, targetResolutionX, targetResolutionY)){
					imath::CVarVector scaleValues = scaleParamPtr->GetValues();
					if (scaleValues.GetElementsCount() >= 2){
						targetResolutionX = scaleValues[0];
						targetResolutionY = scaleValues[1];
					}

					WriteImageResolution(*bitmapPtr, targetResolutionX, targetResolutionY);
				}
			}
		}
	}

	return retVal;
}


} // namespace icam


