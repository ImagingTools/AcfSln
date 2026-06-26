// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iprm_CSelectionParamCameraComp_included
#define iprm_CSelectionParamCameraComp_included


// Qt includes
#include <QtCore/QMap>

// ACF includes
#include <istd/TDelPtr.h>
#include <QtCore/QString>
#include <iprm/IParamsManager.h>
#include <iprm/IParamsInfoProvider.h>

// ACF-Solutions include
#include <icam/CCameraDelegatorComp.h>


namespace icam
{


class CSelectionParamCameraComp: public CCameraDelegatorComp
{
public:
	typedef CCameraDelegatorComp BaseClass;

	I_BEGIN_COMPONENT(CSelectionParamCameraComp);
		I_ASSIGN(m_selectionIdAttrPtr, "SelectionId", "ID of selection in parameter set", false, "CameraIndex");
		I_ASSIGN(m_defaultIndexAttrPtr, "DefaultIndex", "Default selected index, if negative no selected parameter ID will be used", true, 0);
		I_ASSIGN(m_paramsManagerIdAttrPtr, "ParamsManagerId", "ID of parameters manager in parameter set", false, "ParamsManager");
		I_ASSIGN(m_defaultSelectionCompPtr, "DefaultSelection", "Selection used if no selection ID defined", false, "Input");
		I_ASSIGN(m_defaultParamsManagerCompPtr, "DefaultParamsManager", "Manager of parameter set used to realize selection if no manager ID defined", false, "ParamsManager");
	I_END_COMPONENT;

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
	virtual void InitProcessor(const iprm::IParamsSet* paramsPtr) override;

protected:
	const iprm::IParamsSet* CreateParamsSet(const iprm::IParamsSet* paramsPtr) const;

	class JoinParamsSet: virtual public iprm::IParamsSet
	{
	public:
		JoinParamsSet(const iprm::IParamsSet* origParamsPtr, const iprm::IParamsSet* selectedParamsPtr);

		// reimplemented (iprm::IParamsSet)
		virtual Ids GetParamIds(bool editableOnly = false) const override;
		virtual const iser::ISerializable* GetParameter(const QByteArray& id) const override;
		virtual iser::ISerializable* GetEditableParameter(const QByteArray& id) override;
		virtual const iprm::IParamsInfoProvider* GetParamsInfoProvider() const override;

		// reimplemented (iser::IObject)
		virtual QByteArray GetFactoryId() const override;

		// reimplemented (iser::ISerializable)
		virtual bool Serialize(iser::IArchive& archive) override;

	private:
		const iprm::IParamsSet* m_origParamsPtr;
		const iprm::IParamsSet* m_selectedParamsPtr;
	};

private:
	I_ATTR(QByteArray, m_selectionIdAttrPtr);
	I_ATTR(int, m_defaultIndexAttrPtr);
	I_ATTR(QByteArray, m_paramsManagerIdAttrPtr);
	I_REF(iprm::ISelectionParam, m_defaultSelectionCompPtr);
	I_REF(iprm::IParamsManager, m_defaultParamsManagerCompPtr);

	typedef istd::TDelPtr<const iprm::IParamsSet> ParamSetPtr;
	/**
		Map task ID to parameter set.
	*/
	typedef QMap<int, ParamSetPtr> ParamsMap;

	ParamsMap m_paramsMap;
};


} // namespace icam


#endif // !iprm_CSelectionParamCameraComp_included


