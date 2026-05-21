#include "MainWindow.h"

#include "CalendarWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_calendar(new CalendarWidget(this))
{
    setWindowTitle(tr("Desktop Calendar"));
    setMinimumSize(520, 320);
    resize(680, 410);

    setCentralWidget(m_calendar);
}
