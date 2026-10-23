#include "ThemeManager.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
#include <QTextStream>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager s_instance;
    return s_instance;
}

void ThemeManager::setTheme(ThemeType theme)
{
    if (m_currentTheme == theme)
        return;

    m_currentTheme = theme;
    applyStyleSheet();
    emit themeChanged();
}

QColor ThemeManager::getDefaultIconColor() const
{
    return (m_currentTheme == ThemeType::Dark) ? QColor(QStringLiteral("#A3E635"))
        : QColor(QStringLiteral("#1E1E1E"));
}

QIcon ThemeManager::getIcon(const QString& iconName, const QColor& color, const QString& folder) const
{
    const QString path = QStringLiteral(":/Icons/%1/%2.svg").arg(folder, iconName);

    QSvgRenderer renderer(path);
    if (!renderer.isValid()) {
        qWarning() << "SVG not found or invalid:" << path;
        return QIcon();
    }

    QSize size = renderer.defaultSize();
    if (size.isEmpty())
        size = QSize(32, 32);

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();

    return QIcon(pixmap);
}

void ThemeManager::applyStyleSheet()
{
    if (m_currentTheme == ThemeType::None)
        return;

    QString styleFile = QStringLiteral(":/theme.qss");

    QFile file(styleFile);
    if (!file.exists()) {
        styleFile = QStringLiteral("theme.qss");
        file.setFileName(styleFile);
    }

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Failed to load theme:" << styleFile;
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    QString style = stream.readAll();
    file.close();

    if (style.startsWith(QChar(0xFEFF)))
        style.remove(0, 1);

    qApp->setStyleSheet(style);
}