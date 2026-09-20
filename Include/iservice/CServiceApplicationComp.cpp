// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#include <iservice/CServiceApplicationComp.h>


// Qt includes
#include <QtCore/QVector>
#if QT_VERSION < 0x050000
#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QSystemTrayIcon>
#else
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QSystemTrayIcon>
#endif

// ACF includes
#include <iqtgui/IGuiApplication.h>


namespace iservice
{


static int s_argc = 0;
static QVector<char*> s_argv = QVector<char*>();


// public methods

QStringList CServiceApplicationComp::GetApplicationArguments(int argc, char** argv) const
{
	QStringList applicationArguments;

	for (int argIndex = 0; argIndex < argc; argIndex++){
		applicationArguments.push_back(QString().fromLocal8Bit(argv[argIndex]));
	}
	
	return applicationArguments;
}


// reimplemented (ibase::IApplication)

bool CServiceApplicationComp::InitializeApplication(int/* argc*/, char** /* argv*/)
{
	return true;
}


int CServiceApplicationComp::Execute(int argc, char** argv)
{
#ifndef Q_OS_WIN
	if (m_applicationCompPtr.IsValid()){
		return m_applicationCompPtr->Execute(argc, argv);
	}
#else
	Q_ASSERT(m_serviceNameAttrPtr.IsValid());
	Q_ASSERT(m_serviceDescriptionAttrPtr.IsValid());

	if (!m_serviceDescriptionAttrPtr.IsValid() || !m_serviceNameAttrPtr.IsValid()){
		return -1;
	}

	QString serviceName = *m_serviceNameAttrPtr;

	if (m_applicationCompPtr.IsValid()){
		// check whether the service already installed, if not, then install it:
		QtServiceController controller(serviceName);
		if (!controller.isInstalled()){
			// for installation we need 3 additional arguments: -i for install, account and password (last both we leave always empty)
			// standard Qt mechanisms don't allow definition of a service with command line parameters, so we need some workaround:
			QVector<const char*> instalArguments(argc + 3);
			QList<QByteArray> argvData;
			for (int i = 0; i < argc; ++i){
				argvData.append(argv[i]);
			}

			// copy first argument:
			instalArguments[0] = argv[0];

			// insert install arguments needed by Qt for service creation:
			instalArguments[1] = "-i";
			instalArguments[2] = "";
			instalArguments[3] = "";
			
			// append user defined application's arguments:
			for (int i = 4; i < argc + 3; ++i){
				instalArguments[i] = argvData[i - 3].data();
			}

			m_servicePtr.SetPtr(new CService(
						*this,
						*m_applicationCompPtr.GetPtr(),
						argc + 3,
						(char**)instalArguments.data(),
						serviceName));
			
			m_servicePtr->setServiceDescription(*m_serviceDescriptionAttrPtr);

			if (m_failureResetPeriodMinutesAttrPtr.IsValid() && m_failureActionTypesAttrPtr.IsValid() && m_failureRetryDelaysSecAttrPtr.IsValid()) {
				// Configure the Windows service failure/recovery policy from the
				// component attributes. The reset period is entered in minutes; the
				// per-failure recovery actions and their delays are entered as two
				// parallel lists (1st, 2nd, 3rd-and-subsequent failures). Convert to
				// the units expected by the SCM: seconds for the reset period and
				// milliseconds for each action delay.
				const int failureResetSec = std::max(0, (*m_failureResetPeriodMinutesAttrPtr)) * 60; // minutes -> seconds

				QList<QtServiceBase::FailureAction> failureActions;
				const int actionTypeCount = m_failureActionTypesAttrPtr.GetCount();
				for (int failureIndex = 0; failureIndex < m_failureRetryDelaysSecAttrPtr.GetCount(); ++failureIndex){
					QtServiceBase::FailureAction action;
					// Reuse the last action type if fewer types than delays were configured.
					const int typeIndex = (failureIndex < actionTypeCount) ? failureIndex : (actionTypeCount - 1);
					action.actionType = (actionTypeCount > 0) ? m_failureActionTypesAttrPtr[typeIndex] : int(SFA_Restart);
					action.delayMs = std::max(0, m_failureRetryDelaysSecAttrPtr[failureIndex]) * 1000; // seconds -> milliseconds
					failureActions.append(action);
				}
				m_servicePtr->setFailureActions(failureResetSec, failureActions);
			}

			QtServiceController::StartupType serviceStartUp = QtServiceController::AutoStartup;
			if (m_manualStartupAttrPtr.IsValid() && *m_manualStartupAttrPtr){
				serviceStartUp = QtServiceController::ManualStartup;
			}
			m_servicePtr->setStartupType(serviceStartUp);

			// start service installing:
			int retVal = m_servicePtr->exec();
			if (retVal == 0){
				QApplication* appPtr = dynamic_cast<QApplication*>(QCoreApplication::instance());
				QString installMessage = QString(QCoreApplication::tr("Service: %1 was succesfully installed.\nNow you can start the service from the Service Manager.")).arg(serviceName);
				if (appPtr != NULL){				
					QMessageBox::information(
								NULL,
								QCoreApplication::tr("ACF Service Application"), installMessage);
				}
				else{
					SendInfoMessage(0, installMessage);
				}

				// start the installed service:
				controller.start();
			}

			return retVal;
		}

		SendInfoMessage(0, QString("Application arguments: ") + GetApplicationArguments(argc, argv).join(" "));

		m_servicePtr.SetPtr(new CService(
					*this,
					*m_applicationCompPtr.GetPtr(),
					argc,
					argv,
					serviceName));

		return m_servicePtr->exec();
	}
#endif // Q_OS_WIN

	return -1;
}


QString CServiceApplicationComp::GetHelpText() const
{
	// TODO: Combine service and application related  usages.
	if (m_applicationCompPtr.IsValid()){
		return m_applicationCompPtr->GetHelpText();
	}

	return QString();
}


QStringList CServiceApplicationComp::GetApplicationArguments() const
{
	if (m_applicationCompPtr.IsValid()){
		return m_applicationCompPtr->GetApplicationArguments();
	}

	return QStringList();
}


// reimplemented (QObject)
	
bool CServiceApplicationComp::eventFilter(QObject* sourcePtr, QEvent* eventPtr)
{
#ifdef Q_OS_WIN
	Q_ASSERT(m_applicationCompPtr.IsValid());
	iqtgui::IGuiApplication* guiAppPtr = dynamic_cast<iqtgui::IGuiApplication*>(m_applicationCompPtr.GetPtr());
	if (guiAppPtr != NULL){
		const iqtgui::IGuiObject* mainWdigetPtr = guiAppPtr->GetApplicationGui();
		if (mainWdigetPtr != NULL && mainWdigetPtr->IsGuiCreated()){
			QWidget* widgetPtr = mainWdigetPtr->GetWidget();
			Q_ASSERT(widgetPtr != NULL);

			// prevent closing of the main widget:
			if ((widgetPtr == sourcePtr) && eventPtr->type() == QEvent::Close){
				eventPtr->ignore();

				widgetPtr->hide();

				return true;	
			}
		}
	}
#else
	return BaseClass2::eventFilter(sourcePtr, eventPtr);
#endif // Q_OS_WIN

	return false;
}


// protected slots

void CServiceApplicationComp::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	Q_ASSERT(m_applicationCompPtr.IsValid());

