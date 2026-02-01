#include <unistd.h>

#include <QMenu>
#include <QApplication>
#include <QMessageBox>
#include <QSystemTrayIcon>

#include "MainWindow.h"

//#define DEBUG

bool isRoot() { return geteuid() == 0; }

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("intelMsrGui");
    QApplication::setQuitOnLastWindowClosed(false);
    const QIcon icon{":icon/icon.png"};
    QApplication::setWindowIcon(icon);

    bool startHidden = true;

#ifdef DEBUG
    startHidden = false;
#else

    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--show") == 0) {
            startHidden = false;
        }
    }
#endif

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
