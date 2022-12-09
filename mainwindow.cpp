#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "rputility.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),rpUtility()
{
    ui->setupUi(this);
    qRegisterMetaType<std::string>();
   // qRegisterMetaType<int>();


    QObject::connect(&rpUtility,&RPUtility::new_message,this,&MainWindow::logMessages,Qt::AutoConnection);
    QObject::connect(&rpUtility,&RPUtility::connectionStateChanged,this,&MainWindow::connectionStateChangedListener,Qt::AutoConnection);

    connectionIndicatorScene = new QGraphicsScene(this);

    ui->connectionIndicator->setScene(connectionIndicatorScene);
    ui->connectionIndicator->show();
    connectionIndicatorScene->setBackgroundBrush(Qt::red); //offline
    connectionIndicatorScene->addText("Offline");
    // a blue background

    ui->ipAddress->setText("192.168.20.203"); //TODO for debugging, delete later

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_connectButton_clicked()
{
std::string ipAddressText= ui->ipAddress->toPlainText().toStdString();
bool isValidIP=RPUtility::isValidIPAddress(ipAddressText);
if (!isValidIP){
    logMessages("Invalid IP");
    return;
}else {
    logMessages("Valid IP");
    auto currID= std::this_thread::get_id();


    std::thread connectionThread(&RPUtility::connect,&rpUtility,ipAddressText);
    connectionThread.detach();
    logMessages("detached");
    //Sleep(30000);
    //rpUtility.disconnect();
  //int connectionSuccessful= rpUtility.connect(text);
    logMessages("main thread continues");

  int x=4;

}
}
//slots
void MainWindow::logMessages(std::string message){
   QString qstring= QString::fromStdString(message);
     ui->statusBox->setText(qstring);
     ui->logBox->append(qstring);
};

void  MainWindow::connectionStateChangedListener(int code){
    if (code==1){
        connectionIndicatorScene->clear();//i do not want to bother with finding the text for now
        connectionIndicatorScene->setBackgroundBrush(Qt::green); //online
        connectionIndicatorScene->addText("Online");
    }
    if (code==0){
        connectionIndicatorScene->clear();//i do not want to bother with finding the text for now
        connectionIndicatorScene->setBackgroundBrush(Qt::red); //online
        connectionIndicatorScene->addText("Offline");
    }


}








void MainWindow::on_disconnectButton_clicked()
{
    rpUtility.disconnect();
}

