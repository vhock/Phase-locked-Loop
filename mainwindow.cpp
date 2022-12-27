#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "rputility.h"
#include <QLabel>
#include <QtGlobal>
#include <cassert>
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

    ;

    ui->connectionIndicator->setScene(connectionIndicatorScene);
    ui->connectionIndicator->show();
    connectionIndicatorScene->setBackgroundBrush(Qt::red); //offline
    connectionIndicatorScene->addText("Offline");
    // a red background

    //QLabel * f0Label=new QLabel(ui->pll1_f0_box);

    ui->ipAddress->setText("192.168.20.188"); //TODO for debugging, delete later
    ui->connect_user_box->setText("root");
    ui->connect_password_box->setText("root");

    //connect UI parameter fields to the Red Pitaya utility
    //red pitaya listens to parameter changes sent by UI


    QObject::connect(&rpUtility,&RPUtility::parameterInitialValue,this,&MainWindow::parameterInitialValueListener,Qt::AutoConnection);


    // UI listens to up-to-date parameter values sent by RP upon initial connection




    //set keyboard tracking to false for all spinboxes, does not work unfortunately
    //   QList<QSpinBox> spinBoxlist= this->findChildren<QSpinBox>();
    //    QList<QSpinBox>::iterator i;
    //   for (i = spinBoxlist.begin(); i != spinBoxlist.end(); ++i){
    //        (*i).setKeyboardTracking(false);
    //   }


    //    for (iter = spinBoxlist.begin(); iter != spinBoxlist.end(); ++iter){
    //        //iter->setKeyboardTracking(false);
    //}


    //  QObject::connect(ui->pll1_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),&rpUtility,&RPUtility::pll1_f0_ChangedListener,Qt::AutoConnection);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::parameterInitialValueListener(std::string parameter,double value,int pll){

    //is there a more elgant way to do this?
    if (parameter=="output_1")
        ui->output1_combobox->setCurrentIndex(value);
    if(parameter=="output_2")
        ui->output2_combobox->setCurrentIndex(value);
    if (parameter=="ext_pins_n"||parameter=="ext_pins_p"){
        //TODO these do not work in the original either it seems. omitted for now
    }
    if (pll==0){
        if (parameter=="a")
            ui->pll1_amplitude_box->setValue(value);
        if(parameter=="phi") //TODO this has huge rounding errors, is there a better way to do it?
            ui->pll1_phase_box->setValue(value);
        if (parameter=="2nd_harm")
            ui->pll1_2nd_harm_cb->setChecked((int)value);
        if( parameter=="pid_en")
            ui->pll1_pid_en_cb->setChecked((int)value);
        if (parameter=="f0")
            ui->pll1_freq_box->setValue(value);
        if(parameter=="bw")
            ui->pll1_bandwith_box->setValue(value);
        if (parameter=="kp")
            ui->pll1_kp_box->setValue(value);
        if(parameter=="ki")
            ui->pll1_ki_box->setValue(value);
        if (parameter=="alpha")
            ui->pll1_alpha_box->setValue(value);
        if (parameter=="order")
            ui->pll1_order_box->setValue(value);
    }
    if (pll==1){
        if (parameter=="a")
            ui->pll2_amplitude_box->setValue(value);
        if(parameter=="phi") //TODO this has huge rounding errors, is there a better way to do it?
            ui->pll2_phase_box->setValue(value);
        if (parameter=="2nd_harm")
            ui->pll2_2nd_harm_cb->setChecked((int)value);
        if( parameter=="pid_en")
            ui->pll2_pid_en_cb->setChecked((int)value);
        if (parameter=="f0")
            ui->pll2_freq_box->setValue(value);
        if(parameter=="bw")
            ui->pll2_bandwith_box->setValue(value);
        if (parameter=="kp")
            ui->pll2_kp_box->setValue(value);
        if(parameter=="ki")
            ui->pll2_ki_box->setValue(value);
        if (parameter=="alpha")
            ui->pll2_alpha_box->setValue(value);
        if (parameter=="order")
            ui->pll2_order_box->setValue(value);
    }








}
void MainWindow::on_connectButton_clicked()
{
    std::string ipAddressText= ui->ipAddress->toPlainText().toStdString();
    std::string userText=ui->connect_user_box->toPlainText().toStdString();
    std::string passwordText=ui->connect_password_box->toPlainText().toStdString();
    bool isValidIP=RPUtility::isValidIPAddress(ipAddressText);
    if (!isValidIP){
        logMessages("Invalid IP");
        return;
    }else {
        logMessages("Valid IP");
        std::thread connectionThread(&RPUtility::connect,&rpUtility,ipAddressText,userText,passwordText);
        connectionThread.detach();

    }
}



//slots
void MainWindow::logMessages(std::string message){
    QString qstring= QString::fromStdString(message);
    ui->statusBox->setText(qstring);
    ui->logBox->append(qstring);
};

