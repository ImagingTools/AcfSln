// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iipr_CLineProjectionProcessorComp_included
#define iipr_CLineProjectionProcessorComp_included


// Qt includes
#include <QtCore/QString>

// ACF includes
#include <icomp/CComponentBase.h>

// ACF-Solutions includes
#include <iproc/TSyncProcessorCompBase.h>
#include <iipr/ILineProjectionProcessor.h>
#include <iipr/IProjectionConstraints.h>
#include <iipr/IFeatureToImageMapper.h>
#include <iipr/IFeatureToProjectionMapper.h>


namespace iipr
{


class CLineProjectionProcessorComp:
			public iproc::TSyncProcessorCompBase<ILineProjectionProcessor>,
			virtual public IFeatureToImageMapper,
			virtual public IProjectionConstraints
{
public:
	typedef iproc::TSyncProcessorCompBase<ILineProjectionProcessor> BaseClass;

	I_BEGIN_COMPONENT(CLineProjectionProcessorComp);
		I_REGISTER_INTERFACE(IProjectionConstraints);
		I_REGISTER_INTERFACE(IFeatureToImageMapper);
		I_ASSIGN(m_lineParamIdAttrPtr, "LineParamId", "ID of line parameter in parameter set", true, "LineParam");
		I_ASSIGN(m_featureMapperCompPtr, "FeatureMapper", "Map result feature to projection position, it is needed for mapping operation", false, "FeatureMapper");
	I_END_COMPONENT;

	/**
		Do projection along specified line with variable projection size.
	*/
	template <typename InputPixelType, typename DestPixel, typename CalcPixel>
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
				const IProjectionParams* paramsPtr,
				imeas::IDataSequence& results) override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;

	// reimplemented (iipr::IProjectionConstraints)
	virtual istd::CRange GetLineWidthRange() const override;
	virtual int GetMinProjectionSize() const override;
	virtual int GetMaxProjectionSize() const override;
	virtual bool IsAutoProjectionSizeSupported() const override;

protected:
	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private:
	I_ATTR(QByteArray, m_lineParamIdAttrPtr);
	I_REF(IFeatureToProjectionMapper, m_featureMapperCompPtr);
};


} // namespace iipr


#endif // !iipr_CLineProjectionProcessorComp_included


