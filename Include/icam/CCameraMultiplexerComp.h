// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// ACF includes
#include <iprm/IOptionsList.h>
#include <icomp/CComponentBase.h>
#include <imod/TModelWrap.h>

// ACF-Solutions includes
#include <icam/IBitmapAcquisition.h>


namespace icam
{


/**
	This component allows image acquisition based on a camera selection.
	Multiple independent camera implementations can be registered and switched at runtime.
	Each camera can store its parameters in the Parameter Manager accessed from the supplier's parameter set via 'CameraManagerParamId' attribute.
*/
class CCameraMultiplexerComp: public icomp::CComponentBase, virtual public icam::IBitmapAcquisition
{
public:
	typedef icomp::CComponentBase BaseClass;

	I_BEGIN_COMPONENT(CCameraMultiplexerComp);
		I_REGISTER_INTERFACE(IBitmapAcquisition);
		I_REGISTER_INTERFACE(iproc::IProcessor);
		I_REGISTER_SUBELEMENT(CameraList);
		I_REGISTER_SUBELEMENT_INTERFACE(CameraList, iprm::IOptionsList, ExtractCameraList);
		I_REGISTER_SUBELEMENT_INTERFACE(CameraList, istd::IChangeable, ExtractCameraList);
		I_REGISTER_SUBELEMENT_INTERFACE(CameraList, imod::IModel, ExtractCameraList);
		I_ASSIGN_MULTI_0(m_cameraListCompPtr, "Cameras", "List of cameras used for snap", true);
		I_ASSIGN_MULTI_0(m_cameraIdsAttrPtr, "CameraIds", "List of camera IDs", true);
		I_ASSIGN_MULTI_0(m_cameraNamesAttrPtr, "CameraNames", "List of camera names", true);
		I_ASSIGN(m_sourceSelectionParamIdAttrPtr, "CameraSelectorId", "ID of the source camera selector parameter in the supplier's parameter set", true, "CameraSelectorId");
		I_ASSIGN(m_cameraManagerParamIdAttrPtr, "CameraManagerParamId", "ID of the camera parameter manager in the supplier's parameter set", true, "CameraManagerParamId");
		I_ASSIGN(m_parameterSetCompPtr, "ParamsSet", "Parameter set for camera selection", false, "ParamsSet");
	I_END_COMPONENT;

	CCameraMultiplexerComp();

	// reimplemented (iipr::IBitmapAcquisition)
	virtual istd::CIndex2d GetBitmapSize(const iprm::IParamsSet* paramsPtr) const override;

	// reimplemented (iproc::IProcessor)
	virtual iproc::IProcessor::ProcessorState GetProcessorState(const iprm::IParamsSet* paramsPtr) const override;
	virtual bool AreParamsAccepted(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				const istd::IChangeable* outputPtr) const override;
	virtual iproc::IProcessor::TaskState DoProcessing(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
	virtual int BeginTask(
				const iprm::IParamsSet* paramsPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) override;
	virtual iproc::IProcessor::TaskState WaitTaskFinished(
					int taskId = -1,
					double timeoutTime = -1,
					bool killOnTimeout = true) override;
	virtual void CancelTask(int taskId = -1) override;
	virtual int GetReadyTask() override;
	virtual iproc::IProcessor::TaskState GetTaskState(int taskId = -1) const override;
	virtual void InitProcessor(const iprm::IParamsSet* paramsPtr) override;

private:
	class CameraList: virtual public iprm::IOptionsList
	{
	public:
		CameraList();

		void SetParent(CCameraMultiplexerComp* parentPtr);

		// reimplemented (iprm::IOptionsList)
		virtual int GetOptionsFlags() const override;
		virtual int GetOptionsCount() const override;
		virtual QString GetOptionName(int index) const override;
		virtual QString GetOptionDescription(int index) const override;
		virtual QByteArray GetOptionId(int index) const override;
		virtual bool IsOptionEnabled(int index) const override;
		
	private:
		CCameraMultiplexerComp* m_parentPtr;
	};

	template <class InterfaceType>
	static InterfaceType* ExtractCameraList(CCameraMultiplexerComp& component)
	{
		return &component.m_cameraList;
	}

	int GetSelectedIndex(const iprm::IParamsSet* paramsPtr) const;
	const iprm::IParamsSet* GetWorkingParamsSet(const iprm::IParamsSet* paramsPtr) const;
	IBitmapAcquisition* GetSelectedCamera(const iprm::IParamsSet* paramsPtr) const;

private:
	I_MULTIREF(IBitmapAcquisition, m_cameraListCompPtr);
	I_MULTIATTR(QByteArray, m_cameraIdsAttrPtr);
	I_MULTITEXTATTR(m_cameraNamesAttrPtr);
	I_ATTR(QByteArray, m_sourceSelectionParamIdAttrPtr);
	I_ATTR(QByteArray, m_cameraManagerParamIdAttrPtr);
	I_REF(iprm::IParamsSet, m_parameterSetCompPtr);

	imod::TModelWrap<CameraList> m_cameraList;
	mutable const iprm::IParamsSet*  m_currentParamsSetPtr;
};


} // namespace icam

