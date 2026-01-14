#ifndef USERDEFINEDSCENEDIALOG_H
#define USERDEFINEDSCENEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QComboBox>

class UserDefinedSceneDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserDefinedSceneDialog(QWidget *parent = nullptr);
    ~UserDefinedSceneDialog();

    // 获取用户选择的设备状态
    QMap<QString, int> getSelectedDevices() const; // 0: 保持不变, 1: 开, 2: 关
    // 获取用户输入的场景名称
    QString getSceneName() const;

private slots:
    void onConfirmButtonClicked();
    void onCancelButtonClicked();

private:
    // UI组件
    QLineEdit *sceneNameLineEdit;
    QVBoxLayout *devicesLayout;
    QPushButton *confirmButton;
    QPushButton *cancelButton;

    // 设备下拉框映射
    QMap<QString, QComboBox*> deviceComboBoxes;
    // 设备ID和设备名称的映射
    QMap<QString, QString> deviceIdToName;

    // 初始化UI
    void initUI();
    // 添加设备选择下拉框
    void addDeviceComboBox(const QString &deviceName, const QString &deviceId);
};

#endif // USERDEFINEDSCENEDIALOG_H