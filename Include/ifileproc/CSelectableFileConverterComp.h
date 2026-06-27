// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ifileproc_CSelectableFileConverterComp_included
#define ifileproc_CSelectableFileConverterComp_included


// ACF includes
#include <icomp/CComponentBase.h>

#include <iprm/ISelectionParam.h>
#include <iprm/IOptionsList.h>

#include <ifileproc/IFileConversion.h>


namespace ifileproc
{


class CSelectableFileConverterComp:
			public icomp::CComponentBase,
			virtual public ifileproc::IFileConversion,
			virtual public iprm::ISelectionParam,
			virtual protected iprm::IOptionsList
{
public:
	typedef icomp::CComponentBase BaseClass;

	I_BEGIN_COMPONENT(CSelectableFileConverterComp);
		I_REGISTER_INTERFACE(ifileproc::IFileConversion);
		I_REGISTER_INTERFACE(iprm::ISelectionParam);
		I_REGISTER_INTERFACE(iser::ISerializable);
		I_ASSIGN_MULTI_0(m_slaveConvertersCompPtr, "SlaveConverters", "List of the slave converters", true);
		I_ASSIGN_MULTI_0(m_slaveConverterNamesAttrPtr, "SlaveConverterNames", "List of the converter names", true);
		I_ASSIGN_MULTI_0(m_slaveConverterDescriptionsAttrPtr, "SlaveConverterDescriptions", "List of the converter's descriptions", true);
	I_END_COMPONENT;

	CSelectableFileConverterComp();

	// reimplemented (ifileproc::IFileConversion)
	virtual int ConvertFiles(
				const QString& inputPath,
				QString& outputPath,
				const iprm::IParamsSet* paramsPtr = NULL,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) const;

	// reimplemented (iprm::ISelectionParam)
	virtual const IOptionsList* GetSelectionConstraints() const;
	virtual int GetSelectedOptionIndex() const;
	virtual bool SetSelectedOptionIndex(int index);
	virtual ISelectionParam* GetSubselection(int index) const;

	// reimplemented (iser::ISerializable)
	virtual bool Serialize(iser::IArchive& archive);

	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

protected:
	// reimplemented (iprm::IOptionsList)
	virtual int GetOptionsFlags() const;
	virtual int GetOptionsCount() const;
	virtual QString GetOptionName(int index) const;
	virtual QString GetOptionDescription(int index) const;
	virtual QByteArray GetOptionId(int index) const;
	virtual bool IsOptionEnabled(int index) const;

private:
	I_MULTIREF(ifileproc::IFileConversion, m_slaveConvertersCompPtr);
	I_MULTITEXTATTR(m_slaveConverterNamesAttrPtr);
	I_MULTITEXTATTR(m_slaveConverterDescriptionsAttrPtr);

	int m_optionsCount;
	int m_selectedOptionIndex;
};


} // namespace ifileproc


#endif // !ifileproc_CSelectableFileConverterComp_included
