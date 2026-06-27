// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <ifileproc/CSelectableFileConverterComp.h>


// ACF includes
#include <istd/CChangeNotifier.h>

#include <iser/IArchive.h>
#include <iser/CArchiveTag.h>


namespace ifileproc
{


// public methods
	
CSelectableFileConverterComp::CSelectableFileConverterComp()
:	m_optionsCount(0),
	m_selectedOptionIndex(-1)
{
}


// reimplemented (ifileproc::IFileConversion)

int CSelectableFileConverterComp::ConvertFiles(
			const QString& inputPath,
			QString& outputPath,
			const iprm::IParamsSet* /*paramsPtr*/,
			ibase::IProgressManager* /*progressManagerPtr*/) const
{
	if (m_selectedOptionIndex >= 0){
		return m_slaveConvertersCompPtr[m_selectedOptionIndex]->ConvertFiles(inputPath, outputPath);
	}

	return iproc::IProcessor::TS_INVALID;
}


// reimplemented (iprm::ISelectionParam)

const iprm::IOptionsList* CSelectableFileConverterComp::GetSelectionConstraints() const
{
	return this;
}


int CSelectableFileConverterComp::GetSelectedOptionIndex() const
{
	return m_selectedOptionIndex;
}


bool CSelectableFileConverterComp::SetSelectedOptionIndex(int index)
{
	if (index >= 0 && index < m_optionsCount){
		istd::CChangeNotifier changePtr(this);

		m_selectedOptionIndex = index;

		return true;
	}

	return false;
}


iprm::ISelectionParam* CSelectableFileConverterComp::GetSubselection(int /*index*/) const
{
	return NULL;
}


// reimplemented (iser::ISerializable)

bool CSelectableFileConverterComp::Serialize(iser::IArchive& archive)
{
	static iser::CArchiveTag selectedOptionIndexTag("SelectedIndex", "Selected option index", iser::CArchiveTag::TT_LEAF);

	istd::CChangeNotifier notifier(archive.IsStoring()? NULL: this);
	Q_UNUSED(notifier);

	bool retVal = true;

	retVal = retVal && archive.BeginTag(selectedOptionIndexTag);
	retVal = retVal && archive.Process(m_selectedOptionIndex);
	retVal = retVal && archive.EndTag(selectedOptionIndexTag);

	return retVal;
}


// reimplemented (icomp::CComponentBase)

void CSelectableFileConverterComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

	Q_ASSERT(m_slaveConverterNamesAttrPtr.IsValid());
	Q_ASSERT(m_slaveConvertersCompPtr.IsValid());

	m_optionsCount = qMin(m_slaveConvertersCompPtr.GetCount(), m_slaveConverterNamesAttrPtr.GetCount());
	if (m_optionsCount > 0){
		m_selectedOptionIndex = 0;
	}
}


// protected methods

// reimplemented (iprm::IOptionsList)

int CSelectableFileConverterComp::GetOptionsFlags() const
{
	return SCF_NONE;
}

int CSelectableFileConverterComp::GetOptionsCount() const
{
	return m_optionsCount;
}


QString CSelectableFileConverterComp::GetOptionName(int index) const
{
	Q_ASSERT(m_slaveConverterNamesAttrPtr.IsValid());

	return m_slaveConverterNamesAttrPtr[index];
}


QString CSelectableFileConverterComp::GetOptionDescription(int index) const
{
	if (index >= 0 && index < m_slaveConverterDescriptionsAttrPtr.GetCount()){
		return m_slaveConverterDescriptionsAttrPtr[index];
	}

	return QString();
}


QByteArray CSelectableFileConverterComp::GetOptionId(int /*index*/) const
{
	return QByteArray();
}


bool CSelectableFileConverterComp::IsOptionEnabled(int /*index*/) const
{
	return true;
}


} // namespace ifileproc


