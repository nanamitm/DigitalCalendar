#include "AndroidCalendarWidget.h"

#include <QApplication>
#include <QEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QTouchEvent>

namespace {
QString monthTitle(const QDate &date)
{
    return QString("%1.%2").arg(date.year()).arg(date.month(), 2, 10, QLatin1Char('0'));
}

QString weekLabel(int index)
{
    static const QStringList labels = { "日", "月", "火", "水", "木", "金", "土" };
    return labels.at(index);
}

QFont fittedFont(const QString &text, const QFont &base, const QRectF &rect, int minPointSize = 7)
{
    QFont font(base);
    for (int size = font.pointSize(); size >= minPointSize; --size) {
        font.setPointSize(size);
        const QFontMetricsF metrics(font);
        if (metrics.horizontalAdvance(text) <= rect.width() && metrics.height() <= rect.height() * 1.08) {
            return font;
        }
    }
    font.setPointSize(minPointSize);
    return font;
}

int scaledPointSize(qreal basis, qreal ratio, int minimum, int maximum)
{
    return qBound(minimum, qRound(basis * ratio), maximum);
}
}

AndroidCalendarWidget::AndroidCalendarWidget(QWidget *parent)
    : QWidget(parent)
    , m_baseMonth(QDate::currentDate().addMonths(-1))
    , m_currentDate(QDate::currentDate())
    , m_currentTime(QTime::currentTime())
{
    setAutoFillBackground(false);
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&m_timer, &QTimer::timeout, this, &AndroidCalendarWidget::updateClock);
    m_timer.start(1000);
}

bool AndroidCalendarWidget::event(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchEnd) {
        auto *touchEvent = static_cast<QTouchEvent *>(event);
        if (!touchEvent->points().isEmpty()) {
            const QPointF position = touchEvent->points().first().position();
            if (event->type() == QEvent::TouchBegin) {
                handlePress(position);
            } else {
                handleRelease(position);
            }
            event->accept();
            return true;
        }
    }
    return QWidget::event(event);
}

void AndroidCalendarWidget::mousePressEvent(QMouseEvent *event)
{
    handlePress(event->position());
}

void AndroidCalendarWidget::mouseReleaseEvent(QMouseEvent *event)
{
    handleRelease(event->position());
}

void AndroidCalendarWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), backgroundColor());

    const qreal outerMargin = qMax(14.0, qMin(width(), height()) * 0.026);
    const QRectF surface = QRectF(rect()).adjusted(outerMargin, outerMargin, -outerMargin, -outerMargin);
    paintSurface(painter, surface);

    const qreal margin = qMax(18.0, qMin(surface.width(), surface.height()) * 0.04);
    const qreal gap = qMax(12.0, qMin(surface.width(), surface.height()) * 0.022);
    const QRectF content = surface.adjusted(margin, margin, -margin, -margin);
    const QRectF header(content.left(), content.top(), content.width(), content.height() * 0.19);
    const QRectF body(content.left(), header.bottom() + gap, content.width(), content.bottom() - header.bottom() - gap);
    const bool landscape = body.width() >= body.height() * 1.12;

    paintHeader(painter, header);

    if (landscape) {
        const qreal sideWidth = body.width() * 0.30;
        const QRectF sideColumn(body.left(), body.top(), sideWidth, body.height());
        const QRectF currentPanel(sideColumn.right() + gap, body.top(), body.right() - sideColumn.right() - gap,
                                  body.height());
        const qreal controlsHeight = qMax(42.0, currentPanel.height() * 0.13);
        const QRectF controls(currentPanel.left(), currentPanel.top(), currentPanel.width(), controlsHeight);
        const QRectF currentMonth(currentPanel.left(), controls.bottom() + gap, currentPanel.width(),
                                  currentPanel.bottom() - controls.bottom() - gap);
        const qreal miniGap = qMax(10.0, sideColumn.height() * 0.035);
        const qreal miniHeight = (sideColumn.height() - miniGap) / 2.0;

        paintControls(painter, controls);
        paintMonth(painter, MonthLayout { QRectF(sideColumn.left(), sideColumn.top(), sideColumn.width(), miniHeight),
                                          firstDayOfVisibleMonth(0), false });
        paintMonth(painter, MonthLayout { QRectF(sideColumn.left(), sideColumn.top() + miniHeight + miniGap,
                                                 sideColumn.width(), miniHeight),
                                          firstDayOfVisibleMonth(2), false });
        paintMonth(painter, MonthLayout { currentMonth, firstDayOfVisibleMonth(1), true });
    } else {
        const qreal controlsHeight = qMax(42.0, body.height() * 0.09);
        const QRectF controls(body.left(), body.top(), body.width(), controlsHeight);
        const qreal smallHeight = qMax(112.0, body.height() * 0.30);
        const QRectF currentMonth(body.left(), controls.bottom() + gap, body.width(),
                                  body.height() - controlsHeight - smallHeight - gap * 2.0);
        const QRectF previous(body.left(), currentMonth.bottom() + gap, (body.width() - gap) / 2.0, smallHeight);
        const QRectF next(previous.right() + gap, previous.top(), previous.width(), smallHeight);

        paintControls(painter, controls);
        paintMonth(painter, MonthLayout { currentMonth, firstDayOfVisibleMonth(1), true });
        paintMonth(painter, MonthLayout { previous, firstDayOfVisibleMonth(0), false });
        paintMonth(painter, MonthLayout { next, firstDayOfVisibleMonth(2), false });
    }
}

