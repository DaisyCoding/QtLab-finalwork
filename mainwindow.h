#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QLabel>

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
    void on_LightButton_clicked();
    void on_AcButton_clicked();
    void on_CurtainButton_clicked();
    void on_LockButton_clicked();
    
    void on_comingHomeModeButton_clicked();
    void on_leavingHomeModeButton_clicked();
    void on_SleepModeButton_clicked();
    void on_GameModeButton_clicked();
    
    // 返回按钮槽函数
    void on_LightBackpushButton_clicked();

    void on_CurtainBackpushButton_clicked();

    void on_AcBackpushButton_clicked();

    void on_AllOpenCurtainButton_clicked();

private:
    void setupConnections();
    void switchToMainPage();

    Ui::MainWindow *ui;
    QLabel statusTimeLabel;
    QLabel statusWeatherLabel;
    QLabel statusTemperatureLabel;
};
#endif // MAINWINDOW_H
