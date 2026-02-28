#pragma once

#include <QObject>
#include <QStringListModel>

class CategoryController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)

public:
    explicit CategoryController(QObject *parent = nullptr);

    QAbstractItemModel *model();
    Q_INVOKABLE bool addCategory(const QString &name);
    Q_INVOKABLE bool renameCategory(int index, const QString &name);
    Q_INVOKABLE bool removeCategory(int index);

private:
    QStringListModel m_model;
};