void AndroidCalendarWidget::resizeEvent(QResizeEvent *)
{
    update();
}

void AndroidCalendarWidget::showPreviousMonth()
{
    m_baseMonth = m_baseMonth.addMonths(-1);
    update();
}

void AndroidCalendarWidget::showNextMonth()
{
    m_baseMonth = m_baseMonth.addMonths(1);
    update();
}

void AndroidCalendarWidget::showToday()
{
    m_currentDate = QDate::currentDate();
    m_baseMonth = m_currentDate.addMonths(-1);
    update();
}

void AndroidCalendarWidget::toggleTimeMode()
{
    m_twentyFourHour = !m_twentyFourHour;
    update();
}

void AndroidCalendarWidget::cycleThemeMode()
{
    switch (m_themeMode) {
    case ThemeMode::System:
        m_themeMode = ThemeMode::Light;
        break;
    case ThemeMode::Light:
        m_themeMode = ThemeMode::Dark;
        break;
    case ThemeMode::Dark:
        m_themeMode = ThemeMode::System;
        break;
    }
    update();
}

void AndroidCalendarWidget::updateClock()
{
    m_currentTime = QTime::currentTime();
    const QDate today = QDate::currentDate();
    if (today != m_currentDate) {
        m_currentDate = today;
        m_baseMonth = m_currentDate.addMonths(-1);
    }
    update();
}

void AndroidCalendarWidget::handlePress(const QPointF &position)
{
    m_pressPosition = position;
    m_pressActive = true;
}

void AndroidCalendarWidget::handleRelease(const QPointF &position)
{
    if (!m_pressActive) {
        return;
    }
    m_pressActive = false;

    const QPointF delta = position - m_pressPosition;
    const qreal swipeThreshold = qMax(80.0, width() * 0.12);
    if (qAbs(delta.x()) > swipeThreshold && qAbs(delta.x()) > qAbs(delta.y()) * 1.35) {
        if (delta.x() < 0) {
            showNextMonth();
        } else {
            showPreviousMonth();
        }
        return;
    }

    if (m_timeHitRect.contains(position)) {
        toggleTimeMode();
    } else if (m_themeHitRect.contains(position)) {
        cycleThemeMode();
    } else if (m_prevHitRect.contains(position)) {
        showPreviousMonth();
    } else if (m_todayHitRect.contains(position)) {
        showToday();
    } else if (m_nextHitRect.contains(position)) {
        showNextMonth();
    }
}

void AndroidCalendarWidget::paintSurface(QPainter &painter, const QRectF &area)
{
    painter.save();

    painter.setPen(Qt::NoPen);
    painter.setBrush(isDarkMode() ? QColor(0, 0, 0, 95) : QColor(31, 36, 38, 24));
    painter.drawRoundedRect(area.translated(0, 4), 12, 12);

    QLinearGradient fill(area.topLeft(), area.bottomLeft());
    fill.setColorAt(0.0, surfaceTopColor());
    fill.setColorAt(1.0, surfaceBottomColor());
    painter.setBrush(fill);
    painter.setPen(QPen(surfaceBorderColor(), 1));
    painter.drawRoundedRect(area, 12, 12);

    painter.restore();
}

