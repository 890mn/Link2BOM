#pragma once

#include <QStringList>
#include <QList>

class ArchiveController;
class ProjectController;
class CategoryController;
class BomTableModel;

class DefaultDataSeeder
{
public:
    void initialize(ArchiveController &archive,
                    ProjectController &projects,
                    CategoryController &categories,
                    BomTableModel &model) const;

private:
    struct SeedData {
        QStringList headers;
        QList<QStringList> rows;
        QString defaultProjectName;
    };

    SeedData seedData() const;
};
