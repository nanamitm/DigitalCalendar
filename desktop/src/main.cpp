#include "MainWindow.h"

#include "AppIcon.h"

#include <QApplication>
#include <QFont>
#include <QIcon>
#include <QPalette>
#include <QPixmap>
#include <QStyle>
#include <QStyleHints>

namespace {
void applyColorSchemePalette(Qt::ColorScheme scheme)
{
    QPalette palette;
    if (scheme == Qt::ColorScheme::Dark) {
        palette.setColor(QPalette::Window, QColor(19, 22, 23));
        palette.setColor(QPalette::WindowText, QColor(229, 236, 235));
        palette.setColor(QPalette::Base, QColor(28, 32, 34));
        palette.setColor(QPalette::AlternateBase, QColor(35, 40, 42));
        palette.setColor(QPalette::Text, QColor(229, 236, 235));
        palette.setColor(QPalette::Button, QColor(42, 48, 50));
        palette.setColor(QPalette::ButtonText, QColor(229, 236, 235));
        palette.setColor(QPalette::Highlight, QColor(92, 171, 170));
        palette.setColor(QPalette::HighlightedText, QColor(12, 18, 19));
        palette.setColor(QPalette::ToolTipBase, QColor(42, 48, 50));
        palette.setColor(QPalette::ToolTipText, QColor(229, 236, 235));
    } else {
        palette = QApplication::style()->standardPalette();
        palette.setColor(QPalette::Window, QColor(246, 247, 244));
    }
    QApplication::setPalette(palette);
}
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("Digital Calendar");
    QApplication::setOrganizationName("Codex");

    QFont baseFont("Segoe UI", 10);
    QApplication::setFont(baseFont);
    QApplication::setWindowIcon(QIcon(QPixmap(AppIconXpm)));
    applyColorSchemePalette(app.styleHints()->colorScheme());
    QObject::connect(app.styleHints(), &QStyleHints::colorSchemeChanged, &app, applyColorSchemePalette);

    MainWindow window;
    window.show();

    return app.exec();
}
