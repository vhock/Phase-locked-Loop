#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "rputility.h"
#include <QLabel>
#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),rpUtility()
{
    ui->setupUi(this);
    qRegisterMetaType<std::string>();
    // qRegisterMetaType<int>();


    QObject::connect(&rpUtility,&RPUtility::log_message,this,&MainWindow::logMessages,Qt::AutoConnection);
    QObject::connect(&rpUtility,&RPUtility::connectionStateChanged,this,&MainWindow::connectionStateChangedListener,Qt::AutoConnection);

    connectionIndicatorScene = new QGraphicsScene(this);

    ui->connectionIndicator->setScene(connectionIndicatorScene);
    ui->connectionIndicator->show();
    connectionIndicatorScene->setBackgroundBrush(Qt::red); //offline
    connectionIndicatorScene->addText("Offline");
    // a red background

    //QLabel * f0Label=new QLabel(ui->pll1_f0_box);

    ui->ipAddress->setText("192.168.20.188"); //TODO for debugging, delete later

    //connect UI parameter fields to the Red Pitaya utility
    ui->pll1_freq_box->setKeyboardTracking(false);
    ui->pll1_kp_box->setKeyboardTracking(false);
    ui->pll1_ki_box->setKeyboardTracking(false);
    QObject::connect(ui->pll1_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("f0",newValue,0);}
    );

    QObject::connect(ui->pll1_kp_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("kp",newValue,0);}
    );

    QObject::connect(ui->pll1_ki_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("ki",newValue,0);}
    );

    QObject::connect(ui->pll1_amplitude_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("a",newValue,0);}
    );
    //  QObject::connect(ui->pll1_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),&rpUtility,&RPUtility::pll1_f0_ChangedListener,Qt::AutoConnection);

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
        ui->disconnectButton->setEnabled(true);
        ui->connectButton->setEnabled(false);

    }
    if (code==0){
        connectionIndicatorScene->clear();//i do not want to bother with finding the text for now
        connectionIndicatorScene->setBackgroundBrush(Qt::red); //online
        connectionIndicatorScene->addText("Offline");
        ui->disconnectButton->setEnabled(false);
        ui->connectButton->setEnabled(true);
    }


}








void MainWindow::on_disconnectButton_clicked()
{
    rpUtility.disconnect();
}




void MainWindow::on_loadBitfileButton_clicked()
{
    rpUtility.scp_copyBitfile();
    rpUtility.executeBitfile();
}


void MainWindow::on_sendArbitraryCommandBtn_clicked()
{
    auto command= ui->commandBox->toPlainText().toStdString();
    std::string answer{};
    rpUtility.sendCommand(command,answer);
    logMessages("Command "+command+" returned: "+answer);

}


void MainWindow::on_pushButton_clicked()
{
 RPParameterConverter rpConv;
 rpConv.setParameter(0,"pid_en",1);
long ans= rpConv.getParameter(0,"2nd_harm");
int x=4;
}

