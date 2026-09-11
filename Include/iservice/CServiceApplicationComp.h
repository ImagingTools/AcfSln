// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
#ifndef iservice_CServiceApplicationComp_included
#define iservice_CServiceApplicationComp_included


// Qt includes
#include <QtGui/QIcon>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QSystemTrayIcon>
#else
#include <QtGui/QSystemTrayIcon>
#endif

// ACF includes
#include <ibase/IApplication.h>
#include <ilog/TLoggerCompWrap.h>
#include <icomp/CComponentBase.h>

#ifndef Q_OS_MAC
#ifndef Q_OS_LINUX
// ACF-Solutions includes
#include <iservice/QtService.h>
#endif
#endif


namespace iservice
{


/**
	Qt based component for service-based application.
*/
class CServiceApplicationComp:
			public QObject,
			public ilog::CLoggerComponentBase,
			public ibase::IApplication
{
	Q_OBJECT
public:
	typedef ilog::CLoggerComponentBase BaseClass;
	typedef QObject BaseClass2;

	/**
		Recovery action taken by the Windows SCM when the service fails.
		Values deliberately match the Win32 SC_ACTION_TYPE constants (winsvc.h)
		so no lookup table is needed on the Windows side.
	*/
	enum ServiceFailureAction
	{
		SFA_None = 0,        // SC_ACTION_NONE
		SFA_Restart = 1,     // SC_ACTION_RESTART
		SFA_Reboot = 2,      // SC_ACTION_REBOOT
		SFA_RunCommand = 3   // SC_ACTION_RUN_COMMAND
	};

	I_BEGIN_COMPONENT(CServiceApplicationComp);
		I_REGISTER_INTERFACE(ibase::IApplication);
		I_ASSIGN(m_applicationCompPtr, "ApplicationInstance", "Service application object", true, "Application");
		I_ASSIGN(m_serviceDescriptionAttrPtr, "SeriviceDescription", "Service description", true, "This services provides...");
		I_ASSIGN(m_serviceNameAttrPtr, "ServiceName", "The name of the service", true, "MyService");
		I_ASSIGN(m_manualStartupAttrPtr, "ManualStartup", "If enabled, the service is registered with manual start up", false, false);
		I_ASSIGN(m_failureResetPeriodMinutesAttrPtr, "FailureResetPeriodMinutes", "Minutes after which the service failure counter is reset", false, 10);
		I_ASSIGN_MULTI_3(m_failureActionTypesAttrPtr, "FailureActionTypes", "Recovery action per failure (0=None, 1=Restart, 2=Reboot, 3=RunCommand)", false, int(SFA_Restart), int(SFA_Restart), int(SFA_Restart));
		I_ASSIGN_MULTI_3(m_failureRetryDelaysSecAttrPtr, "FailureRetryDelaysSeconds", "Delay in seconds before the recovery action for the 1st, 2nd and 3rd+ failures", false, 3, 8, 30);
	I_END_COMPONENT;

	QStringList GetApplicationArguments(int argc, char** argv) const;

	// reimplemented (ibase::IApplication)
	virtual bool InitializeApplication(int argc, char** argv);
	virtual int Execute(int argc, char** argv);
	virtual QString GetHelpText() const;
	virtual QStringList GetApplicationArguments() const;

	// reimplemented (QObject)
	virtual bool eventFilter(QObject* sourcePtr, QEvent* eventPtr);

protected Q_SLOTS:
	void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

#ifdef Q_OS_WIN

protected:
	class CService: public QtServiceBase
	{
	public:
		typedef QtServiceBase BaseClass;

		CService(CServiceApplicationComp& parent,
					ibase::IApplication& application,
					int serviceArgc,
					char** serviceArgv,
					const QString &name);

	protected:
		// reimplemented (QtServiceBase)
		virtual void start();
		virtual void stop();
		virtual void pause();
		virtual void resume();
		virtual void createApplication(int &argc, char **argv);
		virtual int executeApplication();

	private:
		QVector<char*> GetApplicationArguments() const;

	private:
		CServiceApplicationComp& m_parent;
		ibase::IApplication& m_application;
		QVector<QByteArray> m_applicationArguments;
	};

	istd::TDelPtr<CService> m_servicePtr;
#endif // Q_OS_WIN

private:
	I_REF(ibase::IApplication, m_applicationCompPtr);
	I_TEXTATTR(m_serviceDescriptionAttrPtr);
	I_ATTR(QString, m_serviceNameAttrPtr);
	I_ATTR(bool, m_manualStartupAttrPtr);
	I_ATTR(int, m_failureResetPeriodMinutesAttrPtr);
	I_MULTIATTR(int, m_failureActionTypesAttrPtr);
	I_MULTIATTR(int, m_failureRetryDelaysSecAttrPtr);
};


} // namespace iservice


#endif // iservice_CServiceApplicationComp_included

