// C++ GUI to interface with the RedPitaya based PLL
// Copyright (C) 2023 Vincent Hock
//  Based on work by Felix Tebbenjohanns, Domink Windey and Markus Rademacher

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "rpparameterutility.h"
#include <QLabel>
#include <QtGlobal>
#include <cassert>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),rpSSHCommunicator(),rpParameterUtility(&rpSSHCommunicator)
{
    ui->setupUi(this);
    qRegisterMetaType<std::string>();
    // qRegisterMetaType<int>();


    initConnectionIndicatorScene();


    disableKeyBoardTracking();


    ui->ipAddress->setText("192.168.20.188"); //TODO for debugging, delete later
    ui->connect_user_box->setText("root");
    ui->connect_password_box->setText("root");

    //connect UI parameter fields to the Red Pitaya utility
    //red pitaya listens to parameter changes sent by UI
    //UI listens to parameter synchronization signals sent by RPParameterUtility
    QObject::connect(&rpParameterUtility,&RPParameterUtility::parameterInitialValue,this,&MainWindow::parameterInitialValueListener,Qt::AutoConnection);


    //SSH Connection
    QObject::connect(&rpSSHCommunicator,&RPSSHCommunicator::ssh_connectionStateChanged,this,&MainWindow::connectionStateChangedListener,Qt::AutoConnection);


//Logging
    QObject::connect(&rpSSHCommunicator,&RPSSHCommunicator::ssh_log_message,this,&MainWindow::logMessages,Qt::AutoConnection);
    QObject::connect(&rpParameterUtility,&RPParameterUtility::log_message,this,&MainWindow::logMessages,Qt::AutoConnection);


}

MainWindow::~MainWindow()
{
    rpSSHCommunicator.disconnect();//this one is important

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
    bool isValidIP=RPSSHCommunicator::isValidIPAddress(ipAddressText);
    if (!isValidIP){
        logMessages("Invalid IP");
        return;
    }else {
        logMessages("Valid IP");
        std::thread connectionThread(&RPSSHCommunicator::connect,&rpSSHCommunicator,ipAddressText,userText,passwordText);
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
        rpParameterUtility.synchronizeParameters();
        rpSSHCommunicator.startMonitorActiveSession();
        connectParameterInterface();


    }
    if (code==0){
        connectionIndicatorScene->clear();//i do not want to bother with finding the text for now
        connectionIndicatorScene->setBackgroundBrush(Qt::red); //online
        connectionIndicatorScene->addText("Offline");
        ui->disconnectButton->setEnabled(false);
        ui->connectButton->setEnabled(true);
        disconnectParameterInterface();

    }


}








void MainWindow::on_disconnectButton_clicked()
{
    rpSSHCommunicator.disconnect();
}




void MainWindow::on_loadBitfileButton_clicked()
{
   int copySuccessful= rpSSHCommunicator.scp_copyBitfile();
   if (copySuccessful==0){
      int execSuccessful= rpSSHCommunicator.executeBitfile();
      if (execSuccessful==0){
          rpParameterUtility.synchronizeParameters();
      }
   }
}


void MainWindow::on_sendArbitraryCommandBtn_clicked()
{
    auto command= ui->commandBox->toPlainText().toStdString();
    std::string answer{};
   int commandSent= rpSSHCommunicator.sendCommand(command,answer);
   if (commandSent==0)
    logMessages("Command "+command+" returned: "+answer);

}

void MainWindow::initConnectionIndicatorScene(){
    connectionIndicatorScene = new QGraphicsScene(this);
    ui->connectionIndicator->setScene(connectionIndicatorScene);
    ui->connectionIndicator->show();
    connectionIndicatorScene->setBackgroundBrush(Qt::red); //offline
    connectionIndicatorScene->addText("Offline");
}


