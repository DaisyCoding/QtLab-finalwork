#ifndef TIMEPICKERDIALOG_H
#define TIMEPICKERDIALOG_H

#include <QDialog>
#include <QTime>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TimePickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimePickerDialog(QWidget *parent = nullptr);
    QTime selectedTime() const;
    void setSelectedTime(const QTime &time);

private:
    QSpinBox *hourSpinBox;
    QSpinBox *minuteSpinBox;
    QLabel *timeLabel;
};

#endif // TIMEPICKERDIALOG_H
