#pragma once

#include <QObject>
#include <QVariantList>
#include <QJsonArray>
#include <QString>

class ProjectController;
class CategoryController;
class BomTableModel;

class ArchiveController : public QObject
{
    Q_OBJECT

public:
    explicit ArchiveController(ProjectController *projects,
                               CategoryController *categories,
                               BomTableModel *bomModel,
                               QObject *parent = nullptr);

    Q_INVOKABLE QVariantList listSlots() const;
    Q_INVOKABLE bool saveSlot(int index, const QString &label = QString(), const QString &customPath = QString());
    Q_INVOKABLE bool loadSlot(int index);
    Q_INVOKABLE bool deleteSlot(int index);

private:
    QString baseDir() const;
    QString registryPath() const;
    QString defaultSlotPath(int index) const;
    QString resolveSlotPath(int index) const;
    QVariantMap loadRegistry() const;
    bool saveRegistry(const QVariantMap &registry) const;
    QVariantList readRows(const QJsonArray &rows) const;
    QJsonArray writeRows(const QVariantList &rows) const;

    ProjectController *m_projects = nullptr;
    CategoryController *m_categories = nullptr;
    BomTableModel *m_bomModel = nullptr;
};

