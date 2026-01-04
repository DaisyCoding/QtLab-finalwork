#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QTimer>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(nullptr)
    , weatherReply(nullptr)
    , timeUpdateTimer(nullptr)
    , weatherUpdateTimer(nullptr)
{
    ui->setupUi(this);

    // 初始化状态栏标签
    statusTimeLabel.setMidLineWidth(200);
    statusTimeLabel.setText("加载中...");
    
    statusTemperatureLabel.setMidLineWidth(200);
    statusTemperatureLabel.setText("加载中...");
    
    statusWeatherLabel.setMidLineWidth(200);
    statusWeatherLabel.setText("加载中...");

    ui->statusbar->addPermanentWidget(&statusTimeLabel);
    ui->statusbar->addPermanentWidget(&statusWeatherLabel);
    ui->statusbar->addPermanentWidget(&statusTemperatureLabel);
    ui->statusbar->setMinimumHeight(50);

    // 初始化网络管理器
    networkManager = new QNetworkAccessManager(this);
    
    // 初始化定时器
    timeUpdateTimer = new QTimer(this);
    weatherUpdateTimer = new QTimer(this);
    
    // 连接定时器信号
    connect(timeUpdateTimer, &QTimer::timeout, this, &MainWindow::updateCurrentTime);
    connect(weatherUpdateTimer, &QTimer::timeout, this, &MainWindow::updateWeatherFromNetwork);
    
    // 先启动定时器，再启动网络更新
    startNetworkUpdate();

    setupConnections();
}

MainWindow::~MainWindow()
{
    // 清理网络请求
    if (weatherReply) {
        weatherReply->abort();
        weatherReply->deleteLater();
    }
    
    delete ui;
}

void MainWindow::setupConnections()
{
    // 设备按钮信号槽连接
    connect(ui->LightButton, &QPushButton::clicked, this, &MainWindow::on_LightButton_clicked);
    connect(ui->AcButton, &QPushButton::clicked, this, &MainWindow::on_AcButton_clicked);
    connect(ui->CurtainButton, &QPushButton::clicked, this, &MainWindow::on_CurtainButton_clicked);
    connect(ui->LockButton, &QPushButton::clicked, this, &MainWindow::on_LockButton_clicked);

    // 场景按钮信号槽连接
    connect(ui->comingHomeModeButton, &QPushButton::clicked, this, &MainWindow::on_comingHomeModeButton_clicked);
    connect(ui->leavingHomeModeButton, &QPushButton::clicked, this, &MainWindow::on_leavingHomeModeButton_clicked);
    connect(ui->SleepModeButton, &QPushButton::clicked, this, &MainWindow::on_SleepModeButton_clicked);
    connect(ui->GameModeButton, &QPushButton::clicked, this, &MainWindow::on_GameModeButton_clicked);

    // 返回按钮信号槽连接
    connect(ui->LightBackpushButton, &QPushButton::clicked, this, &MainWindow::on_LightBackpushButton_clicked);
    connect(ui->CurtainBackpushButton, &QPushButton::clicked, this, &MainWindow::on_CurtainBackpushButton_clicked);
    connect(ui->AcBackpushButton, &QPushButton::clicked, this, &MainWindow::on_AcBackpushButton_clicked);

    // 网络请求完成信号连接
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onNetworkReplyFinished);
}

void MainWindow::startNetworkUpdate()
{
    qDebug() << "开始网络更新...";
    
    // 直接使用本地时间更新
    qDebug() << "更新本地时间...";
    updateCurrentTime();
    
    qDebug() << "调用updateWeatherFromNetwork()...";
    updateWeatherFromNetwork();
    
    // 设置定时器，时间每分钟更新一次，天气每30分钟更新一次
    qDebug() << "启动定时器...";
    timeUpdateTimer->start(60000);  // 1分钟
    weatherUpdateTimer->start(1800000);  // 30分钟
    qDebug() << "定时器启动完成";
}

void MainWindow::updateCurrentTime()
{
    // 获取当前时间并格式化
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timeString = currentDateTime.toString("yyyy-MM-dd hh:mm");
    statusTimeLabel.setText(timeString);
}

void MainWindow::onNetworkReplyFinished(QNetworkReply *reply)
{
    qDebug() << "网络请求完成，reply对象:" << reply;
    
    if (reply == weatherReply) {
        qDebug() << "天气请求完成，调用onWeatherReplyFinished()";
        onWeatherReplyFinished();
    } else {
        qDebug() << "未识别的请求类型";
        reply->deleteLater();
    }
}

