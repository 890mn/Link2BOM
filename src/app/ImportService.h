#pragma once

#include <QObject>
#include <QString>

#include "ImportTypes.h"
#include "SpreadsheetConverter.h"
#include "CsvParsers.h"

class ImportService : public QObject
{
    Q_OBJECT
public:
    explicit ImportService(QObject *parent = nullptr);

    ImportResult importLichuangSpreadsheet(const QString &filePath, const QString &projectName) const;
    ImportResult importGenericSpreadsheet(const QString &filePath, const QString &projectName) const;

private:
    SpreadsheetConverter m_converter;
    LichuangCsvParser m_lichuangParser;
    GenericCsvParser m_genericParser;
};

