#include "ImportService.h"
#include "AppLogger.h"

ImportService::ImportService(QObject *parent)
    : QObject(parent)
{
}

ImportResult ImportService::importLichuangSpreadsheet(const QString &filePath, const QString &projectName) const
{
    AppLogger::info(QStringLiteral("Import request: file=%1, project=%2").arg(filePath, projectName));
    ImportResult result;
    if (filePath.isEmpty()) {
        result.error = QStringLiteral("File path is empty.");
        AppLogger::error(result.error);
        return result;
    }

    QString csvPath;
    QString error;
    if (!m_converter.toCsv(filePath, &csvPath, &error)) {
        result.error = QStringLiteral("%1\nSee import log: %2").arg(error, AppLogger::logFilePath());
        AppLogger::error(QStringLiteral("convertSpreadsheetToCsv failed: %1").arg(error));
        return result;
    }

    result = m_lichuangParser.parseFile(csvPath, projectName);
    if (!result.ok) {
        AppLogger::error(QStringLiteral("parseLichuangCsv failed: %1").arg(result.error));
        result.error = QStringLiteral("%1\nSee import log: %2").arg(result.error, AppLogger::logFilePath());
    } else {
        AppLogger::info(QStringLiteral("Import success: file=%1 rows=%2 project=%3")
                            .arg(filePath)
                            .arg(result.rows.size())
                            .arg(projectName));
    }
    return result;
}

ImportResult ImportService::importGenericSpreadsheet(const QString &filePath, const QString &projectName) const
{
    AppLogger::info(QStringLiteral("Generic import request: file=%1, project=%2").arg(filePath, projectName));
    ImportResult result;
    if (filePath.isEmpty()) {
        result.error = QStringLiteral("File path is empty.");
        AppLogger::error(result.error);
        return result;
    }

    QString csvPath;
    QString error;
    if (!m_converter.toCsv(filePath, &csvPath, &error)) {
        result.error = QStringLiteral("%1\nSee import log: %2").arg(error, AppLogger::logFilePath());
        AppLogger::error(QStringLiteral("convertSpreadsheetToCsv failed: %1").arg(error));
        return result;
    }

    result = m_genericParser.parseFile(csvPath, projectName);
    if (!result.ok) {
        AppLogger::error(QStringLiteral("parseGenericCsv failed: %1").arg(result.error));
        result.error = QStringLiteral("%1\nSee import log: %2").arg(result.error, AppLogger::logFilePath());
    } else {
        AppLogger::info(QStringLiteral("Generic import success: file=%1 rows=%2 project=%3")
                            .arg(filePath)
                            .arg(result.rows.size())
                            .arg(projectName));
    }
    return result;
}
