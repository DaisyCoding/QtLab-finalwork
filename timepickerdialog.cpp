#include "timepickerdialog.h"

TimePickerDialog::TimePickerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("设置起床时间");
    setMinimumSize(300, 150);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    timeLabel = new QLabel("请选择起床时间:", this);
    mainLayout->addWidget(timeLabel);

    QHBoxLayout *timeLayout = new QHBoxLayout();

    hourSpinBox = new QSpinBox(this);
    hourSpinBox->setRange(0, 23);
    hourSpinBox->setValue(7);
    hourSpinBox->setPrefix("时: ");

    minuteSpinBox = new QSpinBox(this);
    minuteSpinBox->setRange(0, 59);
    minuteSpinBox->setValue(0);
    minuteSpinBox->setPrefix("分: ");

    timeLayout->addWidget(hourSpinBox);
    timeLayout->addWidget(minuteSpinBox);
    mainLayout->addLayout(timeLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *confirmButton = new QPushButton("确认", this);
    QPushButton *cancelButton = new QPushButton("取消", this);

    connect(confirmButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(confirmButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);
}

QTime TimePickerDialog::selectedTime() const
{
    return QTime(hourSpinBox->value(), minuteSpinBox->value());
}

void TimePickerDialog::setSelectedTime(const QTime &time)
{
    hourSpinBox->setValue(time.hour());
    minuteSpinBox->setValue(time.minute());
}
