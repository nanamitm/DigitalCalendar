#pragma once

#include <QDate>
#include <QTimer>
#include <QTime>
#include <QWidget>

class CalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalendarWidget(QWidget *parent = nullptr);

public slots:
    void showPreviousMonth();
    void showNextMonth();
    void showToday();
    void setTwentyFourHourMode(bool enabled);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void setAlwaysOnTop(bool enabled);
    struct MonthLayout {
        QRectF area;
        QDate month;
        bool current = false;
    };

    void updateClock();
    void paintSurface(QPainter &painter, const QRectF &surfaceRect);
    void paintHeader(QPainter &painter, const QRectF &area);
    void paintMonth(QPainter &painter, const MonthLayout &layout);
    void paintMonthFrame(QPainter &painter, const QRectF &area, bool current);
    void paintMonthControls(QPainter &painter, const QRectF &area);

    QDate firstDayOfVisibleMonth(int offset) const;
    bool isDarkMode() const;
    QColor backgroundColor() const;
    QColor surfaceTopColor() const;
    QColor surfaceBottomColor() const;
    QColor surfaceBorderColor() const;
    QColor monthFillColor(bool current) const;
    QColor monthBorderColor(bool current) const;
    QColor accentColor() const;
    QColor sundayColor() const;
    QColor inactiveDayColor() const;
    QColor textColor() const;
    QColor mutedTextColor() const;

    QDate m_baseMonth;
    QDate m_currentDate;
    QTime m_currentTime;
    QTimer m_timer;
    bool m_twentyFourHour = true;
    QRectF m_timeHitRect;
    QRectF m_prevHitRect;
    QRectF m_todayHitRect;
    QRectF m_nextHitRect;
};