void  MainWindow::connectionStateChangedListener(int code){
    if (code==1){//active
        connectionIndicatorScene->clear();//i do not want to bother with finding the text for now
        connectionIndicatorScene->setBackgroundBrush(Qt::green); //online
        connectionIndicatorScene->addText("Online");
        ui->disconnectButton->setEnabled(true);
        ui->connectButton->setEnabled(false);
        rpUtility.synchronizeParameters();

        rpUtility.startMonitorActiveSession();
        connectParameterInterface();


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
    //tests
    // RPParameterConverter rpConv;
    // rpConv.setParameter(0,"2nd_harm",1);
    // rpConv.setParameter(0,"pid_en",1);
    // unsigned long ans= rpConv.getParameterRegister(0,"2nd_harm");
    // assert(ans==192);
    // rpConv.setParameter(0,"output_1",1);//001-pll2
    // rpConv.setParameter(0,"output_2",5);//101-in2
    // unsigned long ans2= rpConv.getParameterRegister(0,"2nd_harm");
    // assert(ans==233);
    rpUtility.synchronizeParameters();





    int x=4;
}

//disables keyboard tracking for the spinboxes because annoying
void MainWindow::disableKeyBoardTracking(){
    //pll1
    ui->pll1_amplitude_box->setKeyboardTracking(false);
    ui->pll1_phase_box->setKeyboardTracking(false);
    ui->pll1_freq_box->setKeyboardTracking(false);
    ui->pll1_bandwith_box->setKeyboardTracking(false);
    ui->pll1_kp_box->setKeyboardTracking(false);
    ui->pll1_ki_box->setKeyboardTracking(false);
    ui->pll1_alpha_box->setKeyboardTracking(false);
    ui->pll1_order_box->setKeyboardTracking(false);
    //pll2
    ui->pll2_amplitude_box->setKeyboardTracking(false);
    ui->pll2_phase_box->setKeyboardTracking(false);
    ui->pll2_freq_box->setKeyboardTracking(false);
    ui->pll2_bandwith_box->setKeyboardTracking(false);
    ui->pll2_kp_box->setKeyboardTracking(false);
    ui->pll2_ki_box->setKeyboardTracking(false);
    ui->pll2_alpha_box->setKeyboardTracking(false);
    ui->pll2_order_box->setKeyboardTracking(false);
}
void MainWindow::connectParameterInterface(){
    QObject::connect(ui->output1_combobox,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),[=]( const int &newValue ) {
        rpUtility.parameterChangedListener("output_1",newValue,0);}
    );

    QObject::connect(ui->output2_combobox,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),[=]( const int &newValue ) {
        rpUtility.parameterChangedListener("output_2",newValue,0);}
    );
    //PLL1

    QObject::connect(ui->pll1_2nd_harm_cb, &QCheckBox::clicked,[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("2nd_harm",newValue,0);}
    );
    QObject::connect(ui->pll1_pid_en_cb, &QCheckBox::clicked,[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("pid_en",newValue,0);}
    );

    QObject::connect(ui->pll1_amplitude_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("a",newValue,0);}
    );


    QObject::connect(ui->pll1_phase_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("phi",newValue,0);}
    );


    QObject::connect(ui->pll1_kp_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("kp",newValue,0);}
    );

    QObject::connect(ui->pll1_ki_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("ki",newValue,0);}
    );


    QObject::connect(ui->pll1_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("f0",newValue,0);}
    );

    QObject::connect(ui->pll1_bandwith_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("bw",newValue,0);}
    );


    QObject::connect(ui->pll1_alpha_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("alpha",newValue,0);}
    );

    QObject::connect(ui->pll1_order_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("order",newValue,0);}
    );

    //PLL2

    QObject::connect(ui->pll2_2nd_harm_cb, &QCheckBox::clicked,[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("2nd_harm",newValue,1);}
    );
    QObject::connect(ui->pll2_pid_en_cb, &QCheckBox::clicked,[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("pid_en",newValue,1);}
    );

    QObject::connect(ui->pll2_amplitude_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("a",newValue,1);}
    );


    QObject::connect(ui->pll2_phase_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("phi",newValue,1);}
    );


    QObject::connect(ui->pll2_kp_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("kp",newValue,1);}
    );

    QObject::connect(ui->pll2_ki_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("ki",newValue,1);}
    );


    QObject::connect(ui->pll2_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("f0",newValue,1);}
    );

    QObject::connect(ui->pll2_bandwith_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("bw",newValue,1);}
    );



    QObject::connect(ui->pll2_alpha_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("alpha",newValue,1);}
    );

    QObject::connect(ui->pll2_order_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpUtility.parameterChangedListener("order",newValue,1);}
    );
}

void MainWindow::on_log_parameter_changes_cb_stateChanged(int arg1)
{
    rpUtility.logParameterChanges=arg1;
}

