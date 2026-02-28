#pragma once

#include <QObject>
#include <QUrl>
#include <QHash>
#include <QVariant>

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
    Q_INVOKABLE bool deleteProject(int index);
    Q_INVOKABLE void notify(const QString &message);
    Q_INVOKABLE QVariantList loadBomWidthRatios(const QString &layoutHash) const;
    Q_INVOKABLE void saveBomWidthRatios(const QString &layoutHash, const QVariantList &ratios);

signals:
    void statusChanged();

private:
    void setStatus(const QString &status);
    void loadUiSettings();
    void saveUiSettings() const;

    ThemeController m_theme;
    ProjectController m_projects;
    CategoryController m_categories;
    BomTableModel m_bomModel;
    ImportService m_importService;
    QString m_status;
    QHash<QString, QVariantList> m_bomWidthRatiosByLayout;
};
