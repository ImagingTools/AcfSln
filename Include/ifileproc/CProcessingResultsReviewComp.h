// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ifileproc_CProcessingResultsReviewComp_included
#define ifileproc_CProcessingResultsReviewComp_included


// ACF includes
#include <ifile/IFileTypeInfo.h>
#include <ifile/IFileNameParam.h>
#include <ifile/IFilePersistence.h>
#include <ifileproc/IFileConversion.h>
#include <ilog/TLoggerCompWrap.h>

// ACF-Solutions includes
#include <iinsp/ISupplier.h>


namespace ifileproc
{


/**
	Component for creating single file with results of processing single input file or all files in some directory.
	It needs some input object storing current processed file path and supplier chain based on this path.
	The results will be stored in single serialization process.
*/
class CProcessingResultsReviewComp:
			public ilog::CLoggerComponentBase,
			virtual public ifileproc::IFileConversion
{
public:
	typedef ilog::CLoggerComponentBase BaseClass;

	I_BEGIN_COMPONENT(CProcessingResultsReviewComp);
		I_REGISTER_INTERFACE(ifileproc::IFileConversion);
		I_ASSIGN(m_inputPathCompPtr, "InputPathObject", "Optional storage of current processed input (file or directory)", false, "InputPathObject");
		I_ASSIGN(m_currentProcessedFilePathCompPtr, "CurrentProcessedFilePath", "Full path to currently processed file, should be used by supplier as input file path", true, "ProcessingInputFilePath");
		I_ASSIGN(m_outputSupplierCompPtr, "OutputSupplier", "Supplier to process files", true, "OutputSupplier");
		I_ASSIGN_TO(m_outputSupplierSerializerCompPtr, m_outputSupplierCompPtr, true);
		I_ASSIGN(m_outputFileSerializerCompPtr, "OutputFileSerializer", "Output formatter", true, "OutputFileSerializer");
		I_ASSIGN(m_inputFileTypeInfoCompPtr, "InputFilesTypeInfo", "Optional input files type information used if user specified directory", false, "InputFilesTypeInfo");
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
		Process single file and send the results to archive.
	*/
	virtual bool ProcessSingleFile(const QString& filePath, iser::IArchive& archive);

private:
	class ProcessSerializer: public iser::ISerializable
	{
	public:
		/**
			Initialize the serializer with parent object and path to be processed.
		*/
		ProcessSerializer(
					const CProcessingResultsReviewComp* parentPtr,
					const QString& path,
					std::unique_ptr<ibase::IProgressLogger>&& progressLoggerPtr);

		// reimplemented (iser::ISerializable)
		virtual bool Serialize(iser::IArchive& archive) override;

		bool isCanceled;

	private:
		const CProcessingResultsReviewComp& m_parent;
		QString m_path;
		std::unique_ptr<ibase::IProgressLogger> m_progressLoggerPtr;
	};

	I_REF(ifile::IFileNameParam, m_inputPathCompPtr);
	I_REF(ifile::IFileNameParam, m_currentProcessedFilePathCompPtr);
	I_REF(iinsp::ISupplier, m_outputSupplierCompPtr);
	I_REF(iser::ISerializable, m_outputSupplierSerializerCompPtr);
	I_REF(ifile::IFilePersistence, m_outputFileSerializerCompPtr);
	I_REF(ifile::IFileTypeInfo, m_inputFileTypeInfoCompPtr);
};


} // namespace ifileproc


#endif // !ifileproc_CProcessingResultsReviewComp_included


