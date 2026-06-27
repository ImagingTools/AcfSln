// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// ACF includes
#include <icomp/CComponentBase.h>
#include <iproc/TSyncProcessorCompBase.h>
#include <iipr/ILineProjectionProcessor.h>
#include <iipr/IProjectionConstraints.h>
#include <iipr/IFeatureToImageMapper.h>
#include <iipr/IFeatureToProjectionMapper.h>


namespace iipr
{


class CBilinearLineProjectionProcessorComp:
			public iproc::TSyncProcessorCompBase<iipr::ILineProjectionProcessor>,
			virtual public iipr::IFeatureToImageMapper,
			virtual public iipr::IProjectionConstraints
{
public:
	typedef iproc::TSyncProcessorCompBase<ILineProjectionProcessor> BaseClass;

	I_BEGIN_COMPONENT(CBilinearLineProjectionProcessorComp);
		I_REGISTER_INTERFACE(IProjectionConstraints);
		I_REGISTER_INTERFACE(IFeatureToImageMapper);
		I_ASSIGN(m_lineParamIdAttrPtr, "LineParamId", "ID of line parameter in parameter set", true, "LineParam");
		I_ASSIGN(m_featureMapperCompPtr, "FeatureMapper", "Map result feature to projection position, it is needed for mapping operation", false, "FeatureMapper");
	I_END_COMPONENT;

	/**
		Do projection along specified line with variable projection size.
	*/
	template <typename InputPixelType>
	bool DoAutosizeProjection(
				const iimg::IBitmap& bitmap,
				const i2d::CLine2d& bitmapLine,
				imeas::IDataSequence& results) const;

	// reimplemented (iipr::IFeatureToImageMapper)
	virtual bool GetImagePosition(
				const imeas::INumericValue& feature,
				const iprm::IParamsSet* paramsPtr,
				i2d::CVector2d& result) const override;

	// reimplemented (iipr::ILineProjectionProcessor)
	virtual bool DoProjection(
				const iimg::IBitmap& bitmap,
				const i2d::CLine2d& projectionLine,
				const iipr::IProjectionParams* paramsPtr,
				imeas::IDataSequence& results,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (iipr::IProjectionConstraints)
	virtual istd::CRange GetLineWidthRange() const;
	virtual int GetMinProjectionSize() const;
	virtual int GetMaxProjectionSize() const;
	virtual bool IsAutoProjectionSizeSupported() const;

protected:
	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	I_ATTR(QByteArray, m_lineParamIdAttrPtr);
	I_REF(iipr::IFeatureToProjectionMapper, m_featureMapperCompPtr);
};


} // namespace iipr

