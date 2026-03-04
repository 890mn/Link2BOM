#include "BomTableModel.h"

#include <QHash>
#include <QSet>
#include <QVariantMap>
#include <algorithm>

namespace {
int findSourceColumnByAliases(const QStringList &headers, const QStringList &aliases, int fallback = -1)
{
    for (int i = 0; i < headers.size(); ++i) {
        const QString header = headers[i].trimmed().toLower();
        for (const QString &aliasRaw : aliases) {
            const QString alias = aliasRaw.trimmed().toLower();
            if (!alias.isEmpty() && (header == alias || header.contains(alias))) {
                return i;
            }
        }
    }
    return fallback;
}
}

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

QVariantList BomTableModel::analyzeDifferences(const QString &keyword, const QString &groupMode) const
{
    QVariantList result;
    if (m_sourceHeaders.isEmpty() || m_filteredRows.isEmpty()) {
        return result;
    }

    int keyColumn = -1;
    const QString mode = groupMode.trimmed().toLower();
    if (mode == QStringLiteral("project")) {
        keyColumn = 0;
    } else if (mode == QStringLiteral("name")) {
        keyColumn = findSourceColumnByAliases(m_sourceHeaders, {"name", "description", "item"}, 5);
    } else if (mode == QStringLiteral("package")) {
        keyColumn = findSourceColumnByAliases(m_sourceHeaders, {"package", "footprint"}, 4);
    } else {
        keyColumn = findSourceColumnByAliases(m_sourceHeaders, {"part", "pn", "mpn", "item"}, 3);
    }

    if (keyColumn < 0 || keyColumn >= m_sourceHeaders.size()) {
        return result;
    }

    QHash<QString, QList<int>> groupRows;
    for (int rowIndex = 0; rowIndex < m_filteredRows.size(); ++rowIndex) {
        const QStringList &row = m_filteredRows[rowIndex];
        if (keyColumn >= row.size()) {
            continue;
        }
        const QString key = row[keyColumn].trimmed();
        if (!key.isEmpty()) {
            groupRows[key].append(rowIndex);
        }
    }

    struct DiffEntry {
        QString key;
        int rowCount = 0;
        int changedFieldCount = 0;
        QString changedFields;
        QString details;
        QVariantList fieldDetails;
    };
    QList<DiffEntry> entries;

    const QString keywordKey = keyword.trimmed();
    for (auto it = groupRows.cbegin(); it != groupRows.cend(); ++it) {
        const QList<int> &rows = it.value();
        if (rows.size() < 2) {
            continue;
        }

        QStringList changedFields;
        QStringList detailParts;
        QVariantList fieldDetails;
        for (int col = 0; col < m_sourceHeaders.size(); ++col) {
            if (col == keyColumn || col == 0) {
                continue;
            }
            QSet<QString> uniq;
            for (int idx : rows) {
                const QStringList &r = m_filteredRows[idx];
                if (col < r.size()) {
                    const QString value = r[col].trimmed();
                    if (!value.isEmpty()) {
                        uniq.insert(value);
                    }
                }
            }
            if (uniq.size() > 1) {
                changedFields.append(m_sourceHeaders[col]);
                QStringList values = uniq.values();
                std::sort(values.begin(), values.end(), [](const QString &a, const QString &b) {
                    return QString::localeAwareCompare(a, b) < 0;
                });
                if (values.size() > 3) {
                    values = values.mid(0, 3);
                    values.append(QStringLiteral("..."));
                }
                QVariantMap fieldItem;
                fieldItem.insert(QStringLiteral("field"), m_sourceHeaders[col]);
                fieldItem.insert(QStringLiteral("values"), QVariant::fromValue(values));
                fieldDetails.append(fieldItem);
                detailParts.append(m_sourceHeaders[col] + QStringLiteral(": ") + values.join(QStringLiteral(" | ")));
            }
        }

        if (changedFields.isEmpty()) {
            continue;
        }

        DiffEntry entry;
        entry.key = it.key();
        entry.rowCount = rows.size();
        entry.changedFieldCount = changedFields.size();
        entry.changedFields = changedFields.join(QStringLiteral(", "));
        entry.details = detailParts.join(QStringLiteral(" ; "));
        entry.fieldDetails = fieldDetails;

        if (!keywordKey.isEmpty()) {
            const bool matched = entry.key.contains(keywordKey, Qt::CaseInsensitive)
                || entry.changedFields.contains(keywordKey, Qt::CaseInsensitive)
                || entry.details.contains(keywordKey, Qt::CaseInsensitive);
            if (!matched) {
                continue;
            }
        }

        entries.append(entry);
    }

    std::sort(entries.begin(), entries.end(), [](const DiffEntry &a, const DiffEntry &b) {
        if (a.changedFieldCount != b.changedFieldCount) {
            return a.changedFieldCount > b.changedFieldCount;
        }
        if (a.rowCount != b.rowCount) {
            return a.rowCount > b.rowCount;
        }
        return QString::localeAwareCompare(a.key, b.key) < 0;
    });

    const int maxItems = 300;
    const int count = qMin(entries.size(), maxItems);
    for (int i = 0; i < count; ++i) {
        const DiffEntry &entry = entries[i];
        QVariantMap map;
        map.insert(QStringLiteral("key"), entry.key);
        map.insert(QStringLiteral("rowCount"), entry.rowCount);
        map.insert(QStringLiteral("changedFieldCount"), entry.changedFieldCount);
        map.insert(QStringLiteral("changedFields"), entry.changedFields);
        map.insert(QStringLiteral("details"), entry.details);
        map.insert(QStringLiteral("fieldDetails"), entry.fieldDetails);
        result.append(map);
    }

    return result;
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
        || project.compare(QStringLiteral("All Projects"), Qt::CaseInsensitive) == 0;

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
