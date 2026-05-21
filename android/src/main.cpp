#include "AndroidCalendarWidget.h"

#include <QApplication>
#include <QFont>
#include <QJniObject>
#include <QPalette>
#include <QStyle>
#include <QStyleHints>
#include <QtCore/qcoreapplication_platform.h>

namespace {
constexpr int AndroidFlagKeepScreenOn = 0x00000080;
constexpr int AndroidScreenOrientationLandscape = 0;

void applyColorSchemePalette(Qt::ColorScheme scheme)
{
    QPalette palette;
    if (scheme == Qt::ColorScheme::Dark) {
        palette.setColor(QPalette::Window, QColor(15, 18, 19));
        palette.setColor(QPalette::WindowText, QColor(232, 238, 237));
        palette.setColor(QPalette::Base, QColor(25, 30, 31));
        palette.setColor(QPalette::Text, QColor(232, 238, 237));
        palette.setColor(QPalette::Button, QColor(38, 45, 46));
        palette.setColor(QPalette::ButtonText, QColor(232, 238, 237));
        palette.setColor(QPalette::Highlight, QColor(92, 171, 170));
        palette.setColor(QPalette::HighlightedText, QColor(10, 16, 17));
    } else {
        palette = QApplication::style()->standardPalette();
        palette.setColor(QPalette::Window, QColor(246, 247, 244));
    }
    QApplication::setPalette(palette);
}

void configureAndroidWindow()
{
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([]() {
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        if (!activity.isValid()) {
            return;
        }

        activity.callMethod<void>("setRequestedOrientation", "(I)V", AndroidScreenOrientationLandscape);

        QJniObject window = activity.callObjectMethod("getWindow", "()Landroid/view/Window;");
        if (window.isValid()) {
            window.callMethod<void>("addFlags", "(I)V", AndroidFlagKeepScreenOn);
        }
    });
}
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Digital Calendar");
    QApplication::setOrganizationName("Codex");
    QApplication::setFont(QFont("Noto Sans", 10));

    applyColorSchemePalette(app.styleHints()->colorScheme());
    QObject::connect(app.styleHints(), &QStyleHints::colorSchemeChanged, &app, applyColorSchemePalette);

    AndroidCalendarWidget calendar;
    calendar.showFullScreen();
    configureAndroidWindow();

    return app.exec();
}
