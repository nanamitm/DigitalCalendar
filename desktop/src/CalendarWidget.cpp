#include "CalendarWidget.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QLinearGradient>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>

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

CalendarWidget::CalendarWidget(QWidget *parent)
    : QWidget(parent)
    , m_baseMonth(QDate::currentDate().addMonths(-1))
    , m_currentDate(QDate::currentDate())
    , m_currentTime(QTime::currentTime())
{
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&m_timer, &QTimer::timeout, this, &CalendarWidget::updateClock);
    m_timer.start(1000);
}

void CalendarWidget::showPreviousMonth()
{
    m_baseMonth = m_baseMonth.addMonths(-1);
    update();
}

void CalendarWidget::showNextMonth()
{
    m_baseMonth = m_baseMonth.addMonths(1);
    update();
}

void CalendarWidget::showToday()
{
    m_currentDate = QDate::currentDate();
    m_baseMonth = m_currentDate.addMonths(-1);
    update();
}

void CalendarWidget::setTwentyFourHourMode(bool enabled)
{
    m_twentyFourHour = enabled;
    update();
}

void CalendarWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), backgroundColor());

    const QRectF surface = QRectF(rect()).adjusted(14, 14, -14, -14);
    paintSurface(painter, surface);

    const qreal margin = qMax(18.0, surface.width() * 0.035);
    const qreal gap = qMax(10.0, surface.width() * 0.018);
    const QRectF content = surface.adjusted(margin, margin, -margin, -margin);
    const QRectF header(content.left(), content.top(), content.width(), content.height() * 0.20);
    const QRectF body(content.left(), header.bottom() + gap, content.width(),
                      content.bottom() - header.bottom() - gap);
    const qreal sideWidth = body.width() * 0.285;
    const QRectF sideColumn(body.left(), body.top(), sideWidth, body.height());
    const QRectF currentPanelArea(sideColumn.right() + gap, body.top(),
                                  body.right() - sideColumn.right() - gap, body.height());
    const qreal controlsHeight = qMax(28.0, currentPanelArea.height() * 0.13);
    const QRectF currentControlsArea(currentPanelArea.left(), currentPanelArea.top(),
                                     currentPanelArea.width(), controlsHeight);
    const QRectF currentMonthArea(currentPanelArea.left(), currentControlsArea.bottom() + gap,
                                  currentPanelArea.width(), currentPanelArea.bottom() - currentControlsArea.bottom() - gap);
    const qreal miniGap = qMax(8.0, sideColumn.height() * 0.035);
    const qreal miniHeight = (sideColumn.height() - miniGap) / 2.0;

    paintHeader(painter, header);
    paintMonthControls(painter, currentControlsArea);
    paintMonth(painter, MonthLayout { QRectF(sideColumn.left(), sideColumn.top(), sideColumn.width(), miniHeight),
                                      firstDayOfVisibleMonth(0), false });
    paintMonth(painter, MonthLayout { QRectF(sideColumn.left(), sideColumn.top() + miniHeight + miniGap,
                                             sideColumn.width(), miniHeight),
                                      firstDayOfVisibleMonth(2), false });
    paintMonth(painter, MonthLayout { currentMonthArea, firstDayOfVisibleMonth(1), true });
}

void CalendarWidget::resizeEvent(QResizeEvent *)
{
    update();
}

void CalendarWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QWidget *topLevel = window();
    const bool isAlwaysOnTop = topLevel->windowFlags().testFlag(Qt::WindowStaysOnTopHint);

    QMenu menu(this);
    QAction *alwaysOnTopAction = menu.addAction(tr("Always on top"));
    alwaysOnTopAction->setCheckable(true);
    alwaysOnTopAction->setChecked(isAlwaysOnTop);

    QAction *selected = menu.exec(event->globalPos());
    if (selected == alwaysOnTopAction) {
        setAlwaysOnTop(alwaysOnTopAction->isChecked());
    }
}

