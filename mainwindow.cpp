#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

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

