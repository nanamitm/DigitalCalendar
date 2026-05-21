#pragma once

#include <QDate>
#include <QTimer>
#include <QTime>
#include <QWidget>

class AndroidCalendarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AndroidCalendarWidget(QWidget *parent = nullptr);

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    enum class ThemeMode {
        System,
        Light,
        Dark
    };

    struct MonthLayout {
        QRectF area;
        QDate month;
        bool current = false;
    };

    void showPreviousMonth();
    void showNextMonth();
    void showToday();
    void toggleTimeMode();
    void cycleThemeMode();
    void updateClock();

    void handlePress(const QPointF &position);
    void handleRelease(const QPointF &position);

    void paintSurface(QPainter &painter, const QRectF &area);
    void paintHeader(QPainter &painter, const QRectF &area);
    void paintControls(QPainter &painter, const QRectF &area);
    void paintMonth(QPainter &painter, const MonthLayout &layout);
    void paintMonthFrame(QPainter &painter, const QRectF &area, bool current);

    QDate firstDayOfVisibleMonth(int offset) const;
    bool isDarkMode() const;
    QString themeLabel() const;
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
    QPointF m_pressPosition;
    bool m_pressActive = false;
    ThemeMode m_themeMode = ThemeMode::System;
    QRectF m_timeHitRect;
    QRectF m_themeHitRect;
    QRectF m_prevHitRect;
    QRectF m_todayHitRect;
    QRectF m_nextHitRect;
};