void CalendarWidget::mousePressEvent(QMouseEvent *event)
{
    const QPointF position = event->position();
    if (m_timeHitRect.contains(position)) {
        setTwentyFourHourMode(!m_twentyFourHour);
        return;
    }
    if (m_prevHitRect.contains(position)) {
        showPreviousMonth();
        return;
    }
    if (m_todayHitRect.contains(position)) {
        showToday();
        return;
    }
    if (m_nextHitRect.contains(position)) {
        showNextMonth();
        return;
    }

    QWidget::mousePressEvent(event);
}

void CalendarWidget::updateClock()
{
    m_currentTime = QTime::currentTime();
    const QDate today = QDate::currentDate();
    if (today != m_currentDate) {
        m_currentDate = today;
        m_baseMonth = m_currentDate.addMonths(-1);
    }
    update();
}

void CalendarWidget::setAlwaysOnTop(bool enabled)
{
    QWidget *topLevel = window();
    Qt::WindowFlags flags = topLevel->windowFlags();
    if (enabled) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }

    const QPoint position = topLevel->pos();
    const QSize size = topLevel->size();
    topLevel->setWindowFlags(flags);
    topLevel->resize(size);
    topLevel->move(position);
    topLevel->show();
    topLevel->raise();
    topLevel->activateWindow();
}

void CalendarWidget::paintSurface(QPainter &painter, const QRectF &surfaceRect)
{
    painter.save();

    painter.setPen(Qt::NoPen);
    painter.setBrush(isDarkMode() ? QColor(0, 0, 0, 90) : QColor(31, 36, 38, 24));
    painter.drawRoundedRect(surfaceRect.translated(0, 3), 8, 8);

    QLinearGradient fill(surfaceRect.topLeft(), surfaceRect.bottomLeft());
    fill.setColorAt(0.0, surfaceTopColor());
    fill.setColorAt(1.0, surfaceBottomColor());
    painter.setBrush(fill);
    painter.setPen(QPen(surfaceBorderColor(), 1));
    painter.drawRoundedRect(surfaceRect, 8, 8);

    painter.restore();
}

void CalendarWidget::paintHeader(QPainter &painter, const QRectF &area)
{
    painter.save();

    const QString dateText = m_currentDate.toString("yyyy.MM.dd ddd");
    const QString timeText = m_twentyFourHour ? m_currentTime.toString("HH:mm") : m_currentTime.toString("h:mm");
    const QString secondsText = m_currentTime.toString("ss");
    const QString periodText = m_twentyFourHour ? QString() : m_currentTime.toString("AP");

    const QRectF dateArea(area.left(), area.top(), area.width() * 0.50, area.height());
    const QRectF timeArea(area.left() + area.width() * 0.44, area.top(), area.width() * 0.40, area.height());
    const QRectF secondsArea(timeArea.right(), area.top() + area.height() * 0.45,
                             area.width() * 0.065, area.height() * 0.34);
    const QRectF periodArea(secondsArea.right() + 3.0, secondsArea.top(),
                            area.right() - secondsArea.right() - 3.0, secondsArea.height());
    m_timeHitRect = QRectF(timeArea.left(), timeArea.top(), periodArea.right() - timeArea.left(), timeArea.height());

    painter.setPen(mutedTextColor());
    QFont dateFont("Meiryo UI", scaledPointSize(dateArea.height(), 0.48, 14, 56), QFont::DemiBold);
    painter.setFont(fittedFont(dateText, dateFont, dateArea, 8));
    painter.drawText(dateArea, Qt::AlignLeft | Qt::AlignVCenter, dateText);

    painter.setPen(textColor());
    QFont timeFont("Segoe UI", scaledPointSize(timeArea.height(), 0.92, 32, 150), QFont::DemiBold);
    painter.setFont(fittedFont(timeText, timeFont, timeArea, 16));
    painter.drawText(timeArea, Qt::AlignRight | Qt::AlignVCenter, timeText);

    painter.setPen(accentColor());
    QFont secondsFont("Segoe UI", scaledPointSize(secondsArea.height(), 0.86, 13, 52), QFont::DemiBold);
    painter.setFont(fittedFont(secondsText, secondsFont, secondsArea, 8));
    painter.drawText(secondsArea, Qt::AlignLeft | Qt::AlignVCenter, secondsText);

    if (!periodText.isEmpty()) {
        painter.setPen(mutedTextColor());
        QFont periodFont("Segoe UI", scaledPointSize(periodArea.height(), 0.66, 9, 34), QFont::DemiBold);
        painter.setFont(fittedFont(periodText, periodFont, periodArea, 7));
        painter.drawText(periodArea, Qt::AlignLeft | Qt::AlignVCenter, periodText);
    }

    painter.restore();
}

