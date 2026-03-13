#include "ArchiveController.h"

#include "ProjectController.h"
#include "CategoryController.h"
#include "BomTableModel.h"

#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QRegularExpression>

namespace {
QString normalizeLabel(const QString &label)
{
    const QString trimmed = label.trimmed();
    return trimmed.isEmpty() ? QStringLiteral("Archive") : trimmed;
}

QString formatTime(const QString &isoTime)
{
    const QDateTime dt = QDateTime::fromString(isoTime, Qt::ISODate);
    if (!dt.isValid()) {
        return QString();
    }
    return dt.toLocalTime().toString(QStringLiteral("yyyy-MM-dd HH:mm"));
}

bool writeArchiveFile(const QString &path,
                      const QString &label,
                      const QStringList &headers,
                      const QList<QStringList> &rows,
                      const QStringList &projects,
                      const QStringList &categories,
                      const QString &selectedProject)
{
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("label"), label);
    root.insert(QStringLiteral("savedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    root.insert(QStringLiteral("selectedProject"), selectedProject);

    QJsonArray projectArray;
    for (const QString &name : projects) {
        projectArray.append(name);
    }
    root.insert(QStringLiteral("projects"), projectArray);

    QJsonArray categoryArray;
    for (const QString &name : categories) {
        categoryArray.append(name);
    }
    root.insert(QStringLiteral("categories"), categoryArray);

    QJsonArray headerArray;
    for (const QString &header : headers) {
        headerArray.append(header);
    }
    root.insert(QStringLiteral("headers"), headerArray);

    QJsonArray rowArray;
    for (const QStringList &row : rows) {
        QJsonArray rowCells;
        for (const QString &cell : row) {
            rowCells.append(cell);
        }
        rowArray.append(rowCells);
    }
    root.insert(QStringLiteral("rows"), rowArray);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}
}

ArchiveController::ArchiveController(ProjectController *projects,
                                     CategoryController *categories,
                                     BomTableModel *bomModel,
                                     QObject *parent)
    : QObject(parent)
    , m_projects(projects)
    , m_categories(categories)
    , m_bomModel(bomModel)
{
}

QString ArchiveController::baseDir() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (dir.isEmpty()) {
        dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    const QString appName = QCoreApplication::applicationName().trimmed();
    if (!appName.isEmpty()) {
        QString clean = QDir::cleanPath(dir);
        const QString doubleSuffix = QLatin1Char('/') + appName + QLatin1Char('/') + appName;
        if (clean.endsWith(doubleSuffix)) {
            clean = clean.left(clean.size() - appName.size() - 1);
        }
        dir = clean;
    }
    return QDir(dir).filePath(QStringLiteral("saves"));
}

QString ArchiveController::registryPath() const
{
    return QDir(baseDir()).filePath(QStringLiteral("slot_registry.json"));
}

QString ArchiveController::defaultSlotPath(int index) const
{
    const QString base = baseDir();
    const QString name = (index <= 0)
        ? QStringLiteral("save_default.json")
        : QStringLiteral("save_slot_%1.json").arg(index);
    return QDir(base).filePath(name);
}

QString ArchiveController::resolveSlotPath(int index) const
{
    if (index <= 0) {
        return defaultSlotPath(index);
    }
    const QVariantMap registry = loadRegistry();
    const QString key = QString::number(index);
    const QString mapped = registry.value(key).toString().trimmed();
    if (!mapped.isEmpty()) {
        return mapped;
    }
    return defaultSlotPath(index);
}

QVariantMap ArchiveController::loadRegistry() const
{
    QVariantMap registry;
    QFile file(registryPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return registry;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return registry;
    }
    const QJsonObject obj = doc.object();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        registry.insert(it.key(), it.value().toString());
    }
    return registry;
}

