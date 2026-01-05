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
    , lightsOnCount(0)
    , curtainsOpenCount(0)
    , outsideTemperature(25)  // 默认室外温度为25度
    , wakeUpTimer(nullptr)
    , wakeUpStatusLabel(nullptr)
    , isWakeUpModeActive(false)
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
    
    // 初始化灯光状态，所有灯默认关闭
    lightStates[ui->LivingroomLightButton] = false;
    lightStates[ui->KitchenLightButton] = false;
    lightStates[ui->BedroomLightButton] = false;
    lightStates[ui->BathroomLightButton] = false;
    lightStates[ui->StudyroomLightButton] = false;
    lightStates[ui->BalconyLightButton] = false;
    lightStates[ui->DiningroomLightButton] = false;
    
    // 确保所有灯光按钮初始状态一致
    QList<QPushButton*> allLightButtons = {
        ui->LivingroomLightButton,
        ui->KitchenLightButton,
        ui->BedroomLightButton,
        ui->BathroomLightButton,
        ui->StudyroomLightButton,
        ui->BalconyLightButton,
        ui->DiningroomLightButton
    };
    
    for (QPushButton* button : allLightButtons) {
        if (button) {
            button->setText("关");
            button->setStyleSheet("");
            qDebug() << "初始化灯光按钮:" << button->objectName() << "状态:关";
        } else {
            qDebug() << "警告：发现空灯光按钮";
        }
    }
    
    // 重置灯光计数并更新主页面显示
    lightsOnCount = 0;
    updateMainPageLightStatus();
    qDebug() << "灯光系统初始化完成，开启灯数量:" << lightsOnCount;

    //空调界面初始化
    ui->LivingroomAcButton->setText("关");
    ui->BedroomAcButton->setText("关");
    ui->LivingroomAcModecomboBox->setEnabled(false);
    ui->LivingroomTemperaturecomboBox->setEnabled(false);
    ui->BedroomAcModecomboBox->setEnabled(false);
    ui->BedroomTemperaturecomboBox->setEnabled(false);

    //窗帘初始化
    ui->LivingroomCurtainButton->setText("关");
    ui->BedroomCurtainButton->setText("关");
    
    // 初始化窗帘状态，所有窗帘默认关闭
    curtainStates[ui->LivingroomCurtainButton] = false;
    curtainStates[ui->BedroomCurtainButton] = false;
    
    // 重置窗帘计数并更新主页面显示
    curtainsOpenCount = 0;
    updateMainPageCurtainStatus();
    qDebug() << "窗帘系统初始化完成，开启窗帘数量:" << curtainsOpenCount;
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

    // 删除闹钟按钮连接
    connect(ui->DeleteWakeUpAlarmButton, &QPushButton::clicked, this, &MainWindow::cancelWakeUpAlarm);

    // 返回按钮信号槽连接
    connect(ui->LightBackpushButton, &QPushButton::clicked, this, &MainWindow::on_LightBackpushButton_clicked);
    connect(ui->CurtainBackpushButton, &QPushButton::clicked, this, &MainWindow::on_CurtainBackpushButton_clicked);
    connect(ui->AcBackpushButton, &QPushButton::clicked, this, &MainWindow::on_AcBackpushButton_clicked);

    // 使用 lambda 表达式处理灯光按钮点击事件，避免重复调用
    connect(ui->LivingroomLightButton, &QPushButton::clicked, [this]() {
        toggleLight(ui->LivingroomLightButton);
    });
    
    connect(ui->KitchenLightButton, &QPushButton::clicked, [this]() {
        toggleLight(ui->KitchenLightButton);
    });
    
    connect(ui->BedroomLightButton, &QPushButton::clicked, [this]() {
        toggleLight(ui->BedroomLightButton);
    });
    
    connect(ui->BathroomLightButton, &QPushButton::clicked, [this]() {
        toggleLight(ui->BathroomLightButton);
    });
    
    connect(ui->StudyroomLightButton, &QPushButton::clicked, [this]() {
        toggleLight(ui->StudyroomLightButton);
    });
    
    connect(ui->BalconyLightButton, &QPushButton::clicked, [this]() {
        toggleLight(ui->BalconyLightButton);
    });
    
    connect(ui->DiningroomLightButton, &QPushButton::clicked, [this]() {
        toggleLight(ui->DiningroomLightButton);
    });
    
    // 全开/全关按钮信号槽连接
    connect(ui->AllturnOnLightButton, &QPushButton::clicked, this, &MainWindow::on_AllturnOnLightButton_clicked);
    connect(ui->AllturnOffLightButton, &QPushButton::clicked, this, &MainWindow::on_AllturnOffLightButton_clicked);

    // 窗帘按钮信号槽连接
    connect(ui->LivingroomCurtainButton, &QPushButton::clicked, [this]() {
        toggleCurtain(ui->LivingroomCurtainButton);
    });
    
    connect(ui->BedroomCurtainButton, &QPushButton::clicked, [this]() {
        toggleCurtain(ui->BedroomCurtainButton);
    });
    
    // 全开/全关窗帘按钮信号槽连接
    connect(ui->AllOpenCurtainButton, &QPushButton::clicked, this, &MainWindow::on_AllOpenCurtainButton_clicked);
    connect(ui->AllCloseCurtainButton, &QPushButton::clicked, this, &MainWindow::on_AllCloseCurtainButton_clicked);

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
                qDebug()<<temp;
                outsideTemperature = temp.toInt();  // 更新室外温度
                QString tempString = temp + "°C";
                QString insideTemp = QString::number(temp.toInt()+3);//模拟室内温度比室外高3度
                qDebug() << "更新温度:" << tempString << "室外温度已更新为:" << outsideTemperature << "°C";
                statusTemperatureLabel.setText(tempString);
                ui->Aclabel->setText("室内温度为:"+insideTemp+ "°C");
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
    
    return true;
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
    if(ui->Locklabel->text() == "未锁门")
        ui->Locklabel->setText("已锁门");
}

