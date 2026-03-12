#include "CategoryController.h"

CategoryController::CategoryController(QObject *parent)
    : QObject(parent)
{
    m_model.setStringList({QStringLiteral("R/C"),
                           QStringLiteral("IC"),
                           QStringLiteral("Connector"),
                           QStringLiteral("Mechanical")});
}

QAbstractItemModel *CategoryController::model()
{
    return &m_model;
}

bool CategoryController::addCategory(const QString &name)
{
    const QString n = name.trimmed();
    if (n.isEmpty()) {
        return false;
    }
    QStringList list = m_model.stringList();
    if (list.contains(n)) {
        return true;
    }
    list.append(n);
    m_model.setStringList(list);
    return true;
}

bool CategoryController::renameCategory(int index, const QString &name)
{
    const QString n = name.trimmed();
    if (n.isEmpty()) {
        return false;
    }
    QStringList list = m_model.stringList();
    if (index < 0 || index >= list.size()) {
        return false;
    }
    list[index] = n;
    m_model.setStringList(list);
    return true;
}

bool CategoryController::removeCategory(int index)
{
    QStringList list = m_model.stringList();
    if (index < 0 || index >= list.size()) {
        return false;
    }
    list.removeAt(index);
    m_model.setStringList(list);
    return true;
}

QStringList CategoryController::categoryNames() const
{
    return m_model.stringList();
}

void CategoryController::setCategoryNames(const QStringList &names)
{
    QStringList list;
    for (const QString &name : names) {
        const QString trimmed = name.trimmed();
        if (!trimmed.isEmpty() && !list.contains(trimmed)) {
            list.append(trimmed);
        }
    }
    if (list.isEmpty()) {
        list = m_model.stringList();
    }
    m_model.setStringList(list);
}
