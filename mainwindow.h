#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QTime>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
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
    
    // 灯光控制槽函数

    void on_AllturnOnLightButton_clicked();
    void on_AllturnOffLightButton_clicked();
    void toggleLight(QPushButton* lightButton);
    
    // 更新灯光状态到主页面
    void updateMainPageLightStatus();

    // 网络更新槽函数
    void updateWeatherFromNetwork();
    void onWeatherReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);
    
    // 网络请求完成槽函数
    void onNetworkReplyFinished(QNetworkReply *reply);

    void on_LivingroomAcButton_clicked();

    void on_BedroomAcButton_clicked();

    void on_LivingroomCurtainButton_clicked();

    void on_BedroomCurtainButton_clicked();

    void on_AllCloseCurtainButton_clicked();

private:
    void setupConnections();
    void switchToMainPage();
    void startNetworkUpdate();
    void updateCurrentTime();
    bool parseWeatherData(const QJsonObject& jsonObj);

    Ui::MainWindow *ui;
    QLabel statusTimeLabel;
    QLabel statusWeatherLabel;
    QLabel statusTemperatureLabel;
    
    // 网络相关成员变量
    QNetworkAccessManager *networkManager;
    QNetworkReply *weatherReply;
    QTimer *timeUpdateTimer;
    QTimer *weatherUpdateTimer;
    
    // 灯光相关成员变量
    QMap<QPushButton*, bool> lightStates;
    int lightsOnCount;
};
#endif // MAINWINDOW_H
