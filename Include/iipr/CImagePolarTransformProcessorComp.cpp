#include <iipr/CImagePolarTransformProcessorComp.h>


 // ACF includes
#include <istd/CChangeNotifier.h>
#include <ibase/CSize.h>
#include <i2d/CAnnulusSegment.h>
#include <iprm/TParamsPtr.h>
#include <iimg/CScanlineMask.h>
#include <imeas/INumericValue.h>
#include <iipr/TImagePixelInterpolator.h>

// Standard includes
#include <vector>


namespace iipr
{


// helper class for bilinear interpolation

class CGrayBilinearInterpolator
{
public:
	CGrayBilinearInterpolator(const iimg::IBitmap& bitmap)
	{
		m_imageBufferPtr = (quint8*)bitmap.GetLinePtr(0);
		m_imageLineDifference = bitmap.GetLinesDifference();
		m_imageHeight = bitmap.GetImageSize().GetY();
		m_imageWidth = bitmap.GetImageSize().GetX();
		m_pixelBytesCount = bitmap.GetPixelBitsCount() / 8;
	}

	quint8 GetPixel(float x, float y) const
	{
		// Clamp integer coordinates to image dimensions
		int px = int(x);
		int py = int(y);

		if (px < 0){
			px = 0;
		}
		else if (px >= m_imageWidth - 1){
			px = m_imageWidth - 2;
		}

		if (py < 0){
			py = 0;
		}
		else if (py >= m_imageHeight - 1){
			py = m_imageHeight - 2;
		}

		// Calculate the weights for each pixel
		float fx = x - px;
		float fy = y - py;
		float fx1 = 1.0f - fx;
		float fy1 = 1.0f - fy;

		float w1 = fx1 * fy1;
		float w2 = fx * fy1;
		float w3 = fx1 * fy;
		float w4 = fx * fy;

		// Interpolate & clamp
		quint8* ptr = (m_imageBufferPtr + px * m_pixelBytesCount + m_imageLineDifference * py);
		quint8* ptr2 = ptr + m_imageLineDifference;

		float retVal = w1 * *ptr + w2 * *(ptr + 1) + w3 * *ptr2 + w4 * *(ptr2 + 1);

		return retVal > 255 ? 255 : (quint8)retVal;
	}

private:
	quint8 *m_imageBufferPtr;
	int m_imageLineDifference;
	int m_imageHeight;
	int m_imageWidth;
	int m_pixelBytesCount;
};


// protected methods

// reimplemented (CImageRegionProcessorCompBase)

bool CImagePolarTransformProcessorComp::ProcessImageRegion(
			const iimg::IBitmap& inputBitmap,
			const iprm::IParamsSet* paramsPtr,
			const i2d::IObject2d* aoiPtr,
			istd::IChangeable* outputPtr) const
{
	if (inputBitmap.IsEmpty()){
		return true;
	}

	double angleResolution = 1.0;
	
	if (m_angleResolutionParamIdAttrPtr.IsValid()){
		iprm::TParamsPtr<imeas::INumericValue> angleResolutionParamPtr(paramsPtr, *m_angleResolutionParamIdAttrPtr);
	
		if (angleResolutionParamPtr.IsValid()){
			imath::CVarVector values = angleResolutionParamPtr->GetValues();
			if (!values.IsEmpty()){
				angleResolution = values[0];
			}
		}
	}

	int interpolationMode = iipr::IImageInterpolationParams::IM_NO_INTERPOLATION;
	if (m_interpolationParamIdAttrPtr.IsValid()){
		iprm::TParamsPtr<iipr::IImageInterpolationParams> interpolationParamPtr(paramsPtr, *m_interpolationParamIdAttrPtr);
	
		if (interpolationParamPtr.IsValid()){
			interpolationMode = interpolationParamPtr->GetInterpolationMode();
		}
	}
	
	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr == nullptr){
		return false;
	}

	const i2d::IObject2d* realAoiPtr = aoiPtr;
	i2d::CRectangle imageRect(inputBitmap.GetImageSize());
	if (aoiPtr == nullptr){
		realAoiPtr = &imageRect;
	}

	iimg::CScanlineMask bitmapRegion;
	i2d::CRect clipArea(inputBitmap.GetImageSize());
	if (!bitmapRegion.CreateFromGeometry(*realAoiPtr, &clipArea)){
		return false;
	}

	if (bitmapRegion.IsBitmapRegionEmpty()){
		SendWarningMessage(0, "Cannot process an empty region");
		
		return false;
	}

