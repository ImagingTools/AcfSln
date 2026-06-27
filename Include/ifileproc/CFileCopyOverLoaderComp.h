// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef ifileproc_CFileCopyOverLoaderComp_included
#define ifileproc_CFileCopyOverLoaderComp_included


// ACF includes
#include <istd/IChangeable.h>
#include <ifile/IFilePersistence.h>
#include <ilog/TLoggerCompWrap.h>

// ACF-Solutions includes
#include <ifileproc/IFileConversion.h>


namespace ifileproc
{


/**
	General file copy or transformation.
	This implementation uses general file loader, saver and any object instance.
*/
class CFileCopyOverLoaderComp:
			public ilog::CLoggerComponentBase,
			virtual public IFileConversion
{
public:
	typedef ilog::CLoggerComponentBase BaseClass;

	I_BEGIN_COMPONENT(CFileCopyOverLoaderComp);
		I_REGISTER_INTERFACE(IFileConversion);

		I_ASSIGN(m_objectCompPtr, "Object", "Object used as representation of copied data", true, "Object");
		I_ASSIGN(m_inputLoaderCompPtr, "InputLoader", "input file loader", true, "InputLoader");
		I_ASSIGN(m_outputLoaderCompPtr, "OutputLoader", "output file loader", true, "OutputLoader");
	I_END_COMPONENT;

	// reimplemented (ifileproc::IFileConversion)
	virtual int ConvertFiles(
				const QString& inputPath,
				QString& outputPath,
				const iprm::IParamsSet* paramsPtr = NULL,
				ibase::IProgressManager* progressManagerPtr = NULL,
				istd::IChangeable* processingReportPtr = NULL) const;

private:
	I_REF(istd::IChangeable, m_objectCompPtr);
	I_REF(ifile::IFilePersistence, m_inputLoaderCompPtr);
	I_REF(ifile::IFilePersistence, m_outputLoaderCompPtr);
};


} // namespace ifileproc


#endif // !ifileproc_CFileCopyOverLoaderComp_included


