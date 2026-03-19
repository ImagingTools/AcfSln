// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// Qt includes
#include <QtCore/QThread>

// ACF includes
#include <iproc/IProcessor.h>


namespace ibase
{
class IProgressManager;
}

namespace iprm
{
class IParamsSet;
}

namespace istd
{
class IChangeable;
class IPolymorphic;
}


namespace iprocgui
{


class CDocumentProcessingWorkerThread: public QThread
{
public:
	CDocumentProcessingWorkerThread(
				iproc::IProcessor* processorPtr,
				const iprm::IParamsSet* paramsSetPtr,
				const istd::IPolymorphic* inputPtr,
				istd::IChangeable* outputPtr,
				ibase::IProgressManager* progressPtr);

	int GetResultCode() const;
	double GetProcessingTime() const;

protected:
	// reimplemented (QThread)
	void run() override;

private:
	iproc::IProcessor* m_processorPtr;
	const iprm::IParamsSet* m_paramsSetPtr;
	const istd::IPolymorphic* m_inputPtr;
	istd::IChangeable* m_outputPtr;
	ibase::IProgressManager* m_progressPtr;

	int m_resultCode;
	double m_processingTime;
};


} // namespace iprocgui