void MainWindow::on_comingHomeModeButton_clicked()
{
    qDebug() << "执行回家模式";
    qDebug() << "当前室外温度:" << outsideTemperature << "°C";
    
    // 1. 打开客厅灯和厨房灯（确保灯被打开，而不是切换）
    qDebug() << "打开客厅灯和厨房灯";
    turnOnLight(ui->LivingroomLightButton);
    turnOnLight(ui->KitchenLightButton);
    
    // 2. 关闭两个窗帘
    qDebug() << "关闭两个窗帘";
    turnOffCurtain(ui->LivingroomCurtainButton);
    turnOffCurtain(ui->BedroomCurtainButton);
    
    // 3. 根据室外温度智能控制空调
    qDebug() << "检查是否需要打开空调";
    turnOnAirConditionerWithSmartControl();
}

void MainWindow::turnOnLight(QPushButton* lightButton)
{
    // 验证按钮有效性
    if (!lightButton) {
        qDebug() << "错误：无效的灯光按钮";
        return;
    }
    
    // 获取当前灯光状态
    bool isOn = lightStates[lightButton];
    
    if (!isOn) {
        // 如果灯是关闭的，打开它
        lightStates[lightButton] = true;
        lightButton->setText("开");
        lightButton->setStyleSheet("background-color: #FFD700; color: black; font-weight: bold;");
        lightsOnCount++;
        qDebug() << "开灯:" << lightButton->objectName() << "新状态:开" << "灯光数量:" << lightsOnCount;
        updateMainPageLightStatus();
    } else {
        qDebug() << "灯已经是打开状态:" << lightButton->objectName();
    }
}

void MainWindow::turnOffCurtain(QPushButton* curtainButton)
{
    // 验证按钮有效性
    if (!curtainButton) {
        qDebug() << "错误：无效的窗帘按钮";
        return;
    }
    
    // 获取当前窗帘状态
    bool isOpen = curtainStates[curtainButton];
    
    if (isOpen) {
        // 如果窗帘是打开的，关闭它
        curtainStates[curtainButton] = false;
        curtainButton->setText("关");
        curtainButton->setStyleSheet("");
        curtainsOpenCount--;
        qDebug() << "关窗帘:" << curtainButton->objectName() << "新状态:关" << "窗帘数量:" << curtainsOpenCount;
        updateMainPageCurtainStatus();
    } else {
        qDebug() << "窗帘已经是关闭状态:" << curtainButton->objectName();
    }
}

