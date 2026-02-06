#include "ProjectController.h"

ProjectController::ProjectController(QObject *parent)
    : QObject(parent)
{
    m_model.setStringList({QStringLiteral("全部项目"),
                           QStringLiteral("电源管理板 RevA"),
                           QStringLiteral("传感器节点 V2"),
                           QStringLiteral("验证样机 Proto-3")});
}

QAbstractItemModel *ProjectController::model()
{
    return &m_model;
}

QString ProjectController::selectedProject() const
{
    return m_selectedProject;
}

void ProjectController::setSelectedProject(const QString &name)
{
    if (name.isEmpty() || m_selectedProject == name) {
        return;
    }
    m_selectedProject = name;
    emit selectedProjectChanged();
}

QStringList ProjectController::projectNames(bool includeAll) const
{
    QStringList names = m_model.stringList();
    if (!includeAll) {
        names.removeAll(QStringLiteral("全部项目"));
    }
    return names;
}

bool ProjectController::addProject(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }

    QStringList names = m_model.stringList();
    if (!names.contains(trimmed)) {
        names.append(trimmed);
        m_model.setStringList(names);
    }
    setSelectedProject(trimmed);
    return true;
}

bool ProjectController::renameProject(int index, const QString &name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }

    QStringList names = m_model.stringList();
    if (index < 0 || index >= names.size() || names[index] == QStringLiteral("全部项目")) {
        return false;
    }
    names[index] = trimmed;
    m_model.setStringList(names);
    setSelectedProject(trimmed);
    return true;
}

void ProjectController::clearSelection()
{
    setSelectedProject(QStringLiteral("全部项目"));
}
