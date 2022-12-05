#include "mainwindow.h"
#include "rputility.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_sshConnectButton_clicked()
{
  auto text= ui->ipAddress->toPlainText().toStdString();
}

