// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#pragma once


// Qt includes
#include <QtCore/QByteArray>
#include <QtCore/QThread>


namespace ibase
{
class IProgressManager;
}

namespace istd
{
class IChangeable;
}


namespace iprocgui
{


class CDocumentProcessingManagerCompBase;


class CDocumentProcessingWorkerThread: public QThread
{
public:
	CDocumentProcessingWorkerThread(
				CDocumentProcessingManagerCompBase* managerPtr,
				const istd::IChangeable* inputDocumentPtr,
				const QByteArray& documentTypeId,
				ibase::IProgressManager* progressPtr);

protected:
	// reimplemented (QThread)
	void run() override;

private:
	CDocumentProcessingManagerCompBase* m_managerPtr;
	const istd::IChangeable* m_inputDocumentPtr;
	QByteArray m_documentTypeId;
	ibase::IProgressManager* m_progressPtr;
};


} // namespace iprocgui


