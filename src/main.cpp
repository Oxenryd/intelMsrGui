#include <QApplication>
#include <QPushButton>

#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("intelMsrGui");
    a.setWindowIcon(QIcon(":icon.png"));
    MainWindow main;
    main.show();
    return QApplication::exec();
}
