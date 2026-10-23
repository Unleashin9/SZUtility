#pragma once

#include <QColor>
#include <QIcon>
#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    enum class ThemeType { None, Dark, Light };

    static ThemeManager& instance();

    void setTheme(ThemeType theme);
    ThemeType currentTheme() const { return m_currentTheme; }

    QIcon getIcon(const QString& iconName,
        const QColor& color,
        const QString& folder = QStringLiteral("MainWindow")) const;
    QColor getDefaultIconColor() const;

signals:
    void themeChanged();

private:
    ThemeManager() = default;
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void applyStyleSheet();

    ThemeType m_currentTheme = ThemeType::None;
};