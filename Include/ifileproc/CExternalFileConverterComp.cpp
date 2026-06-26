// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <ifileproc/CExternalFileConverterComp.h>


// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

// ACF includes
#include <iprm/TParamsPtr.h>


namespace ifileproc
{


// reimplemented (ifileproc::IFileConversion)

int CExternalFileConverterComp::ConvertFiles(
			const QString& inputPath,
			QString& outputPath,
			const iprm::IParamsSet* paramsSetPtr,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/) const
{
	if (!m_executablePathCompPtr.IsValid()){
		SendErrorMessage(0, "Path for an executable was not set");

		return iproc::IProcessor::TS_INVALID;
	}

	if (m_executablePathCompPtr->GetPathType() != ifile::IFileNameParam::PT_FILE){
		SendErrorMessage(0, "Wrong executable path type. Must be path to a file.");

		return iproc::IProcessor::TS_INVALID;
	}

	QStringList arguments;

	// setup command line arguments:
	if (!m_defaultProcessArgumentsAttrPtr.IsValid()){
		if (paramsSetPtr != NULL){
			iprm::TParamsPtr<iprm::INameParam> commandLineParamPtr(paramsSetPtr, *m_processArgumentsParamsIdAttrPtr);
			if (commandLineParamPtr.IsValid()){
				arguments = commandLineParamPtr->GetName().split(" ");
			}
		}
		else{
			arguments.push_back(inputPath);
			arguments.push_back(outputPath);
		}
	}
	else{
		QString applicationArguments = *m_defaultProcessArgumentsAttrPtr;

		arguments = applicationArguments.split(" ");
	}

	QFileInfo outputFileInfo(outputPath);

	QString outputDirectory = outputFileInfo.dir().absolutePath();

	for (int argIndex = 0; argIndex < int(arguments.size()); argIndex++){
		arguments[argIndex].replace("$(Input)", inputPath);
		arguments[argIndex].replace("$(Output)", outputPath);
		arguments[argIndex].replace("$(OutputDir)", outputDirectory);
	}

	if (m_additionalArgumentsCompPtr.IsValid()){
		int additionalArgumentsCount = m_additionalArgumentsCompPtr.GetCount();

		for (int addIndex = 0; addIndex < additionalArgumentsCount; addIndex++){
			iprm::INameParam* argumentPtr = m_additionalArgumentsCompPtr[addIndex];
			if (argumentPtr != NULL){
				QString argument = argumentPtr->GetName();

				bool placeHolderFound = false;
				for (int argIndex = 0; argIndex < int(arguments.size()); argIndex++){
					QString& existingArgument = arguments[argIndex];
					QString expressionString = QString("$(A%1)").arg(addIndex + 1);

					iprm::INameParam* additionalArgumentPtr = m_additionalArgumentsCompPtr[addIndex];
					if (additionalArgumentPtr != NULL){
						QString replacement = additionalArgumentPtr->GetName();

						if (existingArgument.contains(expressionString)){
							existingArgument.replace(expressionString, replacement);
							
							placeHolderFound = true;
						}
					}
				}

				if (!placeHolderFound){
					arguments.push_back(argument);
				}
			}
		}
	}

	SendVerboseMessage(QString("Process started as %1 %2").arg(m_executablePathCompPtr->GetPath()).arg(arguments.join(" ")));

#ifndef QT_NO_PROCESS
	m_conversionProcess.start(m_executablePathCompPtr->GetPath(), arguments);

	m_conversionProcess.waitForFinished(-1);

	if (m_conversionProcess.error() != QProcess::UnknownError){
		return iproc::IProcessor::TS_INVALID;
	}

	return (m_conversionProcess.exitCode() == 0) ? iproc::IProcessor::TS_OK : iproc::IProcessor::TS_INVALID;
#else
	return iproc::IProcessor::TS_INVALID;
#endif
}


// protected methods

// reimplemented (icomp::CComponentBase)
	
void CExternalFileConverterComp::OnComponentCreated()
{
	BaseClass::OnComponentCreated();

#ifndef QT_NO_PROCESS
	connect(&m_conversionProcess, SIGNAL(readyReadStandardError()), this, SLOT(OnReadyReadStandardError()));
	connect(&m_conversionProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(OnReadyReadStandardOutput()));
#endif
}


// private slots

void CExternalFileConverterComp::OnReadyReadStandardError()
{
#ifndef QT_NO_PROCESS
	QString errorOutput = m_conversionProcess.readAllStandardError();
	
	errorOutput = errorOutput.simplified();

	SendErrorMessage(0, errorOutput);
#endif
}


void CExternalFileConverterComp::OnReadyReadStandardOutput()
{
#ifndef QT_NO_PROCESS
	QString standardOutput = m_conversionProcess.readAllStandardOutput();

	standardOutput = standardOutput.simplified();

	SendInfoMessage(0, standardOutput);
#endif
}



} // namespace ifileproc


