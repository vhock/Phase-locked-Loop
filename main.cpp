#include "mainwindow.h"

#include <QApplication>
#include <libssh/libssh.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    ssh_session rp_session = ssh_new();
    return a.exec();


}