	i2d::CRect regionRect = bitmapRegion.GetBoundingBox();
	i2d::CVector2d aoiCenter = regionRect.GetCenter();
	i2d::CVector2d diffVector = aoiCenter - i2d::CVector2d(regionRect.GetLeftTop());
	int radius = qCeil(diffVector.GetLength());
	int angleDimension = int(radius * I_PI + 0.5f) / angleResolution;
	float startAngle = 0.0f;
	float angleRange = I_2PI;

	int r1 = 0;
	int r2 = radius;

	float extraUnrolling = 1.0f;
	iprm::TParamsPtr<imeas::INumericValue> extraUnrollingParamPtr(paramsPtr, *m_extraUnrollingAttrPtr);
	if (extraUnrollingParamPtr.IsValid()){
		imath::CVarVector values = extraUnrollingParamPtr->GetValues();
		Q_ASSERT(!values.IsEmpty());
		if (!values.IsEmpty()){
			extraUnrolling = values[0];
		}
	}

	const i2d::CAnnulus* annulusPtr = dynamic_cast<const i2d::CAnnulus*>(realAoiPtr);
	if (annulusPtr != nullptr){
		r1 = qCeil(annulusPtr->GetInnerRadius());
		r2 = qFloor(annulusPtr->GetOuterRadius());
		radius = (r2 - r1);

		angleDimension *= extraUnrolling;
		angleRange *= extraUnrolling;
	}

	const i2d::CCircle* circlePtr = dynamic_cast<const i2d::CCircle*>(realAoiPtr);
	if (circlePtr != nullptr){
		r1 = 0;
		r2 = qFloor(circlePtr->GetRadius());
		radius = r2;

		angleDimension *= extraUnrolling;
		angleRange *= extraUnrolling;
	}

	const i2d::CAnnulusSegment* annulusSegmentPtr = dynamic_cast<const i2d::CAnnulusSegment*>(realAoiPtr);
	if (annulusSegmentPtr != nullptr){
		r1 = qCeil(annulusSegmentPtr->GetInnerRadius());
		r2 = qFloor(annulusSegmentPtr->GetOuterRadius());
		radius = (r2 - r1);

		int length = qCeil(diffVector.GetLength());

		angleRange = fabs(annulusSegmentPtr->GetEndAngle() - annulusSegmentPtr->GetBeginAngle());

		angleDimension = int(0.5 * length * angleRange + 0.5) / angleResolution;

		startAngle = annulusSegmentPtr->GetBeginAngle();
	}

	if (!outputBitmapPtr->CreateBitmap(inputBitmap.GetPixelFormat(), istd::CIndex2d(angleDimension, radius))){
		return false;
	}

	int pixelComponentsCount = inputBitmap.GetComponentsCount();

	std::vector<float> cosTable(angleDimension);
	std::vector<float> sinTable(angleDimension);

	for (int alpha = 0; alpha < angleDimension; alpha++){
		float angle = alpha / float(angleDimension) * angleRange * angleResolution;
		angle += startAngle;
	
		cosTable[alpha] = qCos(angle);
		sinTable[alpha] = qSin(angle);
	}

	// special case for 8-bit grayscale bilinear interpolation
	if (pixelComponentsCount == 1 && interpolationMode == iipr::IImageInterpolationParams::IM_BILINEAR){
		CGrayBilinearInterpolator pixelProvider(inputBitmap);

		for (int r = 0; r < radius; r++){
			quint8* outputImageBeginPtr = (quint8*)outputBitmapPtr->GetLinePtr(r);

			for (int alpha = 0; alpha < angleDimension; alpha++){
				float x = (r + r1) * cosTable[alpha];
				float y = (r + r1) * sinTable[alpha];

				x += aoiCenter.GetX();
				y += aoiCenter.GetY();

				*(outputImageBeginPtr + alpha) = pixelProvider.GetPixel(x, y);
			}
		}
	}
	else{	// old solution
		iipr::TImagePixelInterpolator<quint8> pixelInterpolator(inputBitmap, interpolationMode);

		for (int componentIndex = 0; componentIndex < pixelComponentsCount; componentIndex++){
			for (int r = 0; r < radius; r++){
				quint8* outputImageBeginPtr = (quint8*)outputBitmapPtr->GetLinePtr(r);

				for (int alpha = 0; alpha < angleDimension; alpha++){
					float x = (r + r1) * cosTable[alpha];
					float y = (r + r1) * sinTable[alpha];

					x += aoiCenter.GetX();
					y += aoiCenter.GetY();

					*(outputImageBeginPtr + alpha * pixelComponentsCount + componentIndex) = pixelInterpolator.GetInterpolatedValue(x, y, componentIndex);
				}
			}
		}
	}
		
	// cosTable and sinTable automatically cleaned up by std::vector destructor

	return true;
}


} // namespace iipr


