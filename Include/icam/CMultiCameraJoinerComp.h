// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// Qt includes
#include <QtCore/QMutex>

// ACF includes
#include <ilog/TLoggerCompWrap.h>
#include <iproc/TSyncProcessorWrap.h>
#include <iprm/ISelectionParam.h>
#include <iprm/IParamsManager.h>

// ACF-Solutions includes
#include <icam/IBitmapAcquisition.h>
#include <icam/CBitmapJoinerCompBase.h>


namespace icam
{


/**
	Joiner of the several cameras.
	The output bitmap contains all sub-bitmaps ordered in the defined layout (grid)
*/
class CMultiCameraJoinerComp:
			public icam::CBitmapJoinerCompBase,
			public iproc::TSyncProcessorWrap<icam::IBitmapAcquisition>
{
public:
	typedef icam::CBitmapJoinerCompBase BaseClass;

	I_BEGIN_COMPONENT(CMultiCameraJoinerComp);
		I_REGISTER_INTERFACE(icam::IBitmapAcquisition);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_ASSIGN(m_cameraCompPtr, "Camera", "Camera object used for image acquisition", true, "Camera");
		I_ASSIGN(m_cameraParamsManagerIdAttrPtr, "CameraParamsManagerId", "ID of the camera params manager in the processing parameter set", true, "CameraParamsManagerId");
		I_ASSIGN(m_cameraParamsManagerCompPtr, "ParamsManager", "Direct connection to the params manager (ParamSelector must be set as well)", false, "ParamsManager");
		I_ASSIGN(m_cameraSelectorCompPtr, "ParamSelector", "Index of the ParamSet in the ParamsManager (it must be set as well)", false, "ParamSelector");
	I_END_COMPONENT;

	// reimplemented (iipr::IBitmapAcquisition)
	virtual istd::CIndex2d GetBitmapSize(const iprm::IParamsSet* paramsPtr) const override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
	virtual void InitProcessor(const iprm::IParamsSet* paramsPtr) override;

private:
	const iprm::IParamsManager* GetCameraParamsManager(const iprm::IParamsSet* paramsPtr) const;

private:
	I_REF(icam::IBitmapAcquisition, m_cameraCompPtr);
	
	I_ATTR(QByteArray, m_cameraParamsManagerIdAttrPtr);

	I_REF(iprm::IParamsManager, m_cameraParamsManagerCompPtr);
	I_REF(iprm::ISelectionParam, m_cameraSelectorCompPtr);

	QMutex m_lock;
};


} // namespace icam


