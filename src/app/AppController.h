#pragma once

#include <QObject>
#include <QUrl>
#include <QHash>
#include <QVariant>

#include "BomTableModel.h"
#include "CategoryController.h"
#include "AppLogger.h"
#include "DataIoController.h"
#include "ArchiveController.h"
#include "ProjectController.h"
#include "ThemeController.h"
#include "UiSettingsStore.h"
#include "DefaultDataSeeder.h"
#include "StatusHub.h"

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ThemeController *theme READ theme CONSTANT)
    Q_PROPERTY(ProjectController *projects READ projects CONSTANT)
    Q_PROPERTY(CategoryController *categories READ categories CONSTANT)
    Q_PROPERTY(BomTableModel *bomModel READ bomModel CONSTANT)
    Q_PROPERTY(DataIoController *io READ io CONSTANT)
    Q_PROPERTY(ArchiveController *archive READ archive CONSTANT)
    Q_PROPERTY(LogRelay *logRelay READ logRelay CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    ThemeController *theme();
    ProjectController *projects();
    CategoryController *categories();
    BomTableModel *bomModel();
    DataIoController *io();
    ArchiveController *archive();
    LogRelay *logRelay();
    QString status() const;

    Q_INVOKABLE void cycleTheme();
    Q_INVOKABLE bool deleteProject(int index);
    Q_INVOKABLE void notify(const QString &message);
    Q_INVOKABLE void logInfo(const QString &message);
    Q_INVOKABLE void logWarning(const QString &message);
    Q_INVOKABLE void logError(const QString &message);
    Q_INVOKABLE QVariantList loadBomWidthRatios(const QString &layoutHash) const;
    Q_INVOKABLE void saveBomWidthRatios(const QString &layoutHash, const QVariantList &ratios);

signals:
    void statusChanged();

private:
    void setStatus(const QString &status);

    ThemeController m_theme;
    ProjectController m_projects;
    CategoryController m_categories;
    BomTableModel m_bomModel;
    DataIoController m_io;
    ArchiveController m_archive;
    LogRelay m_logRelay;
    UiSettingsStore m_uiSettings;
    DefaultDataSeeder m_defaultSeeder;
    StatusHub m_statusHub;
    QHash<QString, QVariantList> m_bomWidthRatiosByLayout;
};


