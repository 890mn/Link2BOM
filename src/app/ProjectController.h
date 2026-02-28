#pragma once

#include <QObject>
#include <QStringListModel>

class ProjectController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)
    Q_PROPERTY(QString selectedProject READ selectedProject WRITE setSelectedProject NOTIFY selectedProjectChanged)

public:
    explicit ProjectController(QObject *parent = nullptr);

    QAbstractItemModel *model();
    QString selectedProject() const;
    void setSelectedProject(const QString &name);

    Q_INVOKABLE QStringList projectNames(bool includeAll = false) const;
    Q_INVOKABLE bool addProject(const QString &name);
    Q_INVOKABLE bool renameProject(int index, const QString &name);
    Q_INVOKABLE bool removeProject(int index);
    Q_INVOKABLE void clearSelection();

signals:
    void selectedProjectChanged();

private:
    QStringListModel m_model;
    QString m_selectedProject = QStringLiteral("Default Project");
};