bool ArchiveController::saveRegistry(const QVariantMap &registry) const
{
    QDir().mkpath(baseDir());
    QJsonObject obj;
    for (auto it = registry.begin(); it != registry.end(); ++it) {
        obj.insert(it.key(), it.value().toString());
    }
    QFile file(registryPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    return true;
}

QJsonArray ArchiveController::writeRows(const QVariantList &rows) const
{
    QJsonArray array;
    for (const QVariant &rowVar : rows) {
        const QStringList row = rowVar.toStringList();
        QJsonArray rowArray;
        for (const QString &cell : row) {
            rowArray.append(cell);
        }
        array.append(rowArray);
    }
    return array;
}

QVariantList ArchiveController::readRows(const QJsonArray &rows) const
{
    QVariantList list;
    list.reserve(rows.size());
    for (const QJsonValue &rowValue : rows) {
        const QJsonArray rowArray = rowValue.toArray();
        QStringList row;
        row.reserve(rowArray.size());
        for (const QJsonValue &cell : rowArray) {
            row.append(cell.toString());
        }
        list.append(row);
    }
    return list;
}

QVariantList ArchiveController::listSlots() const
{
    QVariantList slotList;
    const QString base = baseDir();
    for (int i = 0; i < 5; ++i) {
        const QString path = resolveSlotPath(i);
        const QFileInfo info(path);
        QVariantMap entry;
        entry.insert(QStringLiteral("index"), i);
        entry.insert(QStringLiteral("path"), path);
        entry.insert(QStringLiteral("canDelete"), i > 0);

        QString title;
        QString subtitle;
        bool hasData = false;

        if (info.exists()) {
            QFile file(path);
            if (file.open(QIODevice::ReadOnly)) {
                const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                if (doc.isObject()) {
                    const QJsonObject obj = doc.object();
                    const QString label = obj.value(QStringLiteral("label")).toString();
                    const QString savedAt = obj.value(QStringLiteral("savedAt")).toString();
                    const QJsonArray savedRows = obj.value(QStringLiteral("rows")).toArray();
                    const QString timeText = formatTime(savedAt);
                    hasData = !savedRows.isEmpty();

                    if (i == 0) {
                        title = QStringLiteral("Local Archive (AppData)");
                        subtitle = QStringLiteral("%1 | %2").arg(base, timeText.isEmpty() ? QStringLiteral("-") : timeText);
                    } else {
                        title = label.isEmpty() ? QStringLiteral("Slot %1").arg(i + 1) : label;
                        const QString timePart = timeText.isEmpty() ? QStringLiteral("-") : timeText;
                        subtitle = QStringLiteral("%1 | %2").arg(info.absoluteFilePath(), timePart);
                    }
                }
            }
        }

        if (!hasData) {
            if (i == 0) {
                title = QStringLiteral("Local Archive (AppData)");
                subtitle = QStringLiteral("%1 | Not saved yet").arg(base);
            } else {
                title = QStringLiteral("Slot %1").arg(i + 1);
                subtitle = QStringLiteral("%1 | Not saved yet").arg(info.absoluteFilePath());
            }
        }

        entry.insert(QStringLiteral("title"), title);
        entry.insert(QStringLiteral("subtitle"), subtitle);
        entry.insert(QStringLiteral("hasData"), hasData);
        slotList.append(entry);
    }
    return slotList;
}

bool ArchiveController::saveSlot(int index, const QString &label, const QString &customPath)
{
    if (!m_projects || !m_categories || !m_bomModel) {
        return false;
    }

    const QString dirPath = baseDir();
    QDir().mkpath(dirPath);

    const QVariantMap snapshot = m_bomModel->exportSnapshot();
    const QStringList headers = snapshot.value(QStringLiteral("headers")).toStringList();
    const QVariantList rows = snapshot.value(QStringLiteral("rows")).toList();

    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    const QString normalizedLabel = normalizeLabel(label);
    root.insert(QStringLiteral("label"), normalizedLabel);
    root.insert(QStringLiteral("savedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    root.insert(QStringLiteral("selectedProject"), m_projects->selectedProject());

    QJsonArray projectArray;
    const QStringList projects = m_projects->projectNames(true);
    for (const QString &name : projects) {
        projectArray.append(name);
    }
    root.insert(QStringLiteral("projects"), projectArray);

    QJsonArray categoryArray;
    for (const QString &name : m_categories->categoryNames()) {
        categoryArray.append(name);
    }
    root.insert(QStringLiteral("categories"), categoryArray);

    QJsonArray headerArray;
    for (const QString &header : headers) {
        headerArray.append(header);
    }
    root.insert(QStringLiteral("headers"), headerArray);
    root.insert(QStringLiteral("rows"), writeRows(rows));

    const QString trimmedPath = customPath.trimmed();
    bool usedCustomPath = false;

    auto resolveCustomPath = [&](const QString &inputPath) {
        QFileInfo info(inputPath);
        if (info.suffix().isEmpty()) {
            QDir().mkpath(inputPath);
            QString fileName = normalizedLabel;
            const QRegularExpression invalidPattern(QStringLiteral(R"([\\/:*?"<>|])"));
            fileName.replace(invalidPattern, QStringLiteral("_"));
            if (!fileName.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
                fileName.append(QStringLiteral(".json"));
            }
            return QDir(inputPath).filePath(fileName);
        }

        const QString parentDir = info.absolutePath();
        QDir().mkpath(parentDir);
        QString fullPath = info.absoluteFilePath();
        if (!fullPath.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
            fullPath.append(QStringLiteral(".json"));
        }
        return fullPath;
    };

    QString path = defaultSlotPath(index);
    if (index > 0 && !trimmedPath.isEmpty()) {
        path = resolveCustomPath(trimmedPath);
        usedCustomPath = true;
    }

    auto writeArchive = [&](const QString &targetPath) {
        QFile file(targetPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return false;
        }
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        return true;
    };

    if (!writeArchive(path)) {
        if (usedCustomPath && index > 0) {
            path = defaultSlotPath(index);
            usedCustomPath = false;
            if (!writeArchive(path)) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (index > 0) {
        QVariantMap registry = loadRegistry();
        const QString key = QString::number(index);
        if (usedCustomPath) {
            registry.insert(key, path);
        } else if (registry.contains(key)) {
            registry.remove(key);
        }
        saveRegistry(registry);
    }
    return true;
}

void ArchiveController::ensureDefaultSlots(const QStringList &headers,
                                           const QList<QStringList> &rows,
                                           const QStringList &projects,
                                           const QStringList &categories,
                                           const QString &selectedProject)
{
    if (headers.isEmpty()) {
        return;
    }
    QDir().mkpath(baseDir());

    const QString defaultPath = defaultSlotPath(0);
    if (!QFileInfo::exists(defaultPath)) {
        writeArchiveFile(defaultPath,
                         QStringLiteral("Default Archive"),
                         headers,
                         rows,
                         projects,
                         categories,
                         selectedProject);
    }

    QList<QStringList> emptyRows;
    for (int i = 1; i < 5; ++i) {
        const QString path = defaultSlotPath(i);
        if (!QFileInfo::exists(path)) {
            writeArchiveFile(path,
                             QStringLiteral("Save%1").arg(i),
                             headers,
                             emptyRows,
                             projects,
                             categories,
                             selectedProject);
        }
    }
}

bool ArchiveController::loadSlot(int index)
{
    if (!m_projects || !m_categories || !m_bomModel) {
        return false;
    }

    QString path = resolveSlotPath(index);
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (index > 0) {
            QVariantMap registry = loadRegistry();
            const QString key = QString::number(index);
            if (registry.contains(key)) {
                registry.remove(key);
                saveRegistry(registry);
            }
            path = defaultSlotPath(index);
            file.setFileName(path);
            if (!file.open(QIODevice::ReadOnly)) {
                return false;
            }
        } else {
            return false;
        }
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return false;
    }

    const QJsonObject obj = doc.object();
    const QJsonArray projectArray = obj.value(QStringLiteral("projects")).toArray();
    QStringList projects;
    projects.reserve(projectArray.size());
    for (const QJsonValue &value : projectArray) {
        projects.append(value.toString());
    }

    const QJsonArray categoryArray = obj.value(QStringLiteral("categories")).toArray();
    QStringList categories;
    categories.reserve(categoryArray.size());
    for (const QJsonValue &value : categoryArray) {
        categories.append(value.toString());
    }

    const QJsonArray headerArray = obj.value(QStringLiteral("headers")).toArray();
    QStringList headers;
    headers.reserve(headerArray.size());
    for (const QJsonValue &value : headerArray) {
        headers.append(value.toString());
    }

    const QVariantList rows = readRows(obj.value(QStringLiteral("rows")).toArray());
    const QVariantMap snapshot {
        {QStringLiteral("headers"), headers},
        {QStringLiteral("rows"), rows}
    };

    if (!m_bomModel->importSnapshot(snapshot)) {
        return false;
    }

    m_projects->setProjectNames(projects, obj.value(QStringLiteral("selectedProject")).toString());
    m_categories->setCategoryNames(categories);
    return true;
}

bool ArchiveController::deleteSlot(int index)
{
    if (index <= 0) {
        return false;
    }
    const QString path = resolveSlotPath(index);
    const bool removed = !QFile::exists(path) || QFile::remove(path);
    QVariantMap registry = loadRegistry();
    const QString key = QString::number(index);
    if (registry.contains(key)) {
        registry.remove(key);
        saveRegistry(registry);
    }
    return removed;
}

