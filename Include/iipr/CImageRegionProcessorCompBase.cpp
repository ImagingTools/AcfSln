#include <iipr/CImageRegionProcessorCompBase.h>


 // ACF includes
#include <istd/CChangeNotifier.h>
#include <ibase/CSize.h>
#include <i2d/IObject2d.h>
#include <i2d/ICalibrationProvider.h>
#include <iimg/IBitmap.h>
#include <iprm/TParamsPtr.h>


namespace iipr
{


// public methods

// reimplemented (iproc::IProcessor)

iproc::IProcessor::TaskState CImageRegionProcessorCompBase::DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* /*progressManagerPtr*/)
{
	const iimg::IBitmap* inputBitmapPtr = dynamic_cast<const iimg::IBitmap*>(inputPtr);
	if (inputBitmapPtr == nullptr){
		SendErrorMessage(0, "Processor input is not a bitmap");

		return TS_INVALID;
	}

	istd::CChangeNotifier changeNotifier(outputPtr);

	i2d::CRectangle inputImageArea(inputBitmapPtr->GetImageSize());

	istd::TUniqueInterfacePtr<i2d::IObject2d> transformedRegionPtr;

	iprm::TParamsPtr<i2d::IObject2d> aoiPtr(paramsPtr, m_aoiParamIdAttrPtr, m_defaultAoiCompPtr, false);
	if (aoiPtr.IsValid()){
		const i2d::ICalibration2d* logToPhysicalTransformPtr = aoiPtr->GetCalibration();
		if (logToPhysicalTransformPtr != nullptr){
			transformedRegionPtr.MoveCastedPtr<istd::IChangeable>(aoiPtr->CloneMe());
			if (transformedRegionPtr.IsValid()){
				transformedRegionPtr->Transform(*logToPhysicalTransformPtr);
				transformedRegionPtr->SetCalibration(nullptr);

				aoiPtr.SetPtr(transformedRegionPtr.GetPtr());
			}
		}
	}
	else{
		if (*m_allowEmptyRegionAttrPtr){
			aoiPtr.SetPtr(&inputImageArea);
		}
	}

	bool retVal = ProcessImageRegion(*inputBitmapPtr, paramsPtr, aoiPtr.GetPtr(), outputPtr);

	iimg::IBitmap* outputBitmapPtr = dynamic_cast<iimg::IBitmap*>(outputPtr);
	if (outputBitmapPtr != nullptr){
		const i2d::ICalibration2d* inputCalibrationPtr = inputBitmapPtr->GetCalibration();
		if (inputCalibrationPtr != nullptr){
			istd::TUniqueInterfacePtr<i2d::ICalibration2d> outputCalibrationPtr;
			outputCalibrationPtr.MoveCastedPtr(inputCalibrationPtr->CloneMe());

			outputBitmapPtr->SetCalibration(outputCalibrationPtr.PopInterfacePtr(), true);
		}
	}

	return retVal ? TS_OK: TS_INVALID;
}


} // namespace iipr


