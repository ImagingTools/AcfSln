// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ACF-Commercial
// Qt includes
#include <QtCore/QCoreApplication>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

// ACF includes
#include <ibase/IApplication.h>

#include <GeneratedFiles/Compositor/CCompositor.h>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(iqtgui);
	Q_INIT_RESOURCE(icmpstr);
	Q_INIT_RESOURCE(AcfLoc);
	Q_INIT_RESOURCE(AcfSlnLoc);
	Q_INIT_RESOURCE(Compositor);

	QApplication::addLibraryPath("./");

#ifdef Q_OS_WIN
	QApplication::setStyle("fusion");
	qputenv("QT_QPA_PLATFORM", "windows:darkmode=0");
#endif
	CCompositor instance(NULL, true);

	QGuiApplication::setDesktopSettingsAware(false);

	ibase::IApplication* applicationPtr = instance.GetInterface<ibase::IApplication>();
	if (applicationPtr != NULL){
		applicationPtr->InitializeApplication(argc, argv);

		instance.EnsureAutoInitComponentsCreated();

		return applicationPtr->Execute(argc, argv);
	}

	return -1;
}


