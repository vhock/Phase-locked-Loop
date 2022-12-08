#include "mainwindow.h"

#include <QApplication>
#include<libssh/libssh.h>
#include<thread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
    ssh_session sesh;
}
