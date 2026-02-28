#include "BomTableModel.h"

#include <QSet>
#include <algorithm>

BomTableModel::BomTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int BomTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_filteredRows.size();
}

int BomTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_visibleSourceColumns.size();
}

QVariant BomTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole || index.row() >= m_filteredRows.size() || index.column() >= m_visibleSourceColumns.size()) {
        return {};
    }

    const int sourceIndex = m_visibleSourceColumns[index.column()];
    const QStringList &row = m_filteredRows[index.row()];
    return (sourceIndex >= 0 && sourceIndex < row.size()) ? row[sourceIndex] : QVariant();
}

QVariant BomTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation == Qt::Horizontal) {
        if (section < 0 || section >= m_visibleSourceColumns.size()) {
            return {};
        }
        const int sourceIndex = m_visibleSourceColumns[section];
        return (sourceIndex >= 0 && sourceIndex < m_sourceHeaders.size()) ? m_sourceHeaders[sourceIndex] : QVariant();
    }
    return section + 1;
}

QStringList BomTableModel::availableHeaders() const
{
    return m_sourceHeaders;
}

QString BomTableModel::visibleHeaderAt(int slot) const
{
    if (slot < 0 || slot >= m_visibleSourceColumns.size()) {
        return QString();
    }
    return m_sourceHeaders.value(m_visibleSourceColumns[slot]);
}

void BomTableModel::setVisibleHeaderAt(int slot, const QString &header)
{
    if (slot < 0 || slot >= m_visibleSourceColumns.size()) {
        return;
    }

    const int sourceIndex = m_sourceHeaders.indexOf(header);
    if (sourceIndex < 0 || sourceIndex == m_visibleSourceColumns[slot]) {
        return;
    }

    m_visibleSourceColumns[slot] = sourceIndex;
    emit headerDataChanged(Qt::Horizontal, slot, slot);
    if (!m_filteredRows.isEmpty()) {
        emit dataChanged(index(0, slot), index(m_filteredRows.size() - 1, slot));
    }
}

int BomTableModel::visibleSlotCount() const
{
    return m_visibleSourceColumns.size();
}

void BomTableModel::sortByVisibleColumn(int slot, bool ascending)
{
    if (slot < 0 || slot >= m_visibleSourceColumns.size()) {
        return;
    }

    const int sourceIndex = m_visibleSourceColumns[slot];

    std::sort(m_sourceRows.begin(), m_sourceRows.end(), [sourceIndex, ascending](const QStringList &a, const QStringList &b) {
        const QString left = sourceIndex < a.size() ? a[sourceIndex] : QString();
        const QString right = sourceIndex < b.size() ? b[sourceIndex] : QString();
        return ascending ? left < right : left > right;
    });

    rebuildFilteredRows();
}

void BomTableModel::insertVisibleSlot(int slot)
{
    if (m_sourceHeaders.isEmpty()) {
        return;
    }

    const int insertPos = std::clamp(slot, 0, static_cast<int>(m_visibleSourceColumns.size()));

    int newSourceIndex = 0;
    for (int i = 0; i < m_sourceHeaders.size(); ++i) {
        if (!m_visibleSourceColumns.contains(i)) {
            newSourceIndex = i;
            break;
        }
    }

    beginResetModel();
    m_visibleSourceColumns.insert(insertPos, newSourceIndex);
    endResetModel();
}

void BomTableModel::removeVisibleSlot(int slot)
{
    if (slot < 0 || slot >= m_visibleSourceColumns.size() || m_visibleSourceColumns.size() <= 1) {
        return;
    }

    beginResetModel();
    m_visibleSourceColumns.removeAt(slot);
    endResetModel();
}

QStringList BomTableModel::distinctValuesByHeaderAliases(const QStringList &aliases, int fallbackSourceColumn) const
{
    if (m_sourceHeaders.isEmpty() || aliases.isEmpty()) {
        return {};
    }

    int sourceIndex = -1;
    for (int i = 0; i < m_sourceHeaders.size(); ++i) {
        const QString header = m_sourceHeaders[i].trimmed().toLower();
        for (const QString &aliasRaw : aliases) {
            const QString alias = aliasRaw.trimmed().toLower();
            if (!alias.isEmpty() && (header == alias || header.contains(alias))) {
                sourceIndex = i;
                break;
            }
        }
        if (sourceIndex >= 0) {
            break;
        }
    }

    if (sourceIndex < 0) {
        sourceIndex = fallbackSourceColumn;
    }
    if (sourceIndex < 0 || sourceIndex >= m_sourceHeaders.size()) {
        return {};
    }

    QSet<QString> uniq;
    for (const QStringList &row : m_filteredRows) {
        if (sourceIndex >= row.size()) {
            continue;
        }
        const QString value = row[sourceIndex].trimmed();
        if (!value.isEmpty()) {
            uniq.insert(value);
        }
    }

    QStringList values = uniq.values();
    std::sort(values.begin(), values.end(), [](const QString &a, const QString &b) {
        return QString::localeAwareCompare(a, b) < 0;
    });
    return values;
}

