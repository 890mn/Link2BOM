#include "BomTableModel.h"

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

    beginResetModel();
    std::sort(m_sourceRows.begin(), m_sourceRows.end(), [sourceIndex, ascending](const QStringList &a, const QStringList &b) {
        const QString left = sourceIndex < a.size() ? a[sourceIndex] : QString();
        const QString right = sourceIndex < b.size() ? b[sourceIndex] : QString();
        return ascending ? left < right : left > right;
    });
    endResetModel();
    rebuildFilteredRows();
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

void BomTableModel::rebuildFilteredRows()
{
    beginResetModel();
    m_filteredRows.clear();
    const QString key = m_filterKeyword.trimmed();
    if (key.isEmpty()) {
        m_filteredRows = m_sourceRows;
    } else {
        for (const QStringList &row : m_sourceRows) {
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