void MainWindow::turnOnAirConditionerWithSmartControl()
{
    // 根据室外温度判断是否需要开空调
    // 15-26度之间不需要开空调
    if (outsideTemperature > 15 && outsideTemperature < 26) {
        qDebug() << "室外温度" << outsideTemperature << "°C 在15-26度之间，不开空调";
        return;
    }
    
    // 室外温度>=26度或<=15度，需要开空调
    qDebug() << "室外温度:" << outsideTemperature << "°C，需要开启空调";
    
    // 获取当前空调状态
    QString status = ui->LivingroomAcButton->text();
    
    if (status == "关") {
        // 开启客厅空调
        ui->LivingroomAcButton->setText("开");
        ui->LivingroomAcButton->setStyleSheet("background-color: #FFD700; color: black; font-weight: bold;");
        ui->LivingroomAcModecomboBox->setEnabled(true);
        ui->LivingroomTemperaturecomboBox->setEnabled(true);
        qDebug() << "开空调:" << ui->LivingroomAcButton->objectName() << "新状态:开";
    }
    
    // 根据室外温度智能设置空调模式和温度
    if (outsideTemperature >= 26) {
        // 室外温度>=26度，设置制冷模式，温度比室外低2度
        int targetTemp = outsideTemperature - 2;
        ui->LivingroomAcModecomboBox->setCurrentText("制冷");
        ui->LivingroomTemperaturecomboBox->setCurrentText(QString::number(targetTemp));
        qDebug() << "制冷模式，温度设置为:" << targetTemp << "°C";
    } else if (outsideTemperature <= 15) {
        // 室外温度<=15度，设置制热模式，温度比室外高3度
        int targetTemp = outsideTemperature + 3;
        ui->LivingroomAcModecomboBox->setCurrentText("制热");
        ui->LivingroomTemperaturecomboBox->setCurrentText(QString::number(targetTemp));
        qDebug() << "制热模式，温度设置为:" << targetTemp << "°C";
    }
}

void MainWindow::on_leavingHomeModeButton_clicked()
{
    qDebug() << "执行离家模式";
    
    // 1. 打开所有窗帘
    qDebug() << "打开所有窗帘";
    turnOnCurtain(ui->LivingroomCurtainButton);
    turnOnCurtain(ui->BedroomCurtainButton);
    
    // 2. 关闭所有灯光
    qDebug() << "关闭所有灯光";
    turnOffLight(ui->LivingroomLightButton);
    turnOffLight(ui->KitchenLightButton);
    turnOffLight(ui->BedroomLightButton);
    turnOffLight(ui->BathroomLightButton);
    turnOffLight(ui->StudyroomLightButton);
    turnOffLight(ui->BalconyLightButton);
    turnOffLight(ui->DiningroomLightButton);
    
    // 3. 关闭所有空调
    qDebug() << "关闭所有空调";
    turnOffAirConditioner();
}

void MainWindow::turnOffLight(QPushButton* lightButton)
{
    if (!lightButton) {
        qDebug() << "错误：无效的灯光按钮";
        return;
    }
    
    bool isOn = lightStates[lightButton];
    
    if (isOn) {
        lightStates[lightButton] = false;
        lightButton->setText("关");
        lightButton->setStyleSheet("");
        lightsOnCount--;
        qDebug() << "关灯:" << lightButton->objectName() << "新状态:关" << "灯光数量:" << lightsOnCount;
        updateMainPageLightStatus();
    } else {
        qDebug() << "灯已经是关闭状态:" << lightButton->objectName();
    }
}

void MainWindow::turnOnCurtain(QPushButton* curtainButton)
{
    if (!curtainButton) {
        qDebug() << "错误：无效的窗帘按钮";
        return;
    }
    
    bool isOpen = curtainStates[curtainButton];
    
    if (!isOpen) {
        curtainStates[curtainButton] = true;
        curtainButton->setText("开");
        curtainButton->setStyleSheet("background-color: #FFD700; color: black; font-weight: bold;");
        curtainsOpenCount++;
        qDebug() << "开窗帘:" << curtainButton->objectName() << "新状态:开" << "窗帘数量:" << curtainsOpenCount;
        updateMainPageCurtainStatus();
    } else {
        qDebug() << "窗帘已经是打开状态:" << curtainButton->objectName();
    }
}

