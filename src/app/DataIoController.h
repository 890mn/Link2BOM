#pragma once

#include <QObject>
#include <QUrl>

#include "BomTableModel.h"
#include "ImportService.h"
#include "ProjectController.h"

class DataIoController : public QObject
{
    Q_OBJECT

public:
    explicit DataIoController(ProjectController *projects, BomTableModel *bomModel, QObject *parent = nullptr);

    Q_INVOKABLE void importLichuang(const QUrl &fileUrl, const QString &projectName);
    Q_INVOKABLE void importGeneric(const QUrl &fileUrl, const QString &projectName);
    Q_INVOKABLE bool exportCsv(const QUrl &fileUrl);

signals:
    void statusMessage(const QString &message);

private:
    ProjectController *m_projects = nullptr;
    BomTableModel *m_bomModel = nullptr;
    ImportService m_importService;
};

