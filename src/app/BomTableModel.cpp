#include "BomTableModel.h"

#include <QHash>
#include <QRegularExpression>
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
    if (m_sourceHeaders.isEmpty() || m_sourceRows.isEmpty()) {
        return result;
    }

    const QString project = m_projectFilter.trimmed();
    const QString typeFilterValue = m_typeFilter.trimmed();
    const bool allProjects = project.isEmpty() || project.compare(QStringLiteral("All Projects"), Qt::CaseInsensitive) == 0;
    const bool hasTypeFilter = !typeFilterValue.isEmpty();

    QList<QStringList> scopedRows;
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
        scopedRows.append(row);
    }
    if (scopedRows.isEmpty()) {
        return result;
    }

    int keyColumn = -1;
    const QString mode = groupMode.trimmed().toLower();
    if (mode == QStringLiteral("project")) {
        keyColumn = 0;
    } else if (mode == QStringLiteral("package")) {
        keyColumn = findSourceColumnByAliases(m_sourceHeaders, {"package", "footprint"}, 4);
    } else if (mode == QStringLiteral("brand")) {
        keyColumn = findSourceColumnByAliases(m_sourceHeaders, {"brand", "manufacturer", "mfr"}, 2);
    } else {
        keyColumn = findSourceColumnByAliases(m_sourceHeaders, {"part", "pn", "mpn", "item"}, 3);
    }

    if (keyColumn < 0 || keyColumn >= m_sourceHeaders.size()) {
        return result;
    }

    QHash<QString, QList<int>> groupRows;
    for (int rowIndex = 0; rowIndex < scopedRows.size(); ++rowIndex) {
        const QStringList &row = scopedRows[rowIndex];
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
                const QStringList &r = scopedRows[idx];
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

QVariantMap BomTableModel::buildAnalytics(const QString &groupMode) const
{
    QVariantMap out;
    if (m_sourceHeaders.isEmpty() || m_sourceRows.isEmpty()) {
        return out;
    }

    const QString project = m_projectFilter.trimmed();
    const QString typeFilterValue = m_typeFilter.trimmed();
    const bool allProjects = project.isEmpty() || project.compare(QStringLiteral("All Projects"), Qt::CaseInsensitive) == 0;
    const bool hasTypeFilter = !typeFilterValue.isEmpty();

    QList<QStringList> scopedRows;
    scopedRows.reserve(m_sourceRows.size());
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
        scopedRows.append(row);
    }
    if (scopedRows.isEmpty()) {
        return out;
    }

    int groupColumn = 0;
    const QString mode = groupMode.trimmed().toLower();
    if (mode == QStringLiteral("package")) {
        groupColumn = findSourceColumnByAliases(m_sourceHeaders, {"package", "footprint"}, 4);
    } else if (mode == QStringLiteral("brand")) {
        groupColumn = findSourceColumnByAliases(m_sourceHeaders, {"brand", "manufacturer", "mfr"}, 2);
    } else {
        groupColumn = 0;
    }
    if (groupColumn < 0 || groupColumn >= m_sourceHeaders.size()) {
        groupColumn = 0;
    }

    const int partColumn = findSourceColumnByAliases(m_sourceHeaders, {"part", "pn", "mpn", "item"}, 3);
    const int qtyColumn = findSourceColumnByAliases(m_sourceHeaders, {"qty", "quantity", "q'ty", "amount"}, -1);

    QHash<QString, int> groupCounts;
    QHash<QString, int> partCounts;
    QSet<QString> uniqueParts;
    int missingPartCount = 0;
    int lowQtyCount = 0;

    const QRegularExpression nonNumeric(QStringLiteral("[^0-9.\\-]"));
    for (const QStringList &row : scopedRows) {
        const QString groupName = (groupColumn < row.size() ? row[groupColumn].trimmed() : QString());
        groupCounts[groupName.isEmpty() ? QStringLiteral("(Empty)") : groupName] += 1;

        QString part = (partColumn >= 0 && partColumn < row.size()) ? row[partColumn].trimmed() : QString();
        if (part.isEmpty()) {
            missingPartCount += 1;
        } else {
            uniqueParts.insert(part);
            partCounts[part] += 1;
        }

        if (qtyColumn >= 0 && qtyColumn < row.size()) {
            QString qtyText = row[qtyColumn].trimmed();
            qtyText.remove(nonNumeric);
            bool ok = false;
            const double qty = qtyText.toDouble(&ok);
            if (ok && qty <= 1.0) {
                lowQtyCount += 1;
            }
        }
    }

    int duplicatePartCount = 0;
    for (auto it = partCounts.cbegin(); it != partCounts.cend(); ++it) {
        if (it.value() > 1) {
            duplicatePartCount += it.value();
        }
    }

    struct GroupItem {
        QString name;
        int count = 0;
    };
    QList<GroupItem> sorted;
    sorted.reserve(groupCounts.size());
    for (auto it = groupCounts.cbegin(); it != groupCounts.cend(); ++it) {
        GroupItem item;
        item.name = it.key();
        item.count = it.value();
        sorted.append(item);
    }
    std::sort(sorted.begin(), sorted.end(), [](const GroupItem &a, const GroupItem &b) {
        if (a.count != b.count) {
            return a.count > b.count;
        }
        return QString::localeAwareCompare(a.name, b.name) < 0;
    });

    QVariantList groupItems;
    const int total = scopedRows.size();
    const int maxItems = 10;
    for (int i = 0; i < qMin(sorted.size(), maxItems); ++i) {
        QVariantMap item;
        item.insert(QStringLiteral("name"), sorted[i].name);
        item.insert(QStringLiteral("count"), sorted[i].count);
        item.insert(QStringLiteral("ratio"), total > 0 ? double(sorted[i].count) / double(total) : 0.0);
        groupItems.append(item);
    }

    const int healthyCount = qMax(0, total - lowQtyCount - missingPartCount);
    const int issueCount = qMin(total, lowQtyCount + missingPartCount);
    const int duplicatePenalty = qMin(25, duplicatePartCount * 2);
    const int missingPenalty = qMin(40, (missingPartCount * 100) / qMax(1, total));
    const int lowQtyPenalty = qMin(30, (lowQtyCount * 100) / qMax(1, total));
    const int healthScore = qMax(0, 100 - duplicatePenalty - missingPenalty - lowQtyPenalty);

    QVariantList pieItems;
    {
        QVariantMap a;
        a.insert(QStringLiteral("label"), QStringLiteral("Healthy"));
        a.insert(QStringLiteral("value"), healthyCount);
        a.insert(QStringLiteral("color"), QStringLiteral("#10B981"));
        pieItems.append(a);
        QVariantMap b;
        b.insert(QStringLiteral("label"), QStringLiteral("At Risk"));
        b.insert(QStringLiteral("value"), qMax(0, lowQtyCount));
        b.insert(QStringLiteral("color"), QStringLiteral("#F59E0B"));
        pieItems.append(b);
        QVariantMap c;
        c.insert(QStringLiteral("label"), QStringLiteral("Missing Key"));
        c.insert(QStringLiteral("value"), qMax(0, missingPartCount));
        c.insert(QStringLiteral("color"), QStringLiteral("#EF4444"));
        pieItems.append(c);
    }

    QStringList suggestions;
    if (missingPartCount > 0) {
        suggestions.append(QStringLiteral("Some rows miss key part numbers; prioritize fixing identification fields."));
    }
    if (lowQtyCount > 0) {
        suggestions.append(QStringLiteral("Several items have low quantity (<=1); review replenishment priority."));
    }
    if (duplicatePartCount > 0) {
        suggestions.append(QStringLiteral("Duplicate part definitions detected; consider consolidating equivalent entries."));
    }
    if (suggestions.isEmpty()) {
        suggestions.append(QStringLiteral("Inventory structure looks stable. Keep periodic checks before procurement."));
    }

    out.insert(QStringLiteral("groupItems"), groupItems);
    out.insert(QStringLiteral("pieItems"), pieItems);
    out.insert(QStringLiteral("totalRows"), total);
    out.insert(QStringLiteral("uniquePartCount"), uniqueParts.size());
    out.insert(QStringLiteral("lowQtyCount"), lowQtyCount);
    out.insert(QStringLiteral("missingPartCount"), missingPartCount);
    out.insert(QStringLiteral("duplicatePartCount"), duplicatePartCount);
    out.insert(QStringLiteral("healthScore"), healthScore);
    out.insert(QStringLiteral("suggestions"), suggestions);
    return out;
}

QVariantMap BomTableModel::exportSnapshot() const
{
    QVariantMap snapshot;
    snapshot.insert(QStringLiteral("headers"), m_sourceHeaders);
    QVariantList rows;
    rows.reserve(m_sourceRows.size());
    for (const QStringList &row : m_sourceRows) {
        rows.append(row);
    }
    snapshot.insert(QStringLiteral("rows"), rows);
    return snapshot;
}

bool BomTableModel::importSnapshot(const QVariantMap &snapshot)
{
    const QStringList headers = snapshot.value(QStringLiteral("headers")).toStringList();
    const QVariantList rowsVar = snapshot.value(QStringLiteral("rows")).toList();
    QList<QStringList> rows;
    rows.reserve(rowsVar.size());
    for (const QVariant &rowVar : rowsVar) {
        rows.append(rowVar.toStringList());
    }

    if (headers.isEmpty()) {
        return false;
    }

    setSourceData(headers, rows);
    return true;
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