void MainWindow::turnOffAirConditioner()
{
    // 关闭客厅空调
    if (ui->LivingroomAcButton->text() == "开") {
        ui->LivingroomAcButton->setText("关");
        ui->LivingroomAcButton->setStyleSheet("");
        ui->LivingroomAcModecomboBox->setEnabled(false);
        ui->LivingroomTemperaturecomboBox->setEnabled(false);
        qDebug() << "关空调:" << ui->LivingroomAcButton->objectName();
    }
    
    // 关闭卧室空调
    if (ui->BedroomAcButton->text() == "开") {
        ui->BedroomAcButton->setText("关");
        ui->BedroomAcButton->setStyleSheet("");
        ui->BedroomAcModecomboBox->setEnabled(false);
        ui->BedroomTemperaturecomboBox->setEnabled(false);
        qDebug() << "关空调:" << ui->BedroomAcButton->objectName();
    }
}

void MainWindow::on_SleepModeButton_clicked()
{
    qDebug() << "执行睡眠模式";
    qDebug() << "当前室外温度:" << outsideTemperature << "°C";
    
    // 1. 打开卧室灯
    qDebug() << "打开卧室灯";
    turnOnLight(ui->BedroomLightButton);
    
    // 2. 关闭其他所有灯光
    qDebug() << "关闭其他灯光";
    turnOffLight(ui->LivingroomLightButton);
    turnOffLight(ui->KitchenLightButton);
    turnOffLight(ui->BathroomLightButton);
    turnOffLight(ui->StudyroomLightButton);
    turnOffLight(ui->BalconyLightButton);
    turnOffLight(ui->DiningroomLightButton);
    
    // 3. 关闭卧室窗帘
    qDebug() << "关闭卧室窗帘";
    turnOffCurtain(ui->BedroomCurtainButton);
    
    // 4. 关闭客厅空调
    qDebug() << "关闭客厅空调";
    if (ui->LivingroomAcButton->text() == "开") {
        ui->LivingroomAcButton->setText("关");
        ui->LivingroomAcButton->setStyleSheet("");
        ui->LivingroomAcModecomboBox->setEnabled(false);
        ui->LivingroomTemperaturecomboBox->setEnabled(false);
        qDebug() << "关空调:" << ui->LivingroomAcButton->objectName();
    }
    
    // 5. 根据室外温度智能控制卧室空调
    qDebug() << "检查是否需要打开卧室空调";
    turnOnAirConditionerWithSmartControlForBedroom();
    
    // 6. 点击门锁按钮
    qDebug() << "点击门锁按钮";
    on_LockButton_clicked();
}

void MainWindow::turnOnAirConditionerWithSmartControlForBedroom()
{
    // 根据室外温度判断是否需要开空调
    // 15-26度之间不需要开空调
    if (outsideTemperature > 15 && outsideTemperature < 26) {
        qDebug() << "室外温度" << outsideTemperature << "°C 在15-26度之间，不开卧室空调";
        return;
    }
    
    // 室外温度>=26度或<=15度，需要开空调
    qDebug() << "室外温度:" << outsideTemperature << "°C，需要开启卧室空调";
    
    // 获取当前空调状态
    QString status = ui->BedroomAcButton->text();
    
    if (status == "关") {
        // 开启卧室空调
        ui->BedroomAcButton->setText("开");
        ui->BedroomAcButton->setStyleSheet("background-color: #FFD700; color: black; font-weight: bold;");
        ui->BedroomAcModecomboBox->setEnabled(true);
        ui->BedroomTemperaturecomboBox->setEnabled(true);
        qDebug() << "开卧室空调:" << ui->BedroomAcButton->objectName() << "新状态:开";
    }
    
    // 设置为睡眠模式
    ui->BedroomAcModecomboBox->setCurrentText("睡眠");
    qDebug() << "空调模式设置为:睡眠";
    
    // 根据室外温度智能设置空调温度
    if (outsideTemperature >= 26) {
        // 室外温度>=26度，设置制冷模式，温度比室外低2度
        int targetTemp = outsideTemperature - 2;
        ui->BedroomTemperaturecomboBox->setCurrentText(QString::number(targetTemp));
        qDebug() << "制冷模式，温度设置为:" << targetTemp << "°C";
    } else if (outsideTemperature <= 15) {
        // 室外温度<=15度，设置制热模式，温度比室外高3度
        int targetTemp = outsideTemperature + 3;
        ui->BedroomTemperaturecomboBox->setCurrentText(QString::number(targetTemp));
        qDebug() << "制热模式，温度设置为:" << targetTemp << "°C";
    }
}

