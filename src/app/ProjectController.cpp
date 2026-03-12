#include "ProjectController.h"

ProjectController::ProjectController(QObject *parent)
    : QObject(parent)
{
    m_model.setStringList({QStringLiteral("All Projects"),
                           QStringLiteral("Default Project")});
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
        names.removeAll(QStringLiteral("All Projects"));
    }
    return names;
}

QStringList ProjectController::allProjectNames() const
{
    return m_model.stringList();
}

void ProjectController::setProjectNames(const QStringList &names, const QString &selected)
{
    QStringList list;
    list.append(QStringLiteral("All Projects"));
    for (const QString &name : names) {
        const QString trimmed = name.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        if (!list.contains(trimmed)) {
            list.append(trimmed);
        }
    }
    if (!list.contains(QStringLiteral("Default Project"))) {
        list.append(QStringLiteral("Default Project"));
    }
    m_model.setStringList(list);

    const QString target = selected.trimmed();
    if (!target.isEmpty() && list.contains(target)) {
        setSelectedProject(target);
    } else if (list.contains(QStringLiteral("Default Project"))) {
        setSelectedProject(QStringLiteral("Default Project"));
    } else {
        setSelectedProject(list.value(0));
    }
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
    if (index < 0 || index >= names.size() || names[index] == QStringLiteral("All Projects")) {
        return false;
    }
    names[index] = trimmed;
    m_model.setStringList(names);
    setSelectedProject(trimmed);
    return true;
}

bool ProjectController::removeProject(int index)
{
    QStringList names = m_model.stringList();
    if (index < 0 || index >= names.size() || names[index] == QStringLiteral("All Projects")) {
        return false;
    }

    const QString removed = names[index];
    names.removeAt(index);
    m_model.setStringList(names);

    if (m_selectedProject == removed) {
        setSelectedProject(QStringLiteral("All Projects"));
    }
    return true;
}

void ProjectController::clearSelection()
{
    setSelectedProject(QStringLiteral("All Projects"));
}
