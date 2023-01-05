#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <rputility.h>
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


    void on_pushButton_clicked();

    void connectParameterInterface();

    void disconnectParameterInterface();

    void on_log_parameter_changes_cb_stateChanged(int arg1);

    void disableKeyBoardTracking();

    void parameterInitialValueListener(std::string parameter,double value,int pll);

private:
    Ui::MainWindow *ui;
    RPSSHCommunicator rpSSHCommunicator;
    RPParameterUtility rpUtility;
    QGraphicsScene *connectionIndicatorScene;
};
#endif // MAINWINDOW_H
