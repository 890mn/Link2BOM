#include "DataIoController.h"

#include <QFile>
#include <QTextStream>
#include <QSet>

namespace {
int findProjectColumn(const QStringList &headers)
{
    auto normalize = [](const QString &text) {
        return QString(text).remove(' ').remove('\t').remove('\r').remove('\n').trimmed().toLower();
    };
    const QStringList keys = {QStringLiteral("项目"), QStringLiteral("project")};
    for (int i = 0; i < headers.size(); ++i) {
        const QString cell = normalize(headers[i]);
        for (const QString &key : keys) {
            if (cell.contains(key)) {
                return i;
            }
        }
    }
    return -1;
}

QStringList collectProjects(const QList<QStringList> &rows, int projectIndex)
{
    QSet<QString> unique;
    for (const QStringList &row : rows) {
        if (projectIndex >= 0 && projectIndex < row.size()) {
            const QString name = row[projectIndex].trimmed();
            if (!name.isEmpty()) {
                unique.insert(name);
            }
        }
    }
    QStringList list = unique.values();
    list.sort();
    return list;
}
}

DataIoController::DataIoController(ProjectController *projects, BomTableModel *bomModel, QObject *parent)
    : QObject(parent)
    , m_projects(projects)
    , m_bomModel(bomModel)
{
}

void DataIoController::importLichuang(const QUrl &fileUrl, const QString &projectName)
{
    if (!m_projects || !m_bomModel) {
        emit statusMessage(QStringLiteral("Import failed: data controller is not ready."));
        return;
    }

    const QString localFile = fileUrl.toLocalFile();
    if (localFile.isEmpty()) {
        emit statusMessage(QStringLiteral("Import failed: no file selected"));
        return;
    }

    const QString targetProject = projectName.trimmed();
    if (targetProject.isEmpty() || targetProject == QStringLiteral("All Projects")) {
        emit statusMessage(QStringLiteral("Import failed: select a specific project"));
        return;
    }

    m_projects->addProject(targetProject);

    const ImportResult result = m_importService.importLichuangSpreadsheet(localFile, targetProject);
    if (!result.ok) {
        emit statusMessage(QStringLiteral("Import failed: %1").arg(result.error));
        return;
    }

    const int projectColumn = findProjectColumn(result.headers);
    if (projectColumn >= 0) {
        const QStringList imported = collectProjects(result.rows, projectColumn);
        for (const QString &name : imported) {
            m_projects->addProject(name);
        }
    }

    if (!m_bomModel->appendRows(result.headers, result.rows)) {
        m_bomModel->setSourceData(result.headers, result.rows);
        emit statusMessage(QStringLiteral("Imported and replaced BOM headers due to new template."));
    } else {
        emit statusMessage(QStringLiteral("Imported %1 -> %2").arg(fileUrl.fileName(), targetProject));
    }

    if (!targetProject.isEmpty()) {
        m_projects->setSelectedProject(targetProject);
    }
}

void DataIoController::importGeneric(const QUrl &fileUrl, const QString &projectName)
{
    if (!m_projects || !m_bomModel) {
        emit statusMessage(QStringLiteral("Import failed: data controller is not ready."));
        return;
    }

    const QString localFile = fileUrl.toLocalFile();
    if (localFile.isEmpty()) {
        emit statusMessage(QStringLiteral("Import failed: no file selected"));
        return;
    }

    const QString targetProject = projectName.trimmed();
    if (targetProject.isEmpty() || targetProject == QStringLiteral("All Projects")) {
        emit statusMessage(QStringLiteral("Import failed: select a specific project"));
        return;
    }

    m_projects->addProject(targetProject);

    const ImportResult result = m_importService.importGenericSpreadsheet(localFile, targetProject);
    if (!result.ok) {
        emit statusMessage(QStringLiteral("Import failed: %1").arg(result.error));
        return;
    }

    const int projectColumn = findProjectColumn(result.headers);
    if (projectColumn >= 0) {
        const QStringList imported = collectProjects(result.rows, projectColumn);
        for (const QString &name : imported) {
            m_projects->addProject(name);
        }
    }

    if (!m_bomModel->appendRows(result.headers, result.rows)) {
        m_bomModel->setSourceData(result.headers, result.rows);
        emit statusMessage(QStringLiteral("Imported and replaced BOM headers due to new template."));
    } else {
        emit statusMessage(QStringLiteral("Imported %1 -> %2").arg(fileUrl.fileName(), targetProject));
    }

    if (!targetProject.isEmpty()) {
        m_projects->setSelectedProject(targetProject);
    }
}
bool DataIoController::exportCsv(const QUrl &fileUrl)
{
    if (!m_bomModel) {
        emit statusMessage(QStringLiteral("Export failed: data controller is not ready."));
        return false;
    }

    const QString localFile = fileUrl.toLocalFile();
    if (localFile.isEmpty()) {
        emit statusMessage(QStringLiteral("Export failed: no file selected"));
        return false;
    }

    QFile out(localFile);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        emit statusMessage(QStringLiteral("Export failed: cannot write %1").arg(localFile));
        return false;
    }

    auto escapeCsv = [](const QString &value) {
        QString text = value;
        text.replace(QStringLiteral("\""), QStringLiteral("\"\""));
        if (text.contains(',') || text.contains('"') || text.contains('\n') || text.contains('\r')) {
            return QStringLiteral("\"%1\"").arg(text);
        }
        return text;
    };

    QTextStream stream(&out);
    stream.setEncoding(QStringConverter::Utf8);

    const int cols = m_bomModel->columnCount();
    const int rows = m_bomModel->rowCount();

    QStringList headerCells;
    headerCells.reserve(cols);
    for (int c = 0; c < cols; ++c) {
        headerCells.append(escapeCsv(m_bomModel->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString()));
    }
    stream << headerCells.join(',') << '\n';

    for (int r = 0; r < rows; ++r) {
        QStringList rowCells;
        rowCells.reserve(cols);
        for (int c = 0; c < cols; ++c) {
            rowCells.append(escapeCsv(m_bomModel->data(m_bomModel->index(r, c), Qt::DisplayRole).toString()));
        }
        stream << rowCells.join(',') << '\n';
    }

    out.close();
    emit statusMessage(QStringLiteral("Exported CSV: %1 (%2 rows)").arg(fileUrl.fileName()).arg(rows));
    return true;
}

