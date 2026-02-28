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
