#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "rputility.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),rpUtility()
{
    ui->setupUi(this);
   int success= qRegisterMetaType<std::string>();
    QObject::connect(&rpUtility,&RPUtility::new_message,this,&MainWindow::logMessages,Qt::AutoConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_connectButton_clicked()
{
std::string text= ui->ipAddress->toPlainText().toStdString();
bool isValidIP=RPUtility::isValidIPAddress(text);
if (!isValidIP){
    logMessages("Invalid IP");
    return;
}else {
    logMessages("Valid IP");
    auto currID= std::this_thread::get_id();


    std::thread connectionThread(&RPUtility::connect,&rpUtility,text);
    connectionThread.detach();
    // blocking fucker std::future<int> connectValue=std::async(&RPUtility::connect,&rpUtility,text);
    logMessages("detached");

  //int connectionSuccessful= rpUtility.connect(text);
    logMessages("main thread continues");

  int x=4;

}
}

void MainWindow::logMessages(std::string message){
   QString qstring= QString::fromStdString(message);
     ui->statusBox->setText(qstring);
     ui->logBox->append(qstring);
};