void CalendarWidget::paintMonthControls(QPainter &painter, const QRectF &area)
{
    painter.save();

    const qreal buttonGap = 6.0;
    const qreal buttonHeight = qMin(30.0, area.height());
    const qreal y = area.center().y() - buttonHeight / 2.0;
    const qreal smallWidth = qMin(42.0, area.width() * 0.18);
    const qreal todayWidth = qMin(76.0, area.width() * 0.27);
    const qreal totalWidth = smallWidth * 2.0 + todayWidth + buttonGap * 2.0;
    const qreal x = area.right() - totalWidth;

    m_prevHitRect = QRectF(x, y, smallWidth, buttonHeight);
    m_todayHitRect = QRectF(m_prevHitRect.right() + buttonGap, y, todayWidth, buttonHeight);
    m_nextHitRect = QRectF(m_todayHitRect.right() + buttonGap, y, smallWidth, buttonHeight);

    painter.setPen(mutedTextColor());
    QFont labelFont("Segoe UI", scaledPointSize(area.height(), 0.34, 9, 18), QFont::DemiBold);
    painter.setFont(fittedFont("Current month", labelFont,
                               QRectF(area.left(), area.top(), x - area.left() - 8.0, area.height()), 8));
    painter.drawText(QRectF(area.left(), area.top(), x - area.left() - 8.0, area.height()),
                     Qt::AlignLeft | Qt::AlignVCenter, "Current month");

    const QList<QPair<QRectF, QString>> buttons = {
        { m_prevHitRect, "<" },
        { m_todayHitRect, "Today" },
        { m_nextHitRect, ">" },
    };

    for (const auto &button : buttons) {
        painter.setPen(QPen(monthBorderColor(false), 1));
        painter.setBrush(monthFillColor(false));
        painter.drawRoundedRect(button.first, 5, 5);
        painter.setPen(accentColor());
        QFont buttonFont("Segoe UI", scaledPointSize(button.first.height(), 0.42, 9, 17), QFont::DemiBold);
        painter.setFont(fittedFont(button.second, buttonFont, button.first.adjusted(4, 0, -4, 0), 7));
        painter.drawText(button.first, Qt::AlignCenter, button.second);
    }

    painter.restore();
}

void CalendarWidget::paintMonthFrame(QPainter &painter, const QRectF &area, bool current)
{
    painter.save();
    painter.setPen(QPen(monthBorderColor(current), current ? 1.4 : 1.0));
    painter.setBrush(monthFillColor(current));
    painter.drawRoundedRect(area, 7, 7);
    painter.restore();
}

