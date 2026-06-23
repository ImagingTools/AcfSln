// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <ifileproc/CFileInfoCopyComp.h>


// Qt includes
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QRegularExpression>

// ACF includes
#include <iser/IVersionInfo.h>
#include <istd/CSystem.h>


#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define QT_ENDL Qt::endl
#else
#define QT_ENDL endl
#endif


namespace ifileproc
{


// reimplemented (ifileproc::IFileConversion)

int CFileInfoCopyComp::ConvertFiles(
			const QString& inputPath,
			QString& outputPath,
			const iprm::IParamsSet* /*paramsSetPtr*/,
			ibase::IProgressManager* /*progressManagerPtr*/,
			istd::IChangeable* /*processingReportPtr*/) const
{
	QString inputFileName = inputPath;
	QString outputFileName = outputPath;

	SendInfoMessage(MI_FILE_INFO, QObject::tr("Processing file %1 to %2").arg(inputFileName).arg(outputFileName));

	if (!m_substitutionTagExprAttrPtr.IsValid() && !m_licensePathAttrPtr.IsValid()){
		QFile::remove(outputFileName);

		return QFile::copy(inputFileName, outputFileName) ? iproc::IProcessor::TS_OK : iproc::IProcessor::TS_INVALID;
	}

	QFile inputFile(inputFileName);
	QFile outputFile(outputFileName);

	QDir inputDir = QFileInfo(inputFileName).absoluteDir();
	QDir outputDir = QFileInfo(inputFileName).absoluteDir();

	if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)){
		SendWarningMessage(MI_INPUT_OPEN, QObject::tr("Opening input file failed (%1)").arg(QFileInfo(inputFileName).absoluteFilePath()));

		return iproc::IProcessor::TS_INVALID;
	}

	if (!outputFile.open(QIODevice::WriteOnly  | QIODevice::Text)){
		SendWarningMessage(MI_OUTPUT_OPEN, QObject::tr("Opening output file failed (%1)").arg(QFileInfo(outputFileName).absoluteFilePath()));

		return iproc::IProcessor::TS_INVALID;
	}

	QTextStream inputStream(&inputFile);
	QTextStream outputStream(&outputFile);

	if (m_licensePathAttrPtr.IsValid()){
		QString licenseFileName = istd::CSystem::GetEnrolledPath(*m_licensePathAttrPtr);

		QFile licenseFile(licenseFileName);
		if (!licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)){
			SendWarningMessage(MI_LICENSE_OPEN, QObject::tr("Opening license file failed (%1)").arg(licenseFileName));

			return iproc::IProcessor::TS_INVALID;
		}

		QTextStream licenseStream(&licenseFile);

		while (!licenseStream.atEnd()){
			QString line = licenseStream.readLine();
			outputStream << line << QT_ENDL;
		}
	}

#if QT_VERSION < 0x060000
	QRegExp tagsExpression;
	if (m_substitutionTagExprAttrPtr.IsValid()){
		tagsExpression = QRegExp(*m_substitutionTagExprAttrPtr);
		tagsExpression.setMinimal(true);
	}
#else
	QRegularExpression tagsExpression;
	if (m_substitutionTagExprAttrPtr.IsValid()){
		tagsExpression = QRegularExpression(*m_substitutionTagExprAttrPtr, QRegularExpression::InvertedGreedinessOption);
	}
#endif

	for (int lineCounter = 1; !inputStream.atEnd(); ++lineCounter){
		QString line = inputStream.readLine();

		if (m_substitutionTagExprAttrPtr.IsValid()){
			QString outputLine;
			int pos = 0;
			int endPos = 0;
#if QT_VERSION < 0x060000
			while ((pos = tagsExpression.indexIn(line, endPos)) != -1){
				outputLine += line.mid(endPos, pos - endPos);

				endPos = pos + tagsExpression.matchedLength();

				QString substitutionTag = tagsExpression.cap(1);
#else
			QRegularExpressionMatch match;

			while ((pos = line.indexOf(tagsExpression, endPos, &match)) != -1){
				outputLine += line.mid(endPos, pos - endPos);

				endPos = pos + match.capturedLength();

				QString substitutionTag = match.captured(1);
#endif

				if (!substitutionTag.isEmpty()){
					QString substituted;
					if (ProcessSubstitutionTag(inputDir, outputDir, substitutionTag, substituted)){
						outputLine += substituted;
					}
					else{
						if (!*m_ignoreUnknownTagsAttrPtr){
							SendWarningMessage(MI_BAD_TAG, QObject::tr("%1(%2) : Cannot process tag '%3'").arg(inputFileName).arg(lineCounter).arg(substitutionTag));
						}

						outputLine += line.mid(pos, endPos - pos);
					}
				}
				else{
					outputLine += line.mid(pos, endPos - pos);
				}
			}

			outputLine += line.mid(endPos);

			line = outputLine;
		}

		outputStream << line << QT_ENDL;
	}

	return iproc::IProcessor::TS_OK;
}