void MainWindow::on_WakeUpModeButton_clicked()
{
    qDebug() << "执行起床模式";
    
    TimePickerDialog dialog(this);
    dialog.setSelectedTime(QTime::currentTime().addSecs(1));  // 默认1分钟后
    
    if (dialog.exec() == QDialog::Accepted) {
        QTime selectedTime = dialog.selectedTime();
        wakeUpTime = QDateTime::currentDateTime();
        wakeUpTime.setTime(selectedTime);
        
        // 如果选择的时间已经过了今天的当前时间，设置到明天
        if (wakeUpTime <= QDateTime::currentDateTime()) {
            wakeUpTime = wakeUpTime.addDays(1);
        }
        
        qDebug() << "设置的起床时间:" << wakeUpTime.toString("yyyy-MM-dd hh:mm:ss");
        
        // 停止并删除之前的定时器
        if (wakeUpTimer) {
            wakeUpTimer->stop();
            delete wakeUpTimer;
            wakeUpTimer = nullptr;
        }
        
        // 计算到目标时间的间隔（毫秒）
        qint64 intervalMs = QDateTime::currentDateTime().msecsTo(wakeUpTime);
        qDebug() << "间隔时间（毫秒）:" << intervalMs;
        
        if (intervalMs > 0) {
            // 使用 QTimer::singleShot 更可靠
            wakeUpTimer = new QTimer(this);
            connect(wakeUpTimer, &QTimer::timeout, this, [this]() {
                executeWakeUpActions();
            });
            wakeUpTimer->start(intervalMs);
            
            isWakeUpModeActive = true;
            qDebug() << "起床模式已启动，距离目标时间还有" << (intervalMs / 1000) << "秒";
            
            // 在状态栏显示提示
            if (wakeUpStatusLabel) {
                delete wakeUpStatusLabel;
            }
            wakeUpStatusLabel = new QLabel(this);
            wakeUpStatusLabel->setText(QString("起床闹钟: %1").arg(selectedTime.toString("hh:mm")));
            ui->statusbar->addWidget(wakeUpStatusLabel, 0);
        } else {
            qDebug() << "错误：选择的时间已过";
        }
    }
}

void MainWindow::cancelWakeUpAlarm()
{
    qDebug() << "删除闹钟";
    
    if (wakeUpTimer) {
        wakeUpTimer->stop();
        delete wakeUpTimer;
        wakeUpTimer = nullptr;
        qDebug() << "定时器已停止并删除";
    }
    
    if (wakeUpStatusLabel) {
        ui->statusbar->removeWidget(wakeUpStatusLabel);
        delete wakeUpStatusLabel;
        wakeUpStatusLabel = nullptr;
        qDebug() << "状态栏标签已删除";
    }
    
    isWakeUpModeActive = false;
    qDebug() << "闹钟已删除";
}

