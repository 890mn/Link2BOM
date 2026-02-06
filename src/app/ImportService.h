#pragma once

#include <QList>
#include <QObject>
#include <QStringList>

struct ImportResult {
    bool ok = false;
    QString error;
    QStringList headers;
    QList<QStringList> rows;
};

class ImportService : public QObject
{
    Q_OBJECT
public:
    explicit ImportService(QObject *parent = nullptr);

    ImportResult importLichuangSpreadsheet(const QString &filePath, const QString &projectName) const;

private:
    QStringList parseCsvLine(const QString &line) const;
    bool convertSpreadsheetToCsv(const QString &inputPath, QString *outputCsvPath, QString *error) const;
    bool convertXlsxToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const;
    bool convertExcelToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const;
    ImportResult parseLichuangCsv(const QString &csvPath, const QString &projectName) const;
};