// protected methods

bool CFileInfoCopyComp::ProcessSubstitutionTag(
			const QDir& inputDir,
			const QDir& outputDir,
			const QString& tag,
			QString& result) const
{
	static const QString acfCompanyNameTag("AcfCompanyName");
	static const QString acfProductNameTag("AcfProductName");
	static const QString acfApplicationNameTag("AcfApplicationName");
	static const QString acfApplicationSubnameTag("AcfApplicationSubname");
	static const QString acfApplicationTypeTag("AcfApplicationType");
	static const QString acfLegalCopyrightTag("AcfLegalCopyright");
	static const QString acfVersionTag("AcfVersion");
	static const QString acfRawVersionTag("AcfRawVersion");
	static const QString acfRcVersionTag("AcfRcVersion");
	static const QString acfTimestampTag("AcfTimestamp");

	Q_ASSERT(!tag.isEmpty());

	int separatorIndex = tag.indexOf(":");

	if (separatorIndex >= 0){
		QString paramTag = tag.left(separatorIndex);
		QString parameter = tag.right(tag.length() - separatorIndex - 1);

		if ((paramTag == acfVersionTag) || (paramTag == acfRawVersionTag) || (paramTag == acfRcVersionTag)){
			if (m_applicationInfoCompPtr.IsValid()){
				bool isNumOk;
				int versionId = parameter.toInt(&isNumOk);

				quint32 versionNumber;
				const iser::IVersionInfo& versionInfo = m_applicationInfoCompPtr->GetVersionInfo();
				if (isNumOk && (versionId >= 0) && versionInfo.GetVersionNumber(versionId, versionNumber)){
					if (paramTag == acfRawVersionTag){
						result = QString::number(versionNumber);
						return true;
					}
					else{
						result = versionInfo.GetEncodedVersionName(versionId, versionNumber);
						if (paramTag == acfVersionTag){
							return true;
						}
						else if (paramTag == acfRcVersionTag){
							QStringList acfRcVersionParts = result.split(".");

							for (QString& acfRcVersionPart: acfRcVersionParts){
								for(int charIndex = 0; charIndex < acfRcVersionPart.length(); ++ charIndex){
									if(!acfRcVersionPart[charIndex].isDigit()){
										acfRcVersionPart.resize(charIndex);

										break;
									}
								}
							}
							acfRcVersionParts.removeAll("");
							result = acfRcVersionParts.join(", ");
							result.replace("'", "");

							return true;
						}
					}
				}
			}
			else{
				SendWarningMessage(0, "Application information component was not registered. Version information cannot be applied", "File Transformation");
			}
		}
	}
	else{
		if (m_applicationInfoCompPtr.IsValid()){
			if (tag == acfCompanyNameTag){
				result = m_applicationInfoCompPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_COMPANY_NAME);

				return true;
			}
			else if (tag == acfProductNameTag){
				result = m_applicationInfoCompPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_PRODUCT_NAME);

				return true;
			}
			else if (tag == acfApplicationNameTag){
				result = m_applicationInfoCompPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_APPLICATION_NAME);

				return true;
			}
			else if (tag == acfApplicationSubnameTag){
				result = m_applicationInfoCompPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_APPLICATION_SUBNAME);

				return true;
			}
			else if (tag == acfApplicationTypeTag){
				result = m_applicationInfoCompPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_APPLICATION_TYPE);

				return true;
			}
			else if (tag == acfLegalCopyrightTag){
				result = m_applicationInfoCompPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_LEGAL_COPYRIGHT);

				return true;
			}
			else if ((tag == acfVersionTag) || (tag == acfRawVersionTag) || (tag == acfRcVersionTag)){
				quint32 versionNumber;

				const iser::IVersionInfo& versionInfo = m_applicationInfoCompPtr->GetVersionInfo();
				int versionId = m_applicationInfoCompPtr->GetMainVersionId();
				if ((versionId >= 0) && versionInfo.GetVersionNumber(versionId, versionNumber)){
					if (tag == acfRawVersionTag){
						result = QString::number(versionNumber);
						return true;
					}
					else{
						result = versionInfo.GetEncodedVersionName(versionId, versionNumber);
						if (tag == acfVersionTag){
							return true;
						}
						else if (tag == acfRcVersionTag){
							result.replace(".", ", ");
							result.replace("'", "");

							return true;
						}
					}
				}
			}
		}

		if (tag == acfTimestampTag){
			QDateTime currentTime = QDateTime::currentDateTime();
			result = currentTime.toString();

			return true;
		}

		int userTagsCount = qMin(m_userSubstitutionTagsAttrPtr.GetCount(), m_userSubstitutionValuesAttrPtr.GetCount());
		for (int userTagIndex = 0; userTagIndex < userTagsCount; ++userTagIndex){
			if (tag == m_userSubstitutionTagsAttrPtr[userTagIndex]){
				QString replacedText = m_userSubstitutionValuesAttrPtr[userTagIndex];

				static const QString relativeInputPrefix = "$RelativeToInputFile:";
				static const QString relativeOutputPrefix = "$RelativeToOutputFile:";
				static const QString absoluteInputPrefix = "$AbsoluteToInputFile:";
				static const QString absoluteOutputPrefix = "$AbsoluteToInputFile:";
				static const QString relativeInputOsPrefix = "$RelativeToInputFileOS:";
				static const QString relativeOutputOsPrefix = "$RelativeToOutputFileOS:";
				static const QString absoluteInputOsPrefix = "$AbsoluteToInputFileOS:";
				static const QString absoluteOutputOsPrefix = "$AbsoluteToInputFileOS:";
				static const QString relativeInputWinPrefix = "$RelativeToInputFileWin:";
				static const QString relativeOutputWinPrefix = "$RelativeToOutputFileWin:";
				static const QString absoluteInputWinPrefix = "$AbsoluteToInputFileWin:";
				static const QString absoluteOutputWinPrefix = "$AbsoluteToInputFileWin:";

				if (replacedText.startsWith(relativeInputPrefix)){
					replacedText = inputDir.relativeFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(relativeInputPrefix.size())));
				}
				else if (replacedText.startsWith(relativeOutputPrefix)){
					replacedText = outputDir.relativeFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(relativeOutputPrefix.size())));
				}
				else if (replacedText.startsWith(absoluteInputPrefix)){
					replacedText = inputDir.absoluteFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(absoluteInputPrefix.size())));
				}
				else if (replacedText.startsWith(absoluteOutputPrefix)){
					replacedText = outputDir.absoluteFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(absoluteOutputPrefix.size())));
				}
				else if (replacedText.startsWith(relativeInputOsPrefix)){
					replacedText = QDir::toNativeSeparators(inputDir.relativeFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(relativeInputOsPrefix.size()))));
				}
				else if (replacedText.startsWith(relativeOutputOsPrefix)){
					replacedText = QDir::toNativeSeparators(outputDir.relativeFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(relativeOutputOsPrefix.size()))));
				}
				else if (replacedText.startsWith(absoluteInputOsPrefix)){
					replacedText = QDir::toNativeSeparators(inputDir.absoluteFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(absoluteInputOsPrefix.size()))));
				}
				else if (replacedText.startsWith(absoluteOutputOsPrefix)){
					replacedText = QDir::toNativeSeparators(outputDir.absoluteFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(absoluteOutputOsPrefix.size()))));
				}
				else if (replacedText.startsWith(relativeInputWinPrefix)){
					replacedText = inputDir.relativeFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(relativeInputWinPrefix.size()))).replace('/', '\\');
				}
				else if (replacedText.startsWith(relativeOutputWinPrefix)){
					replacedText = outputDir.relativeFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(relativeOutputWinPrefix.size()))).replace('/', '\\');
				}
				else if (replacedText.startsWith(absoluteInputWinPrefix)){
					replacedText = inputDir.absoluteFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(absoluteInputWinPrefix.size()))).replace('/', '\\');
				}
				else if (replacedText.startsWith(absoluteOutputWinPrefix)){
					replacedText = outputDir.absoluteFilePath(istd::CSystem::GetEnrolledPath(replacedText.mid(absoluteOutputWinPrefix.size()))).replace('/', '\\');
				}

				result = replacedText;

				return true;
			}
		}

		if (*m_replaceEnvironmentVariablesAttrPtr || *m_replaceEmbeddedVariablesAttrPtr){
			QString variable = istd::CSystem::GetVariableValue(tag, *m_replaceEnvironmentVariablesAttrPtr, *m_replaceEmbeddedVariablesAttrPtr);
			if (!variable.isEmpty()){
				result = variable;

				return true;
			}
		}
	}

	return false;
}


} // namespace ifileproc


