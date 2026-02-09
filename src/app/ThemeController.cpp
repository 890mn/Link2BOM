#include "ThemeController.h"

#include <QSettings>

ThemeController::ThemeController(QObject *parent)
    : QObject(parent)
    , m_themes({QStringLiteral("Light"), QStringLiteral("Dark")})
{
    loadSettings();
}

QStringList ThemeController::themes() const
{
    return m_themes;
}

int ThemeController::currentIndex() const
{
    return m_currentIndex;
}

void ThemeController::setCurrentIndex(int index)
{
    if (m_themes.isEmpty()) {
        return;
    }
    const int normalized = (index % m_themes.size() + m_themes.size()) % m_themes.size();
    if (normalized == m_currentIndex) {
        return;
    }
    m_currentIndex = normalized;
    emit currentIndexChanged();
    saveSettings();
}

QString ThemeController::currentThemeName() const
{
    if (m_themes.isEmpty()) {
        return QString();
    }
    return m_themes.value(m_currentIndex);
}

void ThemeController::cycleTheme()
{
    setCurrentIndex(m_currentIndex + 1);
}

void ThemeController::loadSettings()
{
    QSettings settings;
    const QString value = settings.value(QStringLiteral("theme/name"), QStringLiteral("Light")).toString();
    m_currentIndex = (value.compare(QStringLiteral("Dark"), Qt::CaseInsensitive) == 0) ? 1 : 0;
}

void ThemeController::saveSettings() const
{
    QSettings settings;
    settings.setValue(QStringLiteral("theme/name"), currentThemeName());
}
