#include <iipr/CPerspectiveCalibrationSupplierComp.h>


// ACF include
#include <ilog/CExtMessage.h>
#include <iprm/TParamsPtr.h>
#include <i2d/CPosition2d.h>

// ACF-Solutions includes
#include <iipr/CPerspCalibFinder.h>
#include <iipr/CPointGridFeature.h>


namespace iipr
{


struct PointGridConsumer : public iipr::IFeaturesConsumer
{
	// reimplemented (iipr::IFeaturesConsumer)

	virtual void ResetFeatures() override
	{
		m_points.Reset();
	}

	virtual bool AddFeature(const imeas::INumericValue* featurePtr, bool* /*isFullPtr = nullptr*/) override
	{
		if (featurePtr != nullptr){
			const icalib::CPointGrid* pointGridPtr = dynamic_cast<const icalib::CPointGrid*>(featurePtr);
			if (pointGridPtr != nullptr){
				m_points.CopyFrom(*pointGridPtr);
			}

			delete featurePtr;
		}

		return true;
	}

	icalib::CPointGrid m_points;
};


// reimplemented (i2d::ICalibrationProvider)

const i2d::ICalibration2d* CPerspectiveCalibrationSupplierComp::GetCalibration() const
{
	return GetWorkProduct();
}


//protected methods

bool CPerspectiveCalibrationSupplierComp::CalculateCalibration(const iimg::IBitmap& image, icalib::CPerspectiveCalibration2d& result) const
{
	result.Reset();

	const iprm::IParamsSet* paramsPtr = GetModelParametersSet();

	if (!m_pointGridExtractorCompPtr.IsValid()){
		SendErrorMessage(0, "Points grid extractor is undefined");
		return false;
	}

	PointGridConsumer imagePoints;
	if (m_pointGridExtractorCompPtr->DoExtractFeatures(paramsPtr, image, imagePoints) != iproc::IProcessor::TS_OK){
		SendErrorMessage(0, "Can not extract of points grid");
		return false;
	}

	double cellSize = *m_defaultCellSizeAttrPtr;
	if (m_cellSizeParamId.IsValid()){
		iprm::TParamsPtr<imeas::INumericValue> checkboardParamsPtr(paramsPtr, *m_cellSizeParamId);
		if (checkboardParamsPtr.IsValid()){
			imath::CVarVector params = checkboardParamsPtr->GetValues();
			if (params.GetElementsCount() > 0){
				cellSize = params.GetElement(0);
			}
		}
	}

	const i2d::ICalibration2d* gridTransformPtr = m_pointGridTransformation.IsValid() ? m_pointGridTransformation->GetCalibration() : nullptr;

	icalib::CPointGrid nominalPositions;
	nominalPositions.SetSizes(imagePoints.m_points.GetSizes());

	const int sizeX = nominalPositions.GetSize(0);
	const int sizeY = nominalPositions.GetSize(1);
	const double stepX = m_invertXAttrPtr.IsValid() && *m_invertXAttrPtr ? -cellSize : cellSize;
	const double stepY = m_invertYAttrPtr.IsValid() && *m_invertYAttrPtr ? -cellSize : cellSize;
	const double gridTop = -(sizeY - 1) * 0.5 * stepY;
	const double gridLeft = -(sizeX - 1) * 0.5 * stepX;

	double nomY = gridTop;
	for (int y = 0; y < sizeY; ++y){
		double nomX = gridLeft;
		for (int x = 0; x < sizeX; ++x){
			i2d::CVector2d normalPos(nomX, nomY);
			istd::TIndex<2> ind;
			ind.SetAt(0, x);
			ind.SetAt(1, y);
			
			if (gridTransformPtr != nullptr){
				i2d::CVector2d transform;
				if (gridTransformPtr->GetPositionAt(normalPos, transform)){
					nominalPositions.SetAt(ind, transform);
				}
				else{
					nominalPositions.SetAt(ind, normalPos);
				}
			}
			else{
				nominalPositions.SetAt(ind, normalPos);
			}

			nomX += stepX;
		}
		nomY += stepY;
	}

	iipr::CPerspCalibFinder calibFinder;
	return calibFinder.FindPerspCalib(
				&nominalPositions.GetElements()[0],
				&imagePoints.m_points.GetElements()[0],
				int(imagePoints.m_points.GetElements().size()),
				result);
}


// reimplemented (iinsp::TSupplierCompWrap)

iinsp::ISupplier::WorkStatus CPerspectiveCalibrationSupplierComp::ProduceObject(ProductType& result) const
{
	if (!m_bitmapProviderCompPtr.IsValid()){
		return WS_FAILED;
	}

	const iimg::IBitmap* bitmapPtr = m_bitmapProviderCompPtr->GetBitmap();
	if (bitmapPtr == nullptr){
		AddMessage(new ilog::CMessage(ilog::CMessage::IC_ERROR, 0, QObject::tr("Input image could not be provided"), "PerspectiveCalibrationSupplier"));

		return WS_FAILED;
	}

	Timer performanceTimer(this, "Perspective calibration");

	if (!CalculateCalibration(*bitmapPtr, result)){
		return WS_FAILED;
	}

	return WS_OK;
}


} // namespace iipr


