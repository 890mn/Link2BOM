#pragma once

#include <QObject>
#include <QUrl>

#include "BomTableModel.h"
#include "CategoryController.h"
#include "ImportService.h"
#include "ProjectController.h"
#include "ThemeController.h"

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ThemeController *theme READ theme CONSTANT)
    Q_PROPERTY(ProjectController *projects READ projects CONSTANT)
    Q_PROPERTY(CategoryController *categories READ categories CONSTANT)
    Q_PROPERTY(BomTableModel *bomModel READ bomModel CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    ThemeController *theme();
    ProjectController *projects();
    CategoryController *categories();
    BomTableModel *bomModel();
    QString status() const;

    Q_INVOKABLE void cycleTheme();
    Q_INVOKABLE void importLichuang(const QUrl &fileUrl, const QString &projectName);
    Q_INVOKABLE void notify(const QString &message);

signals:
    void statusChanged();

private:
    void setStatus(const QString &status);

    ThemeController m_theme;
    ProjectController m_projects;
    CategoryController m_categories;
    BomTableModel m_bomModel;
    ImportService m_importService;
    QString m_status;
};
