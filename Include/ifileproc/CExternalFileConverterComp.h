// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ifileproc_CExternalFileConverterComp_included
#define ifileproc_CExternalFileConverterComp_included


// Qt includes
#include <QtCore/QProcess>

// ACF includes
#include <ilog/TLoggerCompWrap.h>
#include <iprm/INameParam.h>
#include <ifile/IFileNameParam.h>
#include <ifileproc/IFileConversion.h>


namespace ifileproc
{


/**
	File converter, which uses an external programm to perform the file conversion.
	There are several ways to define how the command line parameter can be passed to the underlaying conversion process.
	The simplest way is to define the complete command line in the text attribute \c DefaultProcessArguments.
	This way works only for use cases, where the command line can be statically defined and is known at the component definition time.
	For the situation, where the process arguments can be changed at runtime, you have following possibilities:
	- Use command line arguments given as a part of the parameter set.
	This option is only active, if the \c DefaultProcessArguments attribute is not set.
	In this case the implementation will try to extract the command line parameters from the input parameter set using \c ProcessArgumentsParamsId as parameter ID.
	- Use explicitely listed arguments given by \c m_additionalArgumentsCompPtr. This is a list of iprm::INameParam objects.
	For integration into the command line, you can use placeholder (variable) in the form $(An),
	where c\ n is the index in the object list of \c additionalArgumentsCompPtr.
	For example, in the command line <MyProcess.exe $(A1)>,
	the $(A1) argument will be replaced by the additionalArgumentsCompPtr[0]->GetName().
	Additional arguments will be added to the already existing command line arguments.
		
	For defining of the command line you can also use some predefined variables. Currently, following variables are supported:
	- $(Input) - \c inputPath argument of ConvertFiles method is used.
	- $(Output) - \c outputPath argument of ConvertFiles method is used.
	- $(OutputDir) - Output directory extracted from \c outputPath argument of ConvertFiles method is used.
*/
class CExternalFileConverterComp:
			public QObject,
			public ilog::CLoggerComponentBase,
			virtual public ifileproc::IFileConversion
{
	Q_OBJECT

public:
	typedef ilog::CLoggerComponentBase BaseClass;

	I_BEGIN_COMPONENT(CExternalFileConverterComp);
		I_REGISTER_INTERFACE(ifileproc::IFileConversion);
		I_ASSIGN(m_executablePathCompPtr, "ExecutablePath", "Path to the converter's executable file", true, "ExecutablePath");
		I_ASSIGN(m_defaultProcessArgumentsAttrPtr, "DefaultProcessArguments", "Application conversion arguments.\nUse $(Input) to specify the input and $(Output) for output file name", false, "$(Input) $(Output)");
		I_ASSIGN(m_processArgumentsParamsIdAttrPtr, "ProcessArgumentsParamsId", "ID of the command line parameter (given as a INameParam object) in the parameter set", true, "ProcessArgumentsParamsId");
		I_ASSIGN_MULTI_0(m_additionalArgumentsCompPtr, "AdditionalArguments", "Additional command line arguments", false);
	I_END_COMPONENT;

	// reimplemented (ifileproc::IFileConversion)
	virtual int ConvertFiles(
				const QString& inputPath,
				QString& outputPath,
				const iprm::IParamsSet* paramsPtr = NULL,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) const;

protected:
	// reimplemented (icomp::CComponentBase)
	virtual void OnComponentCreated() override;

private Q_SLOTS:
	void OnReadyReadStandardError();
	void OnReadyReadStandardOutput();

private:
	I_REF(ifile::IFileNameParam, m_executablePathCompPtr);
	I_ATTR(QString, m_defaultProcessArgumentsAttrPtr);
	I_ATTR(QByteArray, m_processArgumentsParamsIdAttrPtr);
	I_MULTIREF(iprm::INameParam, m_additionalArgumentsCompPtr);

#ifndef QT_NO_PROCESS
	mutable QProcess m_conversionProcess;
#endif
};


} // namespace ifileproc


#endif // !ifileproc_CExternalFileConverterComp_included