void AndroidCalendarWidget::paintHeader(QPainter &painter, const QRectF &area)
{
    painter.save();

    const QString dateText = m_currentDate.toString("yyyy.MM.dd ddd");
    const QString timeText = m_twentyFourHour ? m_currentTime.toString("HH:mm") : m_currentTime.toString("h:mm");
    const QString secondsText = m_currentTime.toString("ss");
    const QString periodText = m_twentyFourHour ? QString() : m_currentTime.toString("AP");
    const bool compact = area.width() < area.height() * 5.2;

    QRectF dateArea;
    QRectF timeArea;
    QRectF secondsArea;
    QRectF periodArea;
    if (compact) {
        dateArea = QRectF(area.left(), area.top(), area.width(), area.height() * 0.44);
        timeArea = QRectF(area.left(), dateArea.bottom(), area.width() * 0.76, area.bottom() - dateArea.bottom());
        secondsArea = QRectF(timeArea.right(), timeArea.top() + timeArea.height() * 0.42,
                             area.width() * 0.10, timeArea.height() * 0.38);
        periodArea = QRectF(secondsArea.right() + 4.0, secondsArea.top(), area.right() - secondsArea.right() - 4.0,
                            secondsArea.height());
    } else {
        dateArea = QRectF(area.left(), area.top(), area.width() * 0.50, area.height());
        timeArea = QRectF(area.left() + area.width() * 0.44, area.top(), area.width() * 0.40, area.height());
        secondsArea = QRectF(timeArea.right(), area.top() + area.height() * 0.45, area.width() * 0.065,
                             area.height() * 0.34);
        periodArea = QRectF(secondsArea.right() + 4.0, secondsArea.top(), area.right() - secondsArea.right() - 4.0,
                            secondsArea.height());
    }
    m_timeHitRect = QRectF(timeArea.left(), timeArea.top(), periodArea.right() - timeArea.left(), timeArea.height());

    painter.setPen(mutedTextColor());
    QFont dateFont("Noto Sans CJK JP", scaledPointSize(dateArea.height(), compact ? 0.52 : 0.48, 15, 58),
                   QFont::DemiBold);
    painter.setFont(fittedFont(dateText, dateFont, dateArea, 9));
    painter.drawText(dateArea, Qt::AlignLeft | Qt::AlignVCenter, dateText);

    painter.setPen(textColor());
    QFont timeFont("Noto Sans", scaledPointSize(timeArea.height(), compact ? 0.86 : 0.92, 34, 156),
                   QFont::DemiBold);
    painter.setFont(fittedFont(timeText, timeFont, timeArea, 18));
    painter.drawText(timeArea, Qt::AlignRight | Qt::AlignVCenter, timeText);

    painter.setPen(accentColor());
    QFont secondsFont("Noto Sans", scaledPointSize(secondsArea.height(), 0.86, 13, 52), QFont::DemiBold);
    painter.setFont(fittedFont(secondsText, secondsFont, secondsArea, 8));
    painter.drawText(secondsArea, Qt::AlignLeft | Qt::AlignVCenter, secondsText);

    if (!periodText.isEmpty()) {
        painter.setPen(mutedTextColor());
        QFont periodFont("Noto Sans", scaledPointSize(periodArea.height(), 0.66, 9, 34), QFont::DemiBold);
        painter.setFont(fittedFont(periodText, periodFont, periodArea, 7));
        painter.drawText(periodArea, Qt::AlignLeft | Qt::AlignVCenter, periodText);
    }

    painter.restore();
}

