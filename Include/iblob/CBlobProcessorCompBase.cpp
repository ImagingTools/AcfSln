// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iblob/CBlobProcessorCompBase.h>


// ACF includes
#include <iprm/TParamsPtr.h>


namespace iblob
{


static QByteArray areaFeatureId("Area");
static QByteArray perimeterFeatureId("Perimeter");
static QByteArray circularityFeatureId("Circularity");


// reimplemented (iipr::IImageToFeatureProcessor)

iproc::IProcessor::TaskState CBlobProcessorCompBase::DoExtractFeatures(
			const iprm::IParamsSet* paramsPtr,
			const iimg::IBitmap& image,
			iipr::IFeaturesConsumer& results,
			ibase::IProgressManager* /*progressManagerPtr*/)
{
	if (paramsPtr == NULL){
		return TS_INVALID;
	}

	iprm::TParamsPtr<i2d::IObject2d> aoiPtr(paramsPtr, m_aoiParamIdAttrPtr, m_defaultAoiCompPtr);

	istd::TUniqueInterfacePtr<i2d::IObject2d> transformedRegionPtr;
	if (aoiPtr.IsValid()){
		const i2d::ICalibration2d* logToPhysicalTransformPtr = aoiPtr->GetCalibration();
		if (logToPhysicalTransformPtr != NULL){
			transformedRegionPtr.MoveCastedPtr(aoiPtr->CloneMe());
			if (transformedRegionPtr.IsValid()){
				transformedRegionPtr->SetCalibration(nullptr);
				transformedRegionPtr->Transform(*logToPhysicalTransformPtr);
			}
		}
	}

	iprm::TParamsPtr<iblob::IBlobFilterParams> filterParamsPtr(paramsPtr, m_filterParamsIdAttrPtr, m_defaultFilterParamsCompPtr);

	bool retVal = CalculateBlobs(paramsPtr, filterParamsPtr.GetPtr(), transformedRegionPtr.GetPtr(), image, results);

	return retVal ? TS_OK: TS_INVALID;
}


// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CBlobProcessorCompBase::DoProcessing(
			const iprm::IParamsSet* paramsPtr,
			const istd::IPolymorphic* inputPtr,
			istd::IChangeable* outputPtr,
			ibase::IProgressManager* progressManagerPtr,
			istd::IChangeable* /*processingReportPtr*/)
{
	const iimg::IBitmap* inputBitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	if (inputBitmapPtr == NULL){
		return TS_INVALID;
	}

	if (outputPtr == NULL){
		return TS_OK;
	}

	iipr::IFeaturesConsumer* outputConsumerPtr = dynamic_cast<iipr::IFeaturesConsumer*>(outputPtr);
	if (outputConsumerPtr == NULL){
		return TS_INVALID;
	}

	return DoExtractFeatures(paramsPtr, *inputBitmapPtr, *outputConsumerPtr, progressManagerPtr);
}


// protected static methods

bool CBlobProcessorCompBase::IsBlobAcceptedByFilter(const iblob::IBlobFilterParams& filterParams, double area, double perimeter, double circularity)
{
	if (filterParams.IsFiltersEnabled()){
		int filtersCount = filterParams.GetFiltersCount();

		for (int filterIndex = 0; filterIndex < filtersCount; ++filterIndex){
			const iblob::IBlobFilterParams::Filter& filter = filterParams.GetFilterAt(filterIndex);

			if (filter.featureId == areaFeatureId){
				if (!filterParams.IsValueAcceptedByFilter(filterIndex, area)){
					return false;
				}
			}

			if (filter.featureId == perimeterFeatureId){
				if (!filterParams.IsValueAcceptedByFilter(filterIndex, perimeter)){
					return false;
				}
			}

			if (filter.featureId == circularityFeatureId){
				if (!filterParams.IsValueAcceptedByFilter(filterIndex, circularity)){
					return false;
				}
			}
		}
	}

	return true;
}


// public methods of embedded class FilterFeatureList

CBlobProcessorCompBase::FilterFeatureList::FilterFeatureList()
{
	InsertOption("Area", areaFeatureId, "Blob area");
	InsertOption("Perimeter", perimeterFeatureId, "Total length of edges in a blob (including the edges of any holes)");
	InsertOption("Circularity", circularityFeatureId, "Circularity factor of the blob");
}


} // namespace iblob


