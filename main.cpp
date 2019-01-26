#include "mainwindow.h"
#include <QApplication>

using namespace std;

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/img/search.png"));
    MainWindow w;

    w.show();

   return a.exec();
}
