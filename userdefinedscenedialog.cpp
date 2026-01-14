#include "userdefinedscenedialog.h"
#include <QDebug>

UserDefinedSceneDialog::UserDefinedSceneDialog(QWidget *parent)
    : QDialog(parent)
{
    initUI();
}

UserDefinedSceneDialog::~UserDefinedSceneDialog()
{
}

void UserDefinedSceneDialog::initUI()
{
    // 设置窗口标题
    setWindowTitle("自定义场景模式");
    // 设置窗口大小
    resize(400, 500);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 场景名称输入
    QHBoxLayout *nameLayout = new QHBoxLayout();
    QLabel *nameLabel = new QLabel("场景名称:", this);
    sceneNameLineEdit = new QLineEdit(this);
    sceneNameLineEdit->setPlaceholderText("请输入场景名称");
    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(sceneNameLineEdit);
    mainLayout->addLayout(nameLayout);

    // 设备选择区域
    QLabel *devicesLabel = new QLabel("选择设备:", this);
    mainLayout->addWidget(devicesLabel);

    devicesLayout = new QVBoxLayout();
    mainLayout->addLayout(devicesLayout);

    // 添加灯光设备
    QLabel *lightLabel = new QLabel("灯光设备:", this);
    lightLabel->setStyleSheet("font-weight: bold;");
    devicesLayout->addWidget(lightLabel);
    addDeviceComboBox("客厅灯", "LivingroomLight");
    addDeviceComboBox("厨房灯", "KitchenLight");
    addDeviceComboBox("卧室灯", "BedroomLight");
    addDeviceComboBox("浴室灯", "BathroomLight");
    addDeviceComboBox("书房灯", "StudyroomLight");
    addDeviceComboBox("阳台灯", "BalconyLight");
    addDeviceComboBox("餐厅灯", "DiningroomLight");

    // 添加窗帘设备
    QLabel *curtainLabel = new QLabel("窗帘设备:", this);
    curtainLabel->setStyleSheet("font-weight: bold;");
    devicesLayout->addWidget(curtainLabel);
    addDeviceComboBox("客厅窗帘", "LivingroomCurtain");
    addDeviceComboBox("卧室窗帘", "BedroomCurtain");

    // 添加空调设备
    QLabel *acLabel = new QLabel("空调设备:", this);
    acLabel->setStyleSheet("font-weight: bold;");
    devicesLayout->addWidget(acLabel);
    addDeviceComboBox("客厅空调", "LivingroomAc");
    addDeviceComboBox("卧室空调", "BedroomAc");

    // 添加门锁设备
    QLabel *lockLabel = new QLabel("门锁设备:", this);
    lockLabel->setStyleSheet("font-weight: bold;");
    devicesLayout->addWidget(lockLabel);
    addDeviceComboBox("门锁", "Lock");

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    confirmButton = new QPushButton("确认", this);
    cancelButton = new QPushButton("取消", this);
    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    // 连接信号槽
    connect(confirmButton, &QPushButton::clicked, this, &UserDefinedSceneDialog::onConfirmButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &UserDefinedSceneDialog::onCancelButtonClicked);
}

void UserDefinedSceneDialog::addDeviceComboBox(const QString &deviceName, const QString &deviceId)
{
    // 创建一个水平布局来放置设备名称和下拉框
    QHBoxLayout *deviceLayout = new QHBoxLayout();
    
    // 创建设备名称标签
    QLabel *deviceLabel = new QLabel(deviceName + ":", this);
    
    // 创建下拉框
    QComboBox *comboBox = new QComboBox(this);
    comboBox->addItem("保持不变", 0);
    comboBox->addItem("开", 1);
    comboBox->addItem("关", 2);
    
    // 将标签和下拉框添加到水平布局
    deviceLayout->addWidget(deviceLabel);
    deviceLayout->addWidget(comboBox);
    
    // 将水平布局添加到设备选择区域
    devicesLayout->addLayout(deviceLayout);
    
    // 保存设备ID和下拉框的映射关系
    deviceComboBoxes.insert(deviceId, comboBox);
    // 保存设备ID和设备名称的映射关系
    deviceIdToName.insert(deviceId, deviceName);
}

QMap<QString, int> UserDefinedSceneDialog::getSelectedDevices() const
{
    QMap<QString, int> selectedDevices;
    for (auto it = deviceComboBoxes.constBegin(); it != deviceComboBoxes.constEnd(); ++it) {
        QString deviceId = it.key();
        QString deviceName = deviceIdToName.value(deviceId, deviceId); // 使用设备名称，如果没有则使用设备ID
        int status = it.value()->currentData().toInt();
        selectedDevices.insert(deviceName, status);
    }
    return selectedDevices;
}

QString UserDefinedSceneDialog::getSceneName() const
{
    return sceneNameLineEdit->text().trimmed();
}

void UserDefinedSceneDialog::onConfirmButtonClicked()
{
    QString sceneName = getSceneName();
    if (sceneName.isEmpty()) {
        qDebug() << "场景名称不能为空";
        return;
    }
    accept();
}

void UserDefinedSceneDialog::onCancelButtonClicked()
{
    reject();
}