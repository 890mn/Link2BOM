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

    m_bomModel.setSourceData({QStringLiteral("项目"),
                              QStringLiteral("位号"),
                              QStringLiteral("分类"),
                              QStringLiteral("料号"),
                              QStringLiteral("规格"),
                              QStringLiteral("数量"),
                              QStringLiteral("供应商"),
                              QStringLiteral("备注")},
                             {{QStringLiteral("电源管理板 RevA"), QStringLiteral("R1-R8"), QStringLiteral("电阻电容"), QStringLiteral("RC0603-10K"), QStringLiteral("10K 1% 0603"), QStringLiteral("8"), QStringLiteral("LCSC"), QStringLiteral("常规库存")},
                              {QStringLiteral("电源管理板 RevA"), QStringLiteral("C1-C4"), QStringLiteral("电阻电容"), QStringLiteral("CC0603-100N"), QStringLiteral("100nF 16V X7R"), QStringLiteral("4"), QStringLiteral("LCSC"), QStringLiteral("去耦")},
                              {QStringLiteral("传感器节点 V2"), QStringLiteral("U1"), QStringLiteral("芯片 IC"), QStringLiteral("STM32G0"), QStringLiteral("QFN32"), QStringLiteral("1"), QStringLiteral("LCSC"), QStringLiteral("主控")}});

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
    if (targetProject.isEmpty() || targetProject == QStringLiteral("全部项目")) {
        setStatus(QStringLiteral("Import failed: select a specific project"));
        return;
    }

    m_projects.addProject(targetProject);

    const ImportResult result = m_importService.importLichuangSpreadsheet(localFile, targetProject);
    if (!result.ok) {
        setStatus(QStringLiteral("Import failed: %1").arg(result.error));
        return;
    }

    m_bomModel.setSourceData(result.headers, result.rows);
    setStatus(QStringLiteral("Imported %1 -> %2").arg(fileUrl.fileName(), targetProject));
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
