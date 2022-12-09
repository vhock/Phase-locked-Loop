#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <rputility.h>
Q_DECLARE_METATYPE(std::string)
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

private:
    Ui::MainWindow *ui;
    RPUtility rpUtility;

};
#endif // MAINWINDOW_H