void MainWindow::disconnectParameterInterface(){
    QObject::disconnect(ui->output1_combobox,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),nullptr,nullptr
        );
    QObject::disconnect(ui->output1_combobox,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),nullptr,nullptr
        );

        QObject::disconnect(ui->output2_combobox,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),nullptr,nullptr
        );
        //PLL1

        QObject::disconnect(ui->pll1_2nd_harm_cb, &QCheckBox::clicked,nullptr,nullptr
        );
        QObject::disconnect(ui->pll1_pid_en_cb, &QCheckBox::clicked,nullptr,nullptr
        );

        QObject::disconnect(ui->pll1_amplitude_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );


        QObject::disconnect(ui->pll1_phase_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );


        QObject::disconnect(ui->pll1_kp_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),nullptr,nullptr
        );

        QObject::disconnect(ui->pll1_ki_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),nullptr,nullptr
        );


        QObject::disconnect(ui->pll1_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );

        QObject::disconnect(ui->pll1_bandwith_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );


        QObject::disconnect(ui->pll1_alpha_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),nullptr,nullptr
        );

        QObject::disconnect(ui->pll1_order_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );

        //PLL2

        QObject::disconnect(ui->pll2_2nd_harm_cb, &QCheckBox::clicked,nullptr,nullptr
        );
        QObject::disconnect(ui->pll2_pid_en_cb, &QCheckBox::clicked,nullptr,nullptr);

        QObject::disconnect(ui->pll2_amplitude_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );


        QObject::disconnect(ui->pll2_phase_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr);


        QObject::disconnect(ui->pll2_kp_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),nullptr,nullptr
        );

        QObject::disconnect(ui->pll2_ki_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),nullptr,nullptr
        );


        QObject::disconnect(ui->pll2_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );

        QObject::disconnect(ui->pll2_bandwith_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );



        QObject::disconnect(ui->pll2_alpha_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),nullptr,nullptr
        );

        QObject::disconnect(ui->pll2_order_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),nullptr,nullptr
        );


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
        rpParameterUtility.parameterChangedListener("output_1",newValue,0);}
    );

    QObject::connect(ui->output2_combobox,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),[=]( const int &newValue ) {
        rpParameterUtility.parameterChangedListener("output_2",newValue,0);}
    );
    //PLL1

    QObject::connect(ui->pll1_2nd_harm_cb, &QCheckBox::clicked,[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("2nd_harm",newValue,0);}
    );
    QObject::connect(ui->pll1_pid_en_cb, &QCheckBox::clicked,[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("pid_en",newValue,0);}
    );

    QObject::connect(ui->pll1_amplitude_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("a",newValue,0);}
    );


    QObject::connect(ui->pll1_phase_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("phi",newValue,0);}
    );


    QObject::connect(ui->pll1_kp_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("kp",newValue,0);}
    );

    QObject::connect(ui->pll1_ki_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("ki",newValue,0);}
    );


    QObject::connect(ui->pll1_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("f0",newValue,0);}
    );

    QObject::connect(ui->pll1_bandwith_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("bw",newValue,0);}
    );


    QObject::connect(ui->pll1_alpha_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("alpha",newValue,0);}
    );

    QObject::connect(ui->pll1_order_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("order",newValue,0);}
    );

    //PLL2

    QObject::connect(ui->pll2_2nd_harm_cb, &QCheckBox::clicked,[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("2nd_harm",newValue,1);}
    );
    QObject::connect(ui->pll2_pid_en_cb, &QCheckBox::clicked,[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("pid_en",newValue,1);}
    );

    QObject::connect(ui->pll2_amplitude_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("a",newValue,1);}
    );


    QObject::connect(ui->pll2_phase_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("phi",newValue,1);}
    );


    QObject::connect(ui->pll2_kp_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("kp",newValue,1);}
    );

    QObject::connect(ui->pll2_ki_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("ki",newValue,1);}
    );


    QObject::connect(ui->pll2_freq_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("f0",newValue,1);}
    );

    QObject::connect(ui->pll2_bandwith_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("bw",newValue,1);}
    );



    QObject::connect(ui->pll2_alpha_box, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("alpha",newValue,1);}
    );

    QObject::connect(ui->pll2_order_box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),[=]( const double &newValue ) {
        rpParameterUtility.parameterChangedListener("order",newValue,1);}
    );
}

void MainWindow::on_log_parameter_changes_cb_stateChanged(int arg1)
{
    rpParameterUtility.logParameterChanges=arg1;
}


void MainWindow::on_actionSend_Command_triggered()
{
  //test map
   // rpParameterUtility.saveParameters();
    std::map<std::string,std::string> testMap{{"a","1"},{"b","2"}};
    QString suffix=QString::fromStdString(".param");
   QString fileName=  QFileDialog::getSaveFileName(this, "Save file", "set", suffix);
   std::ofstream ofs(fileName.toStdString());
   //boost::archive::text_oarchive oa(ofs);

}