void CalendarWidget::paintMonth(QPainter &painter, const MonthLayout &layout)
{
    painter.save();
    paintMonthFrame(painter, layout.area, layout.current);

    const QRectF area = layout.area.adjusted(10, 8, -10, -9);
    const qreal titleHeight = area.height() * 0.16;
    const qreal weekHeight = area.height() * 0.13;
    const QColor text = textColor();
    const QColor muted = mutedTextColor();

    QFont titleFont("Segoe UI",
                    scaledPointSize(titleHeight, layout.current ? 0.70 : 0.58,
                                    layout.current ? 13 : 9, layout.current ? 34 : 22),
                    QFont::DemiBold);
    painter.setFont(titleFont);
    painter.setPen(layout.current ? accentColor() : text);
    painter.drawText(QRectF(area.left(), area.top(), area.width(), titleHeight),
                     Qt::AlignLeft | Qt::AlignVCenter, monthTitle(layout.month));

    const QRectF weekRow(area.left(), area.top() + titleHeight, area.width(), weekHeight);
    QFont weekFont("Meiryo UI",
                   scaledPointSize(weekHeight, layout.current ? 0.58 : 0.50,
                                   layout.current ? 8 : 7, layout.current ? 18 : 13),
                   QFont::DemiBold);
    painter.setFont(weekFont);
    for (int i = 0; i < 7; ++i) {
        const QRectF cell(weekRow.left() + weekRow.width() / 7.0 * i, weekRow.top(),
                          weekRow.width() / 7.0, weekRow.height());
        painter.setPen(i == 0 ? sundayColor() : muted);
        painter.drawText(cell, Qt::AlignCenter, weekLabel(i));
    }

    const QRectF grid(area.left(), weekRow.bottom() + 3, area.width(), area.bottom() - weekRow.bottom() - 3);
    const qreal cellW = grid.width() / 7.0;
    const qreal cellH = grid.height() / 6.0;
    QFont dayFont("Segoe UI",
                  scaledPointSize(cellH, layout.current ? 0.42 : 0.36,
                                  layout.current ? 11 : 7, layout.current ? 34 : 20),
                  QFont::DemiBold);
    painter.setFont(dayFont);

    const int firstColumn = layout.month.dayOfWeek() % 7;
    const int days = layout.month.daysInMonth();
    for (int day = 1; day <= days; ++day) {
        const int index = firstColumn + day - 1;
        const int row = index / 7;
        const int column = index % 7;
        QRectF cell(grid.left() + cellW * column, grid.top() + cellH * row, cellW, cellH);
        const QDate date(layout.month.year(), layout.month.month(), day);
        const bool isToday = date == m_currentDate;

        if (isToday) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(accentColor());
            const qreal size = qMin(cell.width(), cell.height()) * 0.82;
            const QRectF marker(cell.center().x() - size / 2.0, cell.center().y() - size / 2.0, size, size);
            painter.drawEllipse(marker);
        }

        if (isToday) {
            painter.setPen(Qt::white);
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

QDate CalendarWidget::firstDayOfVisibleMonth(int offset) const
{
    const QDate date = m_baseMonth.addMonths(offset);
    return QDate(date.year(), date.month(), 1);
}

bool CalendarWidget::isDarkMode() const
{
    return palette().color(QPalette::Window).lightness() < 128;
}

QColor CalendarWidget::backgroundColor() const
{
    return isDarkMode() ? QColor(19, 22, 23) : QColor(246, 247, 244);
}

QColor CalendarWidget::surfaceTopColor() const
{
    return isDarkMode() ? QColor(42, 48, 50) : QColor(255, 255, 252);
}

QColor CalendarWidget::surfaceBottomColor() const
{
    return isDarkMode() ? QColor(27, 32, 34) : QColor(236, 241, 239);
}

QColor CalendarWidget::surfaceBorderColor() const
{
    return isDarkMode() ? QColor(72, 83, 84) : QColor(204, 213, 210);
}

QColor CalendarWidget::monthFillColor(bool current) const
{
    if (isDarkMode()) {
        return current ? QColor(31, 48, 49) : QColor(33, 38, 39);
    }
    return current ? QColor(234, 244, 242) : QColor(249, 250, 248);
}

QColor CalendarWidget::monthBorderColor(bool current) const
{
    if (isDarkMode()) {
        return current ? QColor(92, 171, 170) : QColor(68, 78, 79);
    }
    return current ? QColor(72, 119, 121) : QColor(212, 220, 217);
}

QColor CalendarWidget::accentColor() const
{
    return isDarkMode() ? QColor(118, 214, 211) : QColor(35, 79, 82);
}

QColor CalendarWidget::sundayColor() const
{
    return isDarkMode() ? QColor(238, 137, 128) : QColor(173, 73, 66);
}

QColor CalendarWidget::inactiveDayColor() const
{
    return isDarkMode() ? QColor(152, 167, 167) : QColor(86, 99, 99);
}

QColor CalendarWidget::textColor() const
{
    return isDarkMode() ? QColor(229, 236, 235) : QColor(34, 43, 45);
}

QColor CalendarWidget::mutedTextColor() const
{
    return isDarkMode() ? QColor(170, 184, 183) : QColor(94, 108, 108);
}