	if (reason == QSystemTrayIcon::DoubleClick){
		iqtgui::IGuiApplication* guiAppPtr = dynamic_cast<iqtgui::IGuiApplication*>(m_applicationCompPtr.GetPtr());
		if (guiAppPtr != NULL){
			const iqtgui::IGuiObject* mainWdigetPtr = guiAppPtr->GetApplicationGui();
			if (mainWdigetPtr != NULL && mainWdigetPtr->IsGuiCreated()){
				QWidget* widgetPtr = mainWdigetPtr->GetWidget();
				Q_ASSERT(widgetPtr != NULL);
				
				if (widgetPtr->isHidden()){
					widgetPtr->show();
				}
				else{
					widgetPtr->activateWindow();
				}
			}
		}	
	}
}


#ifdef Q_OS_WIN


// public methods of embedded class CService

CServiceApplicationComp::CService::CService(
			CServiceApplicationComp& parent,
			ibase::IApplication& application,
			int serviceArgc,
			char** serviceArgv,
			const QString &name)
	:QtServiceBase(serviceArgc, serviceArgv, name),
	m_parent(parent),
	m_application(application)
{
	 QStringList applicationArguments = m_parent.GetApplicationArguments(serviceArgc, serviceArgv);

	 for (int argIndex = 0; argIndex < applicationArguments.count(); ++argIndex){
		 m_applicationArguments.push_back(applicationArguments[argIndex].toLocal8Bit());
	 }

	setServiceFlags(QtServiceBase::CanBeSuspended);
}


