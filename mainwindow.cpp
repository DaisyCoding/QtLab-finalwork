#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupConnections();

    statusTimeLabel.setMidLineWidth(200);
    statusTimeLabel.setText("TIME");

    statusTemperatureLabel.setMidLineWidth(200);
    statusTemperatureLabel.setText("temperature");

    statusWeatherLabel.setMidLineWidth(200);
    statusWeatherLabel.setText("weather");

    ui->statusbar->addPermanentWidget(&statusTimeLabel);
    ui->statusbar->addPermanentWidget(&statusWeatherLabel);
    ui->statusbar->addPermanentWidget(&statusTemperatureLabel);
    ui->statusbar->setMinimumHeight(50);
}

MainWindow::~MainWindow()
{
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

