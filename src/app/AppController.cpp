#include "AppController.h"
#include "AppLogger.h"

#include <QCoreApplication>
#include <cmath>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_io(&m_projects, &m_bomModel, this)
    , m_archive(&m_projects, &m_categories, &m_bomModel, this)
    , m_statusHub(this)
{
    AppLogger::attachRelay(&m_logRelay);
    m_bomWidthRatiosByLayout = m_uiSettings.loadBomWidthRatios();
    connect(&m_statusHub, &StatusHub::statusChanged, this, &AppController::statusChanged);

    connect(&m_theme, &ThemeController::currentIndexChanged, this, [this] {
        setStatus(QStringLiteral("Theme changed: %1").arg(m_theme.currentThemeName()));
    });
    connect(&m_projects, &ProjectController::selectedProjectChanged, this, [this] {
        m_bomModel.setProjectFilter(m_projects.selectedProject());
    });
    connect(&m_io, &DataIoController::statusMessage, this, &AppController::setStatus);
    m_defaultSeeder.initialize(m_archive, m_projects, m_categories, m_bomModel);

    m_bomModel.setProjectFilter(m_projects.selectedProject());
    setStatus(QStringLiteral("Ready"));
    QObject::connect(qApp, &QCoreApplication::aboutToQuit, this, [this] { m_archive.saveSlot(0); });
}

ThemeController *AppController::theme() { return &m_theme; }
ProjectController *AppController::projects() { return &m_projects; }
CategoryController *AppController::categories() { return &m_categories; }
BomTableModel *AppController::bomModel() { return &m_bomModel; }
DataIoController *AppController::io() { return &m_io; }
ArchiveController *AppController::archive() { return &m_archive; }
LogRelay *AppController::logRelay() { return &m_logRelay; }
QString AppController::status() const { return m_statusHub.status(); }

void AppController::cycleTheme()
{
    m_theme.cycleTheme();
}

bool AppController::deleteProject(int index)
{
    const QStringList names = m_projects.projectNames(true);
    if (index < 0 || index >= names.size()) {
        setStatus(QStringLiteral("Delete project failed: invalid index."));
        return false;
    }

    const QString target = names[index];
    if (target == QStringLiteral("All Projects")) {
        setStatus(QStringLiteral("Delete project failed: cannot delete 'All Projects'."));
        return false;
    }

    if (!m_projects.removeProject(index)) {
        setStatus(QStringLiteral("Delete project failed."));
        return false;
    }

    m_bomModel.removeRowsByProject(target);
    setStatus(QStringLiteral("Deleted project: %1").arg(target));
    return true;
}

void AppController::notify(const QString &message)
{
    if (!message.trimmed().isEmpty()) {
        setStatus(message.trimmed());
    }
}

void AppController::logInfo(const QString &message)
{
    if (!message.trimmed().isEmpty()) {
        AppLogger::info(message.trimmed());
    }
}

void AppController::logWarning(const QString &message)
{
    if (!message.trimmed().isEmpty()) {
        AppLogger::warn(message.trimmed());
    }
}

void AppController::logError(const QString &message)
{
    if (!message.trimmed().isEmpty()) {
        AppLogger::error(message.trimmed());
    }
}

QVariantList AppController::loadBomWidthRatios(const QString &layoutHash) const
{
    return m_bomWidthRatiosByLayout.value(layoutHash.trimmed());
}

void AppController::saveBomWidthRatios(const QString &layoutHash, const QVariantList &ratios)
{
    const QString key = layoutHash.trimmed();
    if (key.isEmpty()) {
        return;
    }

    QVariantList normalized;
    normalized.reserve(ratios.size());
    for (const QVariant &value : ratios) {
        const double ratio = value.toDouble();
        if (std::isfinite(ratio) && ratio > 0.01) {
            normalized.append(ratio);
        }
    }

    if (normalized.isEmpty()) {
        m_bomWidthRatiosByLayout.remove(key);
    } else {
        m_bomWidthRatiosByLayout.insert(key, normalized);
    }

    m_uiSettings.saveBomWidthRatios(m_bomWidthRatiosByLayout);
}

void AppController::setStatus(const QString &status)
{
    m_statusHub.setStatus(status);
}





