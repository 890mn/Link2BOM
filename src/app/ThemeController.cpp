#include "ThemeController.h"

#include <QGuiApplication>
#include <QSettings>
#include <QStyleHints>

ThemeController::ThemeController(QObject *parent)
    : QObject(parent)
    , m_themes({QStringLiteral("System"), QStringLiteral("Light"), QStringLiteral("Dark")})
{
    loadSettings();

    if (QGuiApplication::styleHints()) {
        connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
                this, [this]() {
                    if (m_currentIndex == 0) {
                        emit currentIndexChanged();
                    }
                });
    }
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
        return QStringLiteral("Light");
    }

    if (m_currentIndex == 0) {
        const auto scheme = QGuiApplication::styleHints()
            ? QGuiApplication::styleHints()->colorScheme()
            : Qt::ColorScheme::Unknown;
        return scheme == Qt::ColorScheme::Dark
            ? QStringLiteral("Dark")
            : QStringLiteral("Light");
    }

    return m_themes.value(m_currentIndex, QStringLiteral("Light"));
}

void ThemeController::cycleTheme()
{
    setCurrentIndex(m_currentIndex + 1);
}

void ThemeController::loadSettings()
{
    QSettings settings;
    const QString value = settings.value(QStringLiteral("theme/name"), QStringLiteral("System")).toString();

    if (value.compare(QStringLiteral("Dark"), Qt::CaseInsensitive) == 0) {
        m_currentIndex = 2;
    } else if (value.compare(QStringLiteral("Light"), Qt::CaseInsensitive) == 0) {
        m_currentIndex = 1;
    } else {
        m_currentIndex = 0;
    }
}

void ThemeController::saveSettings() const
{
    QSettings settings;
    settings.setValue(QStringLiteral("theme/name"), m_themes.value(m_currentIndex, QStringLiteral("System")));
}
