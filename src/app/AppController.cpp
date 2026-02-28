#include "AppController.h"

#include <QSettings>
#include <QUrl>
#include <cmath>

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    loadUiSettings();

    connect(&m_theme, &ThemeController::currentIndexChanged, this, [this] {
        setStatus(QStringLiteral("Theme changed: %1").arg(m_theme.currentThemeName()));
    });
    connect(&m_projects, &ProjectController::selectedProjectChanged, this, [this] {
        m_bomModel.setProjectFilter(m_projects.selectedProject());
    });

    m_bomModel.setSourceData({QStringLiteral("\u9879\u76ee"),
                              QStringLiteral("\u5546\u54c1\u7f16\u53f7"),
                              QStringLiteral("\u54c1\u724c"),
                              QStringLiteral("\u5382\u5bb6\u578b\u53f7"),
                              QStringLiteral("\u5c01\u88c5"),
                              QStringLiteral("\u5546\u54c1\u540d\u79f0"),
                              QStringLiteral("\u8ba2\u8d2d\u6570\u91cf\uff08\u4fee\u6539\u540e\uff09"),
                              QStringLiteral("\u5546\u54c1\u5355\u4ef7"),
                              QStringLiteral("\u5546\u54c1\u91d1\u989d")},
                             {{QStringLiteral("Default Project"), QStringLiteral("C25804"), QStringLiteral("Yageo"), QStringLiteral("RC0603FR-0710KL"), QStringLiteral("0603"), QStringLiteral("Resistor 10K"), QStringLiteral("8"), QStringLiteral("0.0015"), QStringLiteral("0.0120")},
                              {QStringLiteral("Default Project"), QStringLiteral("C14663"), QStringLiteral("Samsung"), QStringLiteral("CL10B104KB8NNNC"), QStringLiteral("0603"), QStringLiteral("Cap 100nF"), QStringLiteral("4"), QStringLiteral("0.0020"), QStringLiteral("0.0080")},
                              {QStringLiteral("Default Project"), QStringLiteral("C21120"), QStringLiteral("Murata"), QStringLiteral("GRM188R71C105KA12D"), QStringLiteral("0603"), QStringLiteral("Cap 1uF"), QStringLiteral("3"), QStringLiteral("0.0061"), QStringLiteral("0.0183")},
                              {QStringLiteral("Default Project"), QStringLiteral("C529431"), QStringLiteral("ST"), QStringLiteral("STM32G071KBT6"), QStringLiteral("LQFP32"), QStringLiteral("MCU"), QStringLiteral("1"), QStringLiteral("1.8200"), QStringLiteral("1.8200")},
                              {QStringLiteral("Default Project"), QStringLiteral("C16581"), QStringLiteral("WCH"), QStringLiteral("CH340C"), QStringLiteral("SOP16"), QStringLiteral("USB-UART"), QStringLiteral("1"), QStringLiteral("0.3100"), QStringLiteral("0.3100")},
                              {QStringLiteral("Default Project"), QStringLiteral("C29294"), QStringLiteral("TI"), QStringLiteral("TPS54331DR"), QStringLiteral("SOIC8"), QStringLiteral("DC-DC"), QStringLiteral("1"), QStringLiteral("0.7800"), QStringLiteral("0.7800")},
                              {QStringLiteral("Default Project"), QStringLiteral("C5446"), QStringLiteral("Omron"), QStringLiteral("B3F-1000"), QStringLiteral("THT"), QStringLiteral("Tact Switch"), QStringLiteral("2"), QStringLiteral("0.0900"), QStringLiteral("0.1800")},
                              {QStringLiteral("Default Project"), QStringLiteral("C7213"), QStringLiteral("Littelfuse"), QStringLiteral("1206L050"), QStringLiteral("1206"), QStringLiteral("PTC Fuse"), QStringLiteral("1"), QStringLiteral("0.1500"), QStringLiteral("0.1500")}});

    m_bomModel.setProjectFilter(m_projects.selectedProject());
    setStatus(QStringLiteral("Ready"));
}

ThemeController *AppController::theme() { return &m_theme; }
ProjectController *AppController::projects() { return &m_projects; }
CategoryController *AppController::categories() { return &m_categories; }
BomTableModel *AppController::bomModel() { return &m_bomModel; }
QString AppController::status() const { return m_status; }

void AppController::cycleTheme()
{
    m_theme.cycleTheme();
}

void AppController::importLichuang(const QUrl &fileUrl, const QString &projectName)
{
    const QString localFile = fileUrl.toLocalFile();
    if (localFile.isEmpty()) {
        setStatus(QStringLiteral("Import failed: no file selected"));
        return;
    }

    const QString targetProject = projectName.trimmed();
    if (targetProject.isEmpty() || targetProject == QStringLiteral("All Projects") || targetProject == QStringLiteral("全部项目")) {
        setStatus(QStringLiteral("Import failed: select a specific project"));
        return;
    }

    m_projects.addProject(targetProject);

    const ImportResult result = m_importService.importLichuangSpreadsheet(localFile, targetProject);
    if (!result.ok) {
        setStatus(QStringLiteral("Import failed: %1").arg(result.error));
        return;
    }

    if (!m_bomModel.appendRows(result.headers, result.rows)) {
        setStatus(QStringLiteral("Import failed: header mismatch with current BOM view. Please clear current data or import same template format."));
        return;
    }
    m_projects.setSelectedProject(targetProject);
    setStatus(QStringLiteral("Imported %1 -> %2").arg(fileUrl.fileName(), targetProject));
}

bool AppController::deleteProject(int index)
{
    const QStringList names = m_projects.projectNames(true);
    if (index < 0 || index >= names.size()) {
        setStatus(QStringLiteral("Delete project failed: invalid index."));
        return false;
    }

    const QString target = names[index];
    if (target == QStringLiteral("All Projects") || target == QStringLiteral("全部项目")) {
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

    saveUiSettings();
}

void AppController::setStatus(const QString &status)
{
    if (status == m_status) {
        return;
    }
    m_status = status;
    emit statusChanged();
}

void AppController::loadUiSettings()
{
    m_bomWidthRatiosByLayout.clear();

    QSettings settings;
    const QVariantMap savedMap = settings.value(QStringLiteral("bom/customWidthRatios")).toMap();
    for (auto it = savedMap.constBegin(); it != savedMap.constEnd(); ++it) {
        const QVariantList list = it.value().toList();
        if (!list.isEmpty()) {
            m_bomWidthRatiosByLayout.insert(it.key(), list);
        }
    }
}

void AppController::saveUiSettings() const
{
    QVariantMap toSave;
    for (auto it = m_bomWidthRatiosByLayout.constBegin(); it != m_bomWidthRatiosByLayout.constEnd(); ++it) {
        toSave.insert(it.key(), it.value());
    }

    QSettings settings;
    settings.setValue(QStringLiteral("bom/customWidthRatios"), toSave);
}
