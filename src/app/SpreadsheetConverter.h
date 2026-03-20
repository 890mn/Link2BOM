#pragma once

#include <QString>

class SpreadsheetConverter
{
public:
    bool toCsv(const QString &inputPath, QString *outputCsvPath, QString *error) const;

private:
    bool convertSpreadsheetToCsv(const QString &inputPath, QString *outputCsvPath, QString *error) const;
    bool convertExcelToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const;
    bool convertXlsxToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const;
};
