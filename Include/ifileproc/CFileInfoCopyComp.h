// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ifileproc_CFileInfoCopyComp_included
#define ifileproc_CFileInfoCopyComp_included


// Qt includes
#include <QtCore/QString>
#include <QtCore/QDir>

// ACF includes
#include <ibase/IApplicationInfo.h>
#include <ilog/TLoggerCompWrap.h>

// ACF-Solutions includes
#include <ifileproc/IFileConversion.h>


namespace ifileproc
{


/**
	Copy of text files with some tags substitution and optional license attachment.
	Tags parameter must be separated using \c : character.
	Tags (inclusive parameter) must be enclosed in \c $ character, for example $AcfCompanyName$ or $AcfVersion:0$.
	Known tasks:
	\li	\c AcfCompanyName - gets name of company
	\li	\c AcfProductName - gets name of product
	\li	\c AcfApplicationName - gets name of application
	\li	\c AcfApplicationSubname - gets sub-name of product
	\li	\c AcfApplicationType - gets type information text
	\li	\c AcfLegalCopyright - gets legal copyright information text
	\li	\c AcfVersion - gets version of application, or its module, if parameter with module ID is specified
	\li	\c AcfRawVersion - gets internal version number of application, or its module, if parameter with module ID is specified
	\li	\c AcfRcVersion - gets version number (in RC compiler compatible format) of application, or its module, if parameter with module ID is specified
	\li	\c AcfTimestamp - gets current time
*/
class CFileInfoCopyComp:
			public QObject,
			public ilog::CLoggerComponentBase,
			virtual public ifileproc::IFileConversion
{
public:
	typedef ilog::CLoggerComponentBase BaseClass;

	enum MessageId
	{
		MI_FILE_INFO = 0xa810,
		MI_INPUT_OPEN,
		MI_OUTPUT_OPEN,
		MI_LICENSE_OPEN,
		MI_BAD_TAG
	};

	I_BEGIN_COMPONENT(CFileInfoCopyComp);
		I_REGISTER_INTERFACE(ifileproc::IFileConversion);
		I_ASSIGN(m_applicationInfoCompPtr, "ApplicationInfo", "Provide information about versions for substitution", false, "VersionInfo");
		I_ASSIGN(m_licensePathAttrPtr, "LicensePath", "Path of license file will be included at begin of copied file", false, "License.txt");
		I_ASSIGN(m_substitutionTagExprAttrPtr, "SubstitutionTagExpr", "Define tag used for substitution as QT regular expression", false, "\\$(\\S+)\\$");
		I_ASSIGN(m_replaceEnvironmentVariablesAttrPtr, "ReplaceEnvironmentVariables", "If enabled, environment variables will be replaced", true, false);
		I_ASSIGN(m_replaceEmbeddedVariablesAttrPtr, "ReplaceEmbeddedVariables", "If enabled, embedded variables will be replaced e.g.:\n\tCompileMode for current compile mode (Debug or Release)\n\tCompilerName for current compiler name (VC9)\n\tConfigurationDir for e.g. VC9_64", true, false);
		I_ASSIGN_MULTI_0(m_userSubstitutionTagsAttrPtr, "UserSubstitutionTags", "List of user defined substitution tags will be replaced with specified values", false);
		I_ASSIGN_MULTI_0(m_userSubstitutionValuesAttrPtr, "UserSubstitutionValues", "List of user substitution values according to specified user tags, it can contains special values:\n\t$RelativeToInputFile:<base file name>\n\t$RelativeToOutputFile:<base file name>\n\t$AbsoluteToInputFile:<base file name>\n\t$AbsoluteToInputFile:<base file name>", false);
		I_ASSIGN(m_ignoreUnknownTagsAttrPtr, "IgnoreUnknownTags", "When enabled the unknown tags will not produce warning messages", true, false);
	I_END_COMPONENT;

	// reimplemented (ifileproc::IFileConversion)
	virtual int ConvertFiles(
				const QString& inputPath,
				QString& outputPath,
				const iprm::IParamsSet* paramsPtr = NULL,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) const;

protected:
	bool ProcessSubstitutionTag(
				const QDir& inputDir,
				const QDir& outputDir,
				const QString& tag,
				QString& result) const;

private:
	I_REF(ibase::IApplicationInfo, m_applicationInfoCompPtr);

	I_ATTR(QString, m_licensePathAttrPtr);
	I_ATTR(QString, m_substitutionTagExprAttrPtr);
	I_ATTR(bool, m_replaceEnvironmentVariablesAttrPtr);
	I_ATTR(bool, m_replaceEmbeddedVariablesAttrPtr);
	I_MULTIATTR(QString, m_userSubstitutionTagsAttrPtr);
	I_MULTIATTR(QString, m_userSubstitutionValuesAttrPtr);
	I_ATTR(bool, m_ignoreUnknownTagsAttrPtr);
};


} // namespace ifileproc


#endif // !ifileproc_CFileInfoCopyComp_included