void AndroidCalendarWidget::paintControls(QPainter &painter, const QRectF &area)
{
    painter.save();

    const qreal buttonGap = qMax(8.0, area.width() * 0.012);
    const qreal buttonHeight = qMin(area.height(), qMax(40.0, area.height() * 0.84));
    const qreal y = area.center().y() - buttonHeight / 2.0;
    const qreal smallWidth = qMax(52.0, qMin(78.0, area.width() * 0.14));
    const qreal todayWidth = qMax(86.0, qMin(124.0, area.width() * 0.21));
    const qreal themeWidth = qMax(98.0, qMin(146.0, area.width() * 0.24));
    const qreal totalWidth = themeWidth + smallWidth * 2.0 + todayWidth + buttonGap * 3.0;
    const qreal x = area.right() - totalWidth;

    m_themeHitRect = QRectF(x, y, themeWidth, buttonHeight);
    m_prevHitRect = QRectF(m_themeHitRect.right() + buttonGap, y, smallWidth, buttonHeight);
    m_todayHitRect = QRectF(m_prevHitRect.right() + buttonGap, y, todayWidth, buttonHeight);
    m_nextHitRect = QRectF(m_todayHitRect.right() + buttonGap, y, smallWidth, buttonHeight);

    const QString label = monthTitle(firstDayOfVisibleMonth(1));
    painter.setPen(mutedTextColor());
    QFont labelFont("Noto Sans", scaledPointSize(area.height(), 0.42, 12, 30), QFont::DemiBold);
    painter.setFont(fittedFont(label, labelFont, QRectF(area.left(), area.top(), x - area.left() - 10.0,
                                                       area.height()),
                               9));
    painter.drawText(QRectF(area.left(), area.top(), x - area.left() - 10.0, area.height()),
                     Qt::AlignLeft | Qt::AlignVCenter, label);

    const QList<QPair<QRectF, QString>> buttons = {
        { m_themeHitRect, themeLabel() },
        { m_prevHitRect, "<" },
        { m_todayHitRect, "Today" },
        { m_nextHitRect, ">" },
    };

    for (const auto &button : buttons) {
        painter.setPen(QPen(monthBorderColor(false), 1));
        painter.setBrush(monthFillColor(false));
        painter.drawRoundedRect(button.first, 8, 8);
        painter.setPen(accentColor());
        QFont buttonFont("Noto Sans", scaledPointSize(button.first.height(), 0.43, 11, 22), QFont::DemiBold);
        painter.setFont(fittedFont(button.second, buttonFont, button.first.adjusted(5, 0, -5, 0), 8));
        painter.drawText(button.first, Qt::AlignCenter, button.second);
    }

    painter.restore();
}

void AndroidCalendarWidget::paintMonthFrame(QPainter &painter, const QRectF &area, bool current)
{
    painter.save();
    painter.setPen(QPen(monthBorderColor(current), current ? 1.6 : 1.0));
    painter.setBrush(monthFillColor(current));
    painter.drawRoundedRect(area, 10, 10);
    painter.restore();
}

void AndroidCalendarWidget::paintMonth(QPainter &painter, const MonthLayout &layout)
{
    painter.save();
    paintMonthFrame(painter, layout.area, layout.current);

    const qreal inset = qMax(9.0, qMin(layout.area.width(), layout.area.height()) * 0.04);
    const QRectF area = layout.area.adjusted(inset, inset * 0.85, -inset, -inset);
    const qreal titleHeight = area.height() * (layout.current ? 0.15 : 0.17);
    const qreal weekHeight = area.height() * (layout.current ? 0.12 : 0.13);
    const QColor text = textColor();
    const QColor muted = mutedTextColor();

    QFont titleFont("Noto Sans",
                    scaledPointSize(titleHeight, layout.current ? 0.70 : 0.58,
                                    layout.current ? 14 : 9, layout.current ? 42 : 24),
                    QFont::DemiBold);
    painter.setFont(titleFont);
    painter.setPen(layout.current ? accentColor() : text);
    painter.drawText(QRectF(area.left(), area.top(), area.width(), titleHeight),
                     Qt::AlignLeft | Qt::AlignVCenter, monthTitle(layout.month));

    const QRectF weekRow(area.left(), area.top() + titleHeight, area.width(), weekHeight);
    QFont weekFont("Noto Sans CJK JP",
                   scaledPointSize(weekHeight, layout.current ? 0.58 : 0.50,
                                   layout.current ? 8 : 7, layout.current ? 20 : 14),
                   QFont::DemiBold);
    painter.setFont(weekFont);
    for (int i = 0; i < 7; ++i) {
        const QRectF cell(weekRow.left() + weekRow.width() / 7.0 * i, weekRow.top(), weekRow.width() / 7.0,
                          weekRow.height());
        painter.setPen(i == 0 ? sundayColor() : muted);
        painter.drawText(cell, Qt::AlignCenter, weekLabel(i));
    }

    const QRectF grid(area.left(), weekRow.bottom() + 3, area.width(), area.bottom() - weekRow.bottom() - 3);
    const qreal cellW = grid.width() / 7.0;
    const qreal cellH = grid.height() / 6.0;
    QFont dayFont("Noto Sans",
                  scaledPointSize(cellH, layout.current ? 0.44 : 0.37,
                                  layout.current ? 12 : 7, layout.current ? 42 : 22),
                  QFont::DemiBold);
    painter.setFont(dayFont);

    const int firstColumn = layout.month.dayOfWeek() % 7;
    const int days = layout.month.daysInMonth();
    for (int day = 1; day <= days; ++day) {
        const int index = firstColumn + day - 1;
        const int row = index / 7;
        const int column = index % 7;
        const QRectF cell(grid.left() + cellW * column, grid.top() + cellH * row, cellW, cellH);
        const QDate date(layout.month.year(), layout.month.month(), day);
        const bool isToday = date == m_currentDate;

        if (isToday) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(accentColor());
            const qreal size = qMin(cell.width(), cell.height()) * 0.82;
            painter.drawEllipse(QRectF(cell.center().x() - size / 2.0, cell.center().y() - size / 2.0, size, size));
        }

        if (isToday) {
            painter.setPen(isDarkMode() ? QColor(7, 15, 16) : Qt::white);
        } else if (column == 0) {
            painter.setPen(sundayColor());
        } else if (!layout.current) {
            painter.setPen(inactiveDayColor());
        } else {
            painter.setPen(text);
        }
        painter.drawText(cell, Qt::AlignCenter, QString::number(day));
    }

    painter.restore();
}

