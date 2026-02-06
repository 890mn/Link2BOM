#include "CategoryController.h"

CategoryController::CategoryController(QObject *parent)
    : QObject(parent)
{
    m_model.setStringList({QStringLiteral("电阻电容"), QStringLiteral("芯片 IC"), QStringLiteral("连接器"), QStringLiteral("机械件")});
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