void MainWindow::executeWakeUpActions()
{
    qDebug() << "执行起床操作";
    
    // 停止并删除定时器
    if (wakeUpTimer) {
        wakeUpTimer->stop();
        delete wakeUpTimer;
        wakeUpTimer = nullptr;
    }
    
    // 移除状态栏提示
    if (wakeUpStatusLabel) {
        ui->statusbar->removeWidget(wakeUpStatusLabel);
        delete wakeUpStatusLabel;
        wakeUpStatusLabel = nullptr;
    }
    
    isWakeUpModeActive = false;
    
    // 1. 打开卧室窗帘
    qDebug() << "打开卧室窗帘";
    turnOnCurtain(ui->BedroomCurtainButton);
    
    // 2. 关闭卧室空调
    qDebug() << "关闭卧室空调";
    if (ui->BedroomAcButton->text() == "开") {
        ui->BedroomAcButton->setText("关");
        ui->BedroomAcButton->setStyleSheet("");
        ui->BedroomAcModecomboBox->setEnabled(false);
        ui->BedroomTemperaturecomboBox->setEnabled(false);
        qDebug() << "关卧室空调:" << ui->BedroomAcButton->objectName();
    }
    
    // 3. 如果时间早于7点，打开卧室灯
    QTime currentTime = QTime::currentTime();
    qDebug() << "当前时间:" << currentTime.toString("hh:mm");
    
    if (currentTime.hour() < 7) {
        qDebug() << "当前时间早于7点，打开卧室灯";
        turnOnLight(ui->BedroomLightButton);
    } else {
        qDebug() << "当前时间晚于7点或等于7点，不开灯";
    }
    
    qDebug() << "起床操作执行完成";
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
    qDebug() << "执行全开窗帘操作";
    
    // 遍历所有窗帘状态
    for (auto it = curtainStates.begin(); it != curtainStates.end(); ++it) {
        // 如果窗帘当前是关闭的
        if (!it.value()) {
            // 调用 toggleCurtain 来打开它
            toggleCurtain(it.key());
        }
    }
}


void MainWindow::on_AllturnOnLightButton_clicked()
{
    // 遍历所有灯光状态
    for (auto it = lightStates.begin(); it != lightStates.end(); ++it) {
        // 如果灯当前是关闭的
        if (!it.value()) {
            // 调用 toggleLight 来打开它
            toggleLight(it.key());
        }
    }
}

void MainWindow::on_AllturnOffLightButton_clicked()
{
    // 遍历所有灯光状态
       for (auto it = lightStates.begin(); it != lightStates.end(); ++it) {
           // 如果灯当前是打开的
           if (it.value()) {
               // 调用 toggleLight 来关闭它
               toggleLight(it.key());
           }
       }
}

void MainWindow::toggleLight(QPushButton* lightButton)
{
    // 验证按钮有效性
    if (!lightButton) {
        qDebug() << "错误：无效的灯光按钮";
        return;
    }
    
    // 获取当前灯光状态
    bool isOn = lightStates[lightButton];
    qDebug() << "切换灯光前状态:" << (isOn ? "开" : "关");
    
    // 切换灯光状态
    lightStates[lightButton] = !isOn;
    
    if (!isOn) {
        // 开启灯光（之前是关闭状态）
        lightButton->setText("开");
        lightButton->setStyleSheet("background-color: #FFD700; color: black; font-weight: bold;");
        lightsOnCount++;
        qDebug() << "开灯:" << lightButton->objectName() << "新状态:开" << "灯光数量:" << lightsOnCount;
    } else {
        // 关闭灯光（之前是开启状态）
        lightButton->setText("关");
        lightButton->setStyleSheet("");
        lightsOnCount--;
        qDebug() << "关灯:" << lightButton->objectName() << "新状态:关" << "灯光数量:" << lightsOnCount;
    }
    
    // 更新主页面灯光数量
    updateMainPageLightStatus();
}

void MainWindow::toggleCurtain(QPushButton* curtainButton)
{
    // 验证按钮有效性
    if (!curtainButton) {
        qDebug() << "错误：无效的窗帘按钮";
        return;
    }
    
    // 获取当前窗帘状态
    bool isOpen = curtainStates[curtainButton];
    qDebug() << "切换窗帘前状态:" << (isOpen ? "开" : "关");
    
    // 切换窗帘状态
    curtainStates[curtainButton] = !isOpen;
    
    if (!isOpen) {
        // 开启窗帘（之前是关闭状态）
        curtainButton->setText("开");
        curtainButton->setStyleSheet("background-color: #FFD700; color: black; font-weight: bold;");
        curtainsOpenCount++;
        qDebug() << "开窗帘:" << curtainButton->objectName() << "新状态:开" << "窗帘数量:" << curtainsOpenCount;
    } else {
        // 关闭窗帘（之前是开启状态）
        curtainButton->setText("关");
        curtainButton->setStyleSheet("");
        curtainsOpenCount--;
        qDebug() << "关窗帘:" << curtainButton->objectName() << "新状态:关" << "窗帘数量:" << curtainsOpenCount;
    }
    
    // 更新主页面窗帘数量
    updateMainPageCurtainStatus();
}