void MainWindow::updateWeatherFromNetwork()
{
    // 使用和风天气API获取天气信息
    QUrl weatherUrl("https://n66apx77xf.re.qweatherapi.com/v7/weather/now?location=101281601");
    
    if (weatherReply) {
        weatherReply->abort();
        weatherReply->deleteLater();
    }
    
    // 创建请求并添加API密钥到请求头
    QNetworkRequest request(weatherUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtSmartHomeApp/1.0");
    request.setRawHeader("X-QW-Api-Key", "228b0b2673454eacb238fdefe86d9409");
    
    weatherReply = networkManager->get(request);
}

void MainWindow::onWeatherReplyFinished()
{
    qDebug() << "天气请求完成处理开始";
    
    if (!weatherReply) {
        qDebug() << "天气请求对象为空";
        return;
    }
    
    // 检查响应状态码
    if (weatherReply->error() != QNetworkReply::NoError) {
        qDebug() << "天气请求错误:" << weatherReply->errorString();
        
        statusWeatherLabel.setText("天气获取失败");
        statusTemperatureLabel.setText("温度获取失败");

        weatherReply->deleteLater();
        weatherReply = nullptr;
        return;
    }
    
    qDebug() << "天气请求成功，响应代码:" << weatherReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    // 读取响应数据
    QByteArray responseData = weatherReply->readAll();
    qDebug() << "响应数据长度:" << responseData.size();
    qDebug() << "响应数据内容:" << responseData.left(200); // 只显示前200个字符
    qDebug() << "检查JSON解析...";
    
    // 解析JSON响应
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData, &jsonError);
    
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << jsonError.errorString();
        
        // 不更新天气信息，保持原来的数据
        weatherReply->deleteLater();
        weatherReply = nullptr;
        return;
    }
    
    qDebug() << "JSON解析成功";
    
    // 提取天气信息
    QJsonObject jsonObj = jsonDoc.object();
    
    // 检查API返回的状态码
    if (jsonObj.contains("code")) {
        QString code = jsonObj["code"].toString();
        if (code != "200") {
            qDebug() << "和风天气API返回错误:" << code;
            
            // API返回错误时，不更新天气信息，保持原来的数据
            weatherReply->deleteLater();
            weatherReply = nullptr;
            return;
        }
    }
    
    // 尝试解析天气数据
    if (!parseWeatherData(jsonObj)) {
        // 如果无法解析数据，不更新天气信息，保持原来的数据
        qDebug() << "无法解析天气数据，保持原来的信息";
    }
    
    weatherReply->deleteLater();
    weatherReply = nullptr;
}

// 解析天气数据的函数
bool MainWindow::parseWeatherData(const QJsonObject& jsonObj)
{
    qDebug() << "开始解析天气数据";
    
    if (jsonObj.contains("now")) {
        qDebug() << "找到now字段";
        QJsonObject nowObj = jsonObj["now"].toObject();
        
        if (!nowObj.isEmpty()) {
            qDebug() << "now对象不为空";
            
            // 获取温度
            if (nowObj.contains("temp")) {
                QString temp = nowObj["temp"].toString();
                QString tempString = temp + "°C";
                qDebug() << "更新温度:" << tempString;
                statusTemperatureLabel.setText(tempString);
            }
            
            // 获取天气描述
            if (nowObj.contains("text")) {
                QString weatherDesc = nowObj["text"].toString();
                qDebug() << "更新天气描述:" << weatherDesc;
                statusWeatherLabel.setText(weatherDesc);
            }
            
            qDebug() << "更新天气:" << statusWeatherLabel.text() << "温度:" << statusTemperatureLabel.text();
            return true;
        } else {
            qDebug() << "now对象为空";
        }
    } else {
        qDebug() << "JSON中没有找到now字段";
    }
    
    return false;
}



void MainWindow::onNetworkError(QNetworkReply::NetworkError error)
{
    qDebug() << "网络错误:" << error;
    
    // 设置错误状态
    statusWeatherLabel.setText("网络错误");
    statusTemperatureLabel.setText("网络错误");
    
    // 时间仍然显示本地时间
    updateCurrentTime();
}

void MainWindow::switchToMainPage()
{
    ui->stackedWidget->setCurrentWidget(ui->MainPage);
}

void MainWindow::on_LightButton_clicked()
{
    qDebug() << "切换到灯光控制页面";
    ui->stackedWidget->setCurrentWidget(ui->LightPage);
}

void MainWindow::on_AcButton_clicked()
{
    qDebug() << "切换到空调控制页面";
    ui->stackedWidget->setCurrentWidget(ui->AcPage);
}

void MainWindow::on_CurtainButton_clicked()
{
    qDebug() << "切换到窗帘控制页面";
    ui->stackedWidget->setCurrentWidget(ui->CurtainPage);
}

void MainWindow::on_LockButton_clicked()
{
    qDebug() << "门锁功能暂未实现";
    // TODO: 实现门锁控制页面
}

void MainWindow::on_comingHomeModeButton_clicked()
{
    qDebug() << "执行回家模式";
    // TODO: 实现场景逻辑
}

void MainWindow::on_leavingHomeModeButton_clicked()
{
    qDebug() << "执行离家模式";
    // TODO: 实现场景逻辑
}

void MainWindow::on_SleepModeButton_clicked()
{
    qDebug() << "执行睡眠模式";
    // TODO: 实现场景逻辑
}

void MainWindow::on_GameModeButton_clicked()
{
    qDebug() << "执行游戏模式";
    // TODO: 实现场景逻辑
}

void MainWindow::on_LightBackpushButton_clicked()
{
    qDebug() << "从灯光页面返回主页面";
    switchToMainPage();
}



void MainWindow::on_CurtainBackpushButton_clicked()
{
    qDebug() << "从窗帘页面返回主页面";
    switchToMainPage();
}


void MainWindow::on_AcBackpushButton_clicked()
{
    qDebug() << "从空调页面返回主页面";
    switchToMainPage();
}


void MainWindow::on_AllOpenCurtainButton_clicked()
{

}

