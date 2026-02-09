#pragma once

#include <QObject>
#include <QStringList>

class ThemeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList themes READ themes CONSTANT)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentThemeName READ currentThemeName NOTIFY currentIndexChanged)

public:
    explicit ThemeController(QObject *parent = nullptr);

    QStringList themes() const;
    int currentIndex() const;
    void setCurrentIndex(int index);
    QString currentThemeName() const;

    Q_INVOKABLE void cycleTheme();

signals:
    void currentIndexChanged();

private:
    void loadSettings();
    void saveSettings() const;

    QStringList m_themes;
    int m_currentIndex = 0;
};
