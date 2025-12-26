#pragma once

// ACF-Solutions includes
#include <iproc/TSyncProcessorCompBase.h>
#include <iipr/IImageToFeatureProcessor.h>


namespace iipr
{


/**
	Extracts NxN points grid from no distorted checkerboard
*/
class CCheckerboardPointGridExtractorComp : public iproc::TSyncProcessorCompBase<iipr::IImageToFeatureProcessor>
{
public:
	typedef iproc::TSyncProcessorCompBase<iipr::IImageToFeatureProcessor> BaseClass;

	I_BEGIN_COMPONENT(CCheckerboardPointGridExtractorComp);
		I_ASSIGN(m_resultConsumerCompPtr, "ResultConsumer", "Consumer of result messages with geometrical layout", false, "ResultConsumer");
		I_ASSIGN(m_lineFinderCompPtr, "LineFinder", "Finder of lines on image", true, "LineFinder");
	I_END_COMPONENT;

	// reimplemented (iipr::IImageToFeatureProcessor)
	virtual iproc::IProcessor::TaskState DoExtractFeatures(
		const iprm::IParamsSet* paramsPtr,
		const iimg::IBitmap& image,
		IFeaturesConsumer& results,
		ibase::IProgressManager* progressManagerPtr = nullptr)  override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
		const iprm::IParamsSet* paramsPtr,
		const istd::IPolymorphic* inputPtr,
		istd::IChangeable* outputPtr,
		ibase::IProgressManager* progressManagerPtr = nullptr) override;

protected:

private:
	I_REF(ilog::IMessageConsumer, m_resultConsumerCompPtr);
	I_REF(iipr::IImageToFeatureProcessor, m_lineFinderCompPtr);
};


} // namespace iipr


