#pragma once

#include <QAbstractTableModel>

class BomTableModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterKeyword READ filterKeyword WRITE setFilterKeyword NOTIFY filterKeywordChanged)

public:
    explicit BomTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Q_INVOKABLE QStringList availableHeaders() const;
    Q_INVOKABLE QString visibleHeaderAt(int slot) const;
    Q_INVOKABLE void setVisibleHeaderAt(int slot, const QString &header);
    Q_INVOKABLE int visibleSlotCount() const;
    Q_INVOKABLE void sortByVisibleColumn(int slot, bool ascending);
    Q_INVOKABLE void insertVisibleSlot(int slot);
    Q_INVOKABLE void removeVisibleSlot(int slot);

    QString filterKeyword() const;
    Q_INVOKABLE void setFilterKeyword(const QString &keyword);

    void setSourceData(const QStringList &headers, const QList<QStringList> &rows);

signals:
    void filterKeywordChanged();

private:
    void rebuildFilteredRows();

    QStringList m_sourceHeaders;
    QList<QStringList> m_sourceRows;
    QList<QStringList> m_filteredRows;
    QList<int> m_visibleSourceColumns;
    QString m_filterKeyword;
};