QString BomTableModel::filterKeyword() const
{
    return m_filterKeyword;
}

void BomTableModel::setFilterKeyword(const QString &keyword)
{
    if (keyword == m_filterKeyword) {
        return;
    }

    m_filterKeyword = keyword;
    emit filterKeywordChanged();
    rebuildFilteredRows();
}

QString BomTableModel::projectFilter() const
{
    return m_projectFilter;
}

void BomTableModel::setProjectFilter(const QString &project)
{
    if (project == m_projectFilter) {
        return;
    }

    m_projectFilter = project;
    emit projectFilterChanged();
    rebuildFilteredRows();
}

QString BomTableModel::typeFilter() const
{
    return m_typeFilter;
}

void BomTableModel::setTypeFilter(const QString &typeValue)
{
    const QString normalized = typeValue.trimmed();
    if (normalized == m_typeFilter) {
        return;
    }

    m_typeFilter = normalized;
    emit typeFilterChanged();
    rebuildFilteredRows();
}

void BomTableModel::clearTypeFilter()
{
    setTypeFilter(QString());
}

void BomTableModel::removeRowsByProject(const QString &projectName)
{
    const QString key = projectName.trimmed();
    if (key.isEmpty()) {
        return;
    }

    beginResetModel();
    auto endIt = std::remove_if(m_sourceRows.begin(), m_sourceRows.end(), [&key](const QStringList &row) {
        return !row.isEmpty() && row.first().trimmed() == key;
    });
    m_sourceRows.erase(endIt, m_sourceRows.end());
    endResetModel();

    rebuildFilteredRows();
}

void BomTableModel::setSourceData(const QStringList &headers, const QList<QStringList> &rows)
{
    beginResetModel();
    m_sourceHeaders = headers;
    m_sourceRows = rows;
    m_visibleSourceColumns.clear();
    for (int i = 0; i < qMin(6, m_sourceHeaders.size()); ++i) {
        m_visibleSourceColumns.append(i);
    }
    endResetModel();

    rebuildFilteredRows();
}

bool BomTableModel::appendRows(const QStringList &headers, const QList<QStringList> &rows)
{
    if (headers.isEmpty()) {
        return false;
    }

    if (m_sourceHeaders.isEmpty()) {
        setSourceData(headers, rows);
        return true;
    }

    if (m_sourceHeaders != headers) {
        return false;
    }

    beginResetModel();
    for (const QStringList &row : rows) {
        m_sourceRows.append(row);
    }
    endResetModel();

    rebuildFilteredRows();
    return true;
}

void BomTableModel::rebuildFilteredRows()
{
    beginResetModel();
    m_filteredRows.clear();

    const QString key = m_filterKeyword.trimmed();
    const QString project = m_projectFilter.trimmed();
    const QString typeFilterValue = m_typeFilter.trimmed();
    const bool hasTypeFilter = !typeFilterValue.isEmpty();
    const bool allProjects = project.isEmpty()
        || project.compare(QStringLiteral("All Projects"), Qt::CaseInsensitive) == 0
        || project == QStringLiteral("全部项目");

    if (key.isEmpty()) {
        if (allProjects) {
            m_filteredRows = m_sourceRows;
        } else {
            for (const QStringList &row : m_sourceRows) {
                if (!row.isEmpty() && row.first().trimmed() == project) {
                    if (hasTypeFilter) {
                        if (row.size() <= 5 || !row[5].contains(typeFilterValue, Qt::CaseInsensitive)) {
                            continue;
                        }
                    }
                    m_filteredRows.append(row);
                }
            }
        }
    } else {
        for (const QStringList &row : m_sourceRows) {
            if (!allProjects) {
                if (row.isEmpty() || row.first().trimmed() != project) {
                    continue;
                }
            }
            if (hasTypeFilter) {
                if (row.size() <= 5 || !row[5].contains(typeFilterValue, Qt::CaseInsensitive)) {
                    continue;
                }
            }
            bool matched = false;
            for (const QString &cell : row) {
                if (cell.contains(key, Qt::CaseInsensitive)) {
                    matched = true;
                    break;
                }
            }
            if (matched) {
                m_filteredRows.append(row);
            }
        }
    }
    endResetModel();
}
