// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ifileproc_CCopyFilesTreeComp_included
#define ifileproc_CCopyFilesTreeComp_included


// Qt includes
#include <QtCore/QString>
#include <QtCore/QDir>

// ACF includes
#include <ilog/TLoggerCompWrap.h>

// ACF-Solutions includes
#include <ifileproc/IFileConversion.h>


namespace ifileproc
{


/**
	Copy files from one directory to another one.
	This component is part of installation concept. It is designed to use it as free component.
*/
class CCopyFilesTreeComp:
			public ilog::CLoggerComponentBase,
			virtual public ifileproc::IFileConversion
{
public:
	typedef ilog::CLoggerComponentBase BaseClass;

	enum MessageId
	{
		MI_END_STATUS = 0xa830,
		MI_NO_INPUT,
		MI_NO_OUTPUT
	};

	I_BEGIN_COMPONENT(CCopyFilesTreeComp);
		I_REGISTER_INTERFACE(ifileproc::IFileConversion);
		I_ASSIGN(m_fileCopyCompPtr, "FileCopy", "Provide copy of single file", true, "FileCopy");
		I_ASSIGN_MULTI_1(m_filtersAttrPtr, "Filters", "List of file filters will be copied", true, "*");
		I_ASSIGN_MULTI_0(m_excludeFiltersAttrPtr, "ExcludeFilters", "List of file filters will be exclude from copy", false);
		I_ASSIGN(m_recursionDepthAttrPtr, "RecursionDepth", "Depth of recursion. Negative value means no depth limitation", true, 0);
		I_ASSIGN(m_inputSubdirAttrPtr, "InputSubDir", "Input sub-directory", true, ".");
		I_ASSIGN(m_outputSubdirAttrPtr, "OutputSubDir", "Output sub-directory", true, ".");
	I_END_COMPONENT;

	// reimplemented (ifileproc::IFileConversion)
	virtual int ConvertFiles(
				const QString& inputPath,
				QString& outputPath,
				const iprm::IParamsSet* paramsPtr = NULL,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) const;

protected:
	/**
		Check if in tree specified files exists.
		\param	inputDir		input directory.
		\param	filters			list of file filter will be copied.
		\param	recursionDepth	maximale recursion depth.
	*/
	bool CheckFileExistTree(
				const QDir& inputDir,
				const QStringList& filters,
				const QStringList& excludeFilters,
				int recursionDepth) const;
	/**
		Copy complete files tree for specified filter.
		\param	inputDir		input directory.
		\param	outputDir		output directory.
		\param	filters			list of file filter will be copied.
		\param	copyFunc		function used to copy single file.
		\param	recursionDepth	maximale recursion depth.
	*/
	bool CopyFileTree(
				const QDir& inputDir,
				const QDir& outputDir,
				const iprm::IParamsSet* paramsPtr,
				const QStringList& filters,
				const QStringList& excludeFilters,
				int recursionDepth,
				int& counter) const;

	/**
		Check if some file should be excluded from processing.
	*/
	bool CheckIfExcluded(const QString& fileName, const QStringList& excludeFilters) const;

private:
	I_REF(ifileproc::IFileConversion, m_fileCopyCompPtr);
	I_MULTIATTR(QString, m_filtersAttrPtr);
	I_MULTIATTR(QString, m_excludeFiltersAttrPtr);
	I_ATTR(int, m_recursionDepthAttrPtr);
	I_ATTR(QString, m_inputSubdirAttrPtr);
	I_ATTR(QString, m_outputSubdirAttrPtr);
};


} // namespace ifileproc


#endif // !ifileproc_CCopyFilesTreeComp_included