QDate AndroidCalendarWidget::firstDayOfVisibleMonth(int offset) const
{
    const QDate date = m_baseMonth.addMonths(offset);
    return QDate(date.year(), date.month(), 1);
}

bool AndroidCalendarWidget::isDarkMode() const
{
    if (m_themeMode == ThemeMode::Light) {
        return false;
    }
    if (m_themeMode == ThemeMode::Dark) {
        return true;
    }
    return palette().color(QPalette::Window).lightness() < 128;
}

QString AndroidCalendarWidget::themeLabel() const
{
    switch (m_themeMode) {
    case ThemeMode::System:
        return "Theme: Auto";
    case ThemeMode::Light:
        return "Theme: Light";
    case ThemeMode::Dark:
        return "Theme: Dark";
    }
    return "Theme";
}

QColor AndroidCalendarWidget::backgroundColor() const
{
    return isDarkMode() ? QColor(15, 18, 19) : QColor(246, 247, 244);
}

QColor AndroidCalendarWidget::surfaceTopColor() const
{
    return isDarkMode() ? QColor(42, 48, 50) : QColor(255, 255, 252);
}

QColor AndroidCalendarWidget::surfaceBottomColor() const
{
    return isDarkMode() ? QColor(25, 30, 31) : QColor(236, 241, 239);
}

QColor AndroidCalendarWidget::surfaceBorderColor() const
{
    return isDarkMode() ? QColor(72, 83, 84) : QColor(204, 213, 210);
}

QColor AndroidCalendarWidget::monthFillColor(bool current) const
{
    if (isDarkMode()) {
        return current ? QColor(30, 48, 49) : QColor(32, 38, 39);
    }
    return current ? QColor(234, 244, 242) : QColor(249, 250, 248);
}

QColor AndroidCalendarWidget::monthBorderColor(bool current) const
{
    if (isDarkMode()) {
        return current ? QColor(92, 171, 170) : QColor(68, 78, 79);
    }
    return current ? QColor(72, 119, 121) : QColor(212, 220, 217);
}

QColor AndroidCalendarWidget::accentColor() const
{
    return isDarkMode() ? QColor(118, 214, 211) : QColor(35, 79, 82);
}

QColor AndroidCalendarWidget::sundayColor() const
{
    return isDarkMode() ? QColor(238, 137, 128) : QColor(173, 73, 66);
}

QColor AndroidCalendarWidget::inactiveDayColor() const
{
    return isDarkMode() ? QColor(152, 167, 167) : QColor(86, 99, 99);
}

QColor AndroidCalendarWidget::textColor() const
{
    return isDarkMode() ? QColor(232, 238, 237) : QColor(34, 43, 45);
}

QColor AndroidCalendarWidget::mutedTextColor() const
{
    return isDarkMode() ? QColor(170, 184, 183) : QColor(94, 108, 108);
}