// protected methods of embedded class CService

// reimplemented (QtServiceBase)

void CServiceApplicationComp::CService::start()
{
	iqtgui::IGuiApplication* guiAppPtr = dynamic_cast<iqtgui::IGuiApplication*>(&m_application);
	if (guiAppPtr != NULL){
		const iqtgui::IGuiObject* mainWdigetPtr = guiAppPtr->GetApplicationGui();
		if (mainWdigetPtr != NULL && mainWdigetPtr->IsGuiCreated()){
			QWidget* widgetPtr = mainWdigetPtr->GetWidget();
			Q_ASSERT(widgetPtr != NULL);
			QSystemTrayIcon* trayIcon = new QSystemTrayIcon(widgetPtr);
			connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), &m_parent, SLOT(OnTrayIconActivated(QSystemTrayIcon::ActivationReason)));

			trayIcon->setIcon(widgetPtr->windowIcon());

			trayIcon->setVisible(true);

			widgetPtr->installEventFilter(&m_parent);

			widgetPtr->hide();
		}
	}
}


void CServiceApplicationComp::CService::stop()
{
}


void CServiceApplicationComp::CService::pause()
{
	iqtgui::IGuiApplication* guiAppPtr = dynamic_cast<iqtgui::IGuiApplication*>(&m_application);
	if (guiAppPtr != NULL){
		const iqtgui::IGuiObject* mainWdigetPtr = guiAppPtr->GetApplicationGui();
		if (mainWdigetPtr != NULL && mainWdigetPtr->IsGuiCreated()){
			mainWdigetPtr->GetWidget()->hide();
		}
	}
}


void CServiceApplicationComp::CService::resume()
{
	iqtgui::IGuiApplication* guiAppPtr = dynamic_cast<iqtgui::IGuiApplication*>(&m_application);
	if (guiAppPtr != NULL){
		const iqtgui::IGuiObject* mainWdigetPtr = guiAppPtr->GetApplicationGui();
		if (mainWdigetPtr != NULL && mainWdigetPtr->IsGuiCreated()){
			mainWdigetPtr->GetWidget()->show();
		}
	}
}


void CServiceApplicationComp::CService::createApplication(int&/*argc*/, char** /* argv*/)
{
	s_argv = GetApplicationArguments();

	s_argc = s_argv.count();

	iqtgui::IGuiApplication* guiAppPtr = dynamic_cast<iqtgui::IGuiApplication*>(&m_application);
	if (guiAppPtr != NULL){
		new QApplication(s_argc, s_argv.data());
	}
	else{
		new QCoreApplication(s_argc, s_argv.data());
	}

	m_application.InitializeApplication(s_argc, s_argv.data());
}


int CServiceApplicationComp::CService::executeApplication()
{
	QVector<char*> argv = GetApplicationArguments();

	return m_application.Execute(argv.count(), argv.data());
}


// private methods of embedded class CService

QVector<char*> CServiceApplicationComp::CService::GetApplicationArguments() const
{
	QVector<char*> argv(m_applicationArguments.count());

	for (int argIndex = 0; argIndex < m_applicationArguments.count(); argIndex++){
		argv[argIndex] = const_cast<char*>(m_applicationArguments[argIndex].data());
	}

	return argv;
}

#endif // Q_OS_WIN


} // namespace iservice


