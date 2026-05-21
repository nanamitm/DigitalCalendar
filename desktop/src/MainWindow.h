#pragma once

#include <QMainWindow>

class CalendarWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    CalendarWidget *m_calendar = nullptr;
};
