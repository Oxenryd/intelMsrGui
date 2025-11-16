#include <unistd.h>

#include <QMenu>
#include <QApplication>
#include <QMessageBox>
#include <QSystemTrayIcon>

#include "MainWindow.h"



bool isRoot() { return geteuid() == 0; }

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("intelMsrGui");
    QApplication::setQuitOnLastWindowClosed(false);
    const QIcon icon{":icon/icon.png"};
    a.setWindowIcon(icon);

    // if (!isRoot()) {
    //     QMessageBox::critical(nullptr,
    //                           "IntelMsrGui",
    //                           "This tool needs proper MSR permissions.\n"
    //                           "Please run it via sudo/pkexec or set up permissions.");
    //     return 1;
    // }
    bool startHidden = true;
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--show") == 0) {
            startHidden = false;
        }
    }

    MainWindow w;
    w.hide();
    if (!startHidden)
        w.show();

    auto *tray = new QSystemTrayIcon(icon, &a);

    auto *menu = new QMenu();

    // We'll fill presets later
    QMenu *presetsMenu = menu->addMenu("Presets");

    QAction *showAction = menu->addAction("Show");
    QObject::connect(showAction, &QAction::triggered, &w, [&] {
        w.show();
        w.raise();
        w.activateWindow();
    });

    QAction *quitAction = menu->addAction("Quit");
    QObject::connect(quitAction, &QAction::triggered, &a, &QCoreApplication::quit);

    tray->setContextMenu(menu);
    tray->setToolTip("IntelMsrGui");
    tray->show();


    QObject::connect(tray, &QSystemTrayIcon::activated,
                     &w, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            if (w.isVisible())
                w.hide();
            else {
                w.show();
                w.raise();
                w.activateWindow();
            }
        }
    });

    QObject::connect(presetsMenu, &QMenu::aboutToShow,
                 &w, [&w, presetsMenu]() {
            w.updateTrayMenu(presetsMenu);
    });

    return QApplication::exec();
}
