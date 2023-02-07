#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QFileDialog>
#include <rpparameterutility.h>
#include "rpsshcommunicator.h"

#include <rpregisterutility.h>
Q_DECLARE_METATYPE(std::string)
//Q_DECLARE_METATYPE(int)

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void logMessages(std::string message);
    void connectionStateChangedListener(int code);

    void on_disconnectButton_clicked();

    void on_loadBitfileButton_clicked();

    void on_sendArbitraryCommandBtn_clicked();


    void connectParameterInterface();

    void disconnectParameterInterface();

    void on_log_parameter_changes_cb_stateChanged(int arg1);

    void disableKeyBoardTracking();

    void parameterUIValueListener(std::string parameter,double value,int pll);

    void initConnectionIndicatorScene();

    void on_actionSend_Command_triggered();

    void on_actionLoad_Parameters_triggered();

private:
    Ui::MainWindow *ui;
    RPSSHCommunicator rpSSHCommunicator;
    RPParameterUtility rpParameterUtility;
    QGraphicsScene *connectionIndicatorScene;
};
#endif // MAINWINDOW_H