void MainWindow::updateMainPageLightStatus()
{
    // 确保lightsOnCount不会小于0
    if (lightsOnCount < 0) {
        lightsOnCount = 0;
        qDebug() << "修正负数灯光计数:" << lightsOnCount;
    }
    
    // 更新主页面灯光状态标签
    QString lightStatusText = QString("已打开灯光数：%1").arg(lightsOnCount);
    ui->Lightlabel->setText(lightStatusText);
    
    qDebug() << "更新主页面灯光状态:" << lightStatusText;
}

void MainWindow::updateMainPageCurtainStatus()
{
    // 确保curtainsOpenCount不会小于0
    if (curtainsOpenCount < 0) {
        curtainsOpenCount = 0;
        qDebug() << "修正负数窗帘计数:" << curtainsOpenCount;
    }
    
    // 更新主页面窗帘状态标签
    QString curtainStatusText = QString("已打开窗帘数：%1").arg(curtainsOpenCount);
    ui->Curtainlabel->setText(curtainStatusText);
    
    qDebug() << "更新主页面窗帘状态:" << curtainStatusText;
}


void MainWindow::on_LivingroomAcButton_clicked()
{
    // 获取当前空调状态
    QString status = ui->LivingroomAcButton->text();
    qDebug() << "切换空调前状态:" << status;


    if (status == "关") {
        // 开启空调（之前是关闭状态）
        ui->LivingroomAcButton->setText("开");
        ui->LivingroomAcButton->setStyleSheet("background-color: #FFD700; color: black; font-weight: bold;");
        ui->LivingroomAcModecomboBox->setEnabled(true);
        ui->LivingroomTemperaturecomboBox->setEnabled(true);
        qDebug() << "开空调:" << ui->LivingroomAcButton->objectName() << "新状态:开";
    } else if(status == "开"){
        // 关闭空调（之前是开启状态）
        ui->LivingroomAcButton->setText("关");
        ui->LivingroomAcButton->setStyleSheet("");
        ui->LivingroomAcModecomboBox->setEnabled(false);
        ui->LivingroomTemperaturecomboBox->setEnabled(false);
        qDebug() << "关空调:" << ui->LivingroomAcButton->objectName() << "新状态:关" ;
    }
}


void MainWindow::on_BedroomAcButton_clicked()
{
    // 获取当前空调状态
    QString status = ui->BedroomAcButton->text();


    if (status == "关") {
        // 开启空调（之前是关闭状态）
        ui->BedroomAcButton->setText("开");
        ui->BedroomAcButton->setStyleSheet("background-color: #FFD700; color: black; font-weight: bold;");
        ui->BedroomAcModecomboBox->setEnabled(true);
        ui->BedroomTemperaturecomboBox->setEnabled(true);
        qDebug() << "开空调:" << ui->BedroomAcButton->objectName() << "新状态:开";
    } else if(status == "开"){
        // 关闭空调（之前是开启状态）
        ui->BedroomAcButton->setText("关");
        ui->BedroomAcButton->setStyleSheet("");
        ui->BedroomAcModecomboBox->setEnabled(false);
        ui->BedroomTemperaturecomboBox->setEnabled(false);
        qDebug() << "关空调:" << ui->BedroomAcButton->objectName() << "新状态:关" ;
    }
}



void MainWindow::on_AllCloseCurtainButton_clicked()
{
    qDebug() << "执行全关窗帘操作";
    
    // 遍历所有窗帘状态
    for (auto it = curtainStates.begin(); it != curtainStates.end(); ++it) {
        // 如果窗帘当前是开启的
        if (it.value()) {
            // 调用 toggleCurtain 来关闭它
            toggleCurtain(it.key());
        }
    }
}



