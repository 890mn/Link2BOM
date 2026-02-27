#include "ImportService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringConverter>
#include <QTextStream>
#include <optional>

namespace {
struct PythonCommand {
    QString program;
    QStringList prefixArgs;
};

QString importLogPath()
{
    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return QDir(tempDir).filePath(QStringLiteral("Link2BOM-import.log"));
}

void appendImportLog(const QString &message)
{
    QFile file(importLogPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << " | " << message << '\n';
}

std::optional<PythonCommand> detectPythonCommand()
{
    const QList<PythonCommand> candidates = {
        {QStringLiteral("python3"), {}},
        {QStringLiteral("python"), {}},
        {QStringLiteral("py"), {QStringLiteral("-3")}}
    };

    for (const PythonCommand &candidate : candidates) {
        QProcess check;
        QStringList args = candidate.prefixArgs;
        args.append(QStringLiteral("--version"));
        check.start(candidate.program, args);
        if (!check.waitForStarted(2500)) {
            continue;
        }
        check.waitForFinished(4000);
        if (check.exitStatus() == QProcess::NormalExit && check.exitCode() == 0) {
            return candidate;
        }
    }
    return std::nullopt;
}

bool runPythonInline(const QString &pythonCode, const QStringList &scriptArgs, QString *stdErr, QString *launchError)
{
    const std::optional<PythonCommand> python = detectPythonCommand();
    if (!python.has_value()) {
        if (launchError) {
            *launchError = QStringLiteral("No Python interpreter found (tried: python3, python, py -3).");
        }
        return false;
    }

    QProcess process;
    QStringList args = python->prefixArgs;
    args << QStringLiteral("-c") << pythonCode;
    args << scriptArgs;
    process.start(python->program, args);
    if (!process.waitForStarted(3000)) {
        if (launchError) {
            *launchError = QStringLiteral("Failed to launch Python command: %1 %2")
                .arg(python->program, python->prefixArgs.join(' '));
        }
        return false;
    }

    process.waitForFinished(45000);
    const QString err = QString::fromUtf8(process.readAllStandardError()).trimmed();
    if (stdErr) {
        *stdErr = err;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (launchError) {
            *launchError = QStringLiteral("Python exited with code %1.").arg(process.exitCode());
        }
        return false;
    }
    return true;
}
}

ImportService::ImportService(QObject *parent)
    : QObject(parent)
{
}

ImportResult ImportService::importLichuangSpreadsheet(const QString &filePath, const QString &projectName) const
{
    appendImportLog(QStringLiteral("Import request: file=%1, project=%2").arg(filePath, projectName));
    ImportResult result;
    if (filePath.isEmpty()) {
        result.error = QStringLiteral("File path is empty.");
        appendImportLog(result.error);
        return result;
    }

    QString csvPath = filePath;
    if (!filePath.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) {
        QString error;
        if (!convertSpreadsheetToCsv(filePath, &csvPath, &error)) {
            result.error = QStringLiteral("%1\nSee import log: %2").arg(error, importLogPath());
            appendImportLog(QStringLiteral("convertSpreadsheetToCsv failed: %1").arg(error));
            return result;
        }
    }

    result = parseLichuangCsv(csvPath, projectName);
    if (!result.ok) {
        appendImportLog(QStringLiteral("parseLichuangCsv failed: %1").arg(result.error));
        result.error = QStringLiteral("%1\nSee import log: %2").arg(result.error, importLogPath());
    }
    return result;
}

ImportResult ImportService::parseLichuangCsv(const QString &csvPath, const QString &projectName) const
{
    ImportResult result;

    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.error = QStringLiteral("Cannot open CSV file: %1").arg(csvPath);
        return result;
    }

    const QByteArray raw = file.readAll();
    if (raw.isEmpty()) {
        result.error = QStringLiteral("CSV file is empty: %1").arg(csvPath);
        return result;
    }

    const auto normalized = [](QString text) {
        return text.remove(' ').remove('\t').remove('\r').remove('\n').trimmed();
    };
    const auto containsAny = [](const QString &text, const QStringList &keys) {
        for (const QString &key : keys) {
            if (!key.isEmpty() && text.contains(key, Qt::CaseInsensitive)) {
                return true;
            }
        }
        return false;
    };
    const auto parseByText = [this](const QString &text) {
        QList<QStringList> rows;
        const QStringList lines = text.split(QRegularExpression(QStringLiteral("\r?\n")));
        rows.reserve(lines.size());
        for (const QString &line : lines) {
            rows.append(parseCsvLine(line));
        }
        return rows;
    };

    const QString textUtf8 = QString::fromUtf8(raw);
    const QString textLocal = QString::fromLocal8Bit(raw);

    QList<QStringList> lines = parseByText(textUtf8);
    if (lines.size() < 2) {
        lines = parseByText(textLocal);
    }

    const QStringList itemCodeKeys = {
        QStringLiteral("\u5546\u54c1\u7f16\u53f7"), QStringLiteral("\u6599\u53f7"),
        QStringLiteral("item"), QStringLiteral("part"), QStringLiteral("lcsc")
    };
    const QStringList brandKeys = {
        QStringLiteral("\u54c1\u724c"), QStringLiteral("brand")
    };
    const QStringList modelKeys = {
        QStringLiteral("\u5382\u5bb6\u578b\u53f7"), QStringLiteral("\u578b\u53f7"),
        QStringLiteral("mpn"), QStringLiteral("manufacturer")
    };
    const QStringList packageKeys = {
        QStringLiteral("\u5c01\u88c5"), QStringLiteral("package")
    };
    const QStringList nameKeys = {
        QStringLiteral("\u5546\u54c1\u540d\u79f0"), QStringLiteral("\u63cf\u8ff0"),
        QStringLiteral("name"), QStringLiteral("description")
    };
    const QStringList qtyKeys = {
        QStringLiteral("\u8ba2\u8d2d\u6570\u91cf"), QStringLiteral("\u6570\u91cf"),
        QStringLiteral("qty"), QStringLiteral("quantity")
    };
    const QStringList unitPriceKeys = {
        QStringLiteral("\u5546\u54c1\u5355\u4ef7"), QStringLiteral("\u5355\u4ef7"),
        QStringLiteral("unit"), QStringLiteral("price")
    };
    const QStringList amountKeys = {
        QStringLiteral("\u5546\u54c1\u91d1\u989d"), QStringLiteral("\u91d1\u989d"),
        QStringLiteral("amount"), QStringLiteral("total")
    };

    int headerRow = -1;
    for (int r = 0; r < lines.size(); ++r) {
        const QStringList row = lines[r];
        const auto at = [&](int index) { return index >= 0 && index < row.size() ? normalized(row[index]) : QString(); };
        const QString merged = normalized(row.join(QString()));

        const bool headerByMerged = containsAny(merged, itemCodeKeys)
            && containsAny(merged, modelKeys)
            && containsAny(merged, qtyKeys)
            && containsAny(merged, amountKeys);

        const bool headerByKnownColumns = containsAny(at(1), itemCodeKeys)
            && containsAny(at(3), modelKeys)
            && containsAny(at(6), qtyKeys)
            && containsAny(at(10), amountKeys);

        if (headerByMerged || headerByKnownColumns) {
            headerRow = r;
            break;
        }
    }

    if (headerRow < 0) {
        result.error = QStringLiteral("Cannot detect LCSC header row (need item/model/qty/amount columns).");
        return result;
    }

    const QStringList headerCells = lines.value(headerRow);
    auto findColumn = [&](const QStringList &keys, int fallbackIndex) {
        for (int i = 0; i < headerCells.size(); ++i) {
            if (containsAny(normalized(headerCells[i]), keys)) {
                return i;
            }
        }
        return fallbackIndex;
    };

    const int colItemCode = findColumn(itemCodeKeys, 1);
    const int colBrand = findColumn(brandKeys, 2);
    const int colModel = findColumn(modelKeys, 3);
    const int colPackage = findColumn(packageKeys, 4);
    const int colName = findColumn(nameKeys, 5);
    const int colQty = findColumn(qtyKeys, 6);
    const int colUnitPrice = findColumn(unitPriceKeys, 9);
    const int colAmount = findColumn(amountKeys, 10);

    QList<QStringList> rows;
    for (int r = headerRow + 1; r < lines.size(); ++r) {
        const QStringList row = lines[r];
        const auto at = [&](int i) { return i >= 0 && i < row.size() ? row[i].trimmed() : QString(); };

        const QString itemCode = at(colItemCode);
        const QString brand = at(colBrand);
        const QString model = at(colModel);
        const QString pkg = at(colPackage);
        const QString name = at(colName);
        const QString qty = at(colQty);
        const QString unitPrice = at(colUnitPrice);
        const QString amount = at(colAmount);

        if (itemCode.isEmpty() && brand.isEmpty() && model.isEmpty() && pkg.isEmpty()
            && name.isEmpty() && qty.isEmpty() && unitPrice.isEmpty() && amount.isEmpty()) {
            continue;
        }

        rows.append({projectName, itemCode, brand, model, pkg, name, qty, unitPrice, amount});
    }

    if (rows.isEmpty()) {
        result.error = QStringLiteral("No valid BOM rows found after the detected header row.");
        return result;
    }

    result.ok = true;
    result.headers = {QStringLiteral("\u9879\u76ee"),
                      QStringLiteral("\u5546\u54c1\u7f16\u53f7"),
                      QStringLiteral("\u54c1\u724c"),
                      QStringLiteral("\u5382\u5bb6\u578b\u53f7"),
                      QStringLiteral("\u5c01\u88c5"),
                      QStringLiteral("\u5546\u54c1\u540d\u79f0"),
                      QStringLiteral("\u8ba2\u8d2d\u6570\u91cf\uff08\u4fee\u6539\u540e\uff09"),
                      QStringLiteral("\u5546\u54c1\u5355\u4ef7"),
                      QStringLiteral("\u5546\u54c1\u91d1\u989d")};
    result.rows = rows;
    return result;
}

QStringList ImportService::parseCsvLine(const QString &line) const
{
    QStringList result;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line[i];
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                current.append('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            result.append(current);
            current.clear();
        } else {
            current.append(ch);
        }
    }

    result.append(current);
    return result;
}

bool ImportService::convertSpreadsheetToCsv(const QString &inputPath, QString *outputCsvPath, QString *error) const
{
    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempDir.isEmpty()) {
        if (error) {
            *error = QStringLiteral("Cannot resolve temporary directory.");
        }
        return false;
    }

    const QFileInfo info(inputPath);
    const QString outPath = QDir(tempDir).filePath(QStringLiteral("%1_link2bom.csv").arg(info.completeBaseName()));

    QString pythonError;
    if (info.suffix().compare(QStringLiteral("xlsx"), Qt::CaseInsensitive) == 0
        || info.suffix().compare(QStringLiteral("xls"), Qt::CaseInsensitive) == 0) {
        if (convertExcelToCsvWithPython(inputPath, outPath, &pythonError) && QFile::exists(outPath)) {
            if (outputCsvPath) {
                *outputCsvPath = outPath;
            }
            return true;
        }
        appendImportLog(QStringLiteral("Python conversion failed: %1").arg(pythonError));
    }

    auto runConverter = [&](const QString &program, const QStringList &args) -> bool {
        QProcess process;
        process.start(program, args);
        if (!process.waitForStarted(3000)) {
            appendImportLog(QStringLiteral("Converter start failed: %1 %2").arg(program, args.join(' ')));
            return false;
        }
        process.waitForFinished(40000);
        const bool ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
        if (!ok) {
            appendImportLog(QStringLiteral("Converter failed: %1 exit=%2 stderr=%3")
                            .arg(program)
                            .arg(process.exitCode())
                            .arg(QString::fromUtf8(process.readAllStandardError()).trimmed()));
        }
        return ok;
    };

    const QStringList officeCandidates {QStringLiteral("libreoffice"), QStringLiteral("soffice")};
    for (const QString &program : officeCandidates) {
        const bool ok = runConverter(program,
                                     {QStringLiteral("--headless"),
                                      QStringLiteral("--convert-to"),
                                      QStringLiteral("csv:Text - txt - csv (StarCalc):44,34,76,1"),
                                      QStringLiteral("--outdir"),
                                      QFileInfo(outPath).absolutePath(),
                                      inputPath});
        if (ok) {
            const QString converted = QDir(QFileInfo(outPath).absolutePath()).filePath(QStringLiteral("%1.csv").arg(info.completeBaseName()));
            if (QFile::exists(converted)) {
                if (outputCsvPath) {
                    *outputCsvPath = converted;
                }
                return true;
            }
        }
    }

    if (runConverter(QStringLiteral("ssconvert"), {inputPath, outPath}) && QFile::exists(outPath)) {
        if (outputCsvPath) {
            *outputCsvPath = outPath;
        }
        return true;
    }

    if (error) {
        const QString fallback = QStringLiteral(
            "Import failed. No available converter worked (python/libreoffice/soffice/ssconvert).\n"
            "For .xls, ensure Python package 'xlrd' is installed, or convert the file to .xlsx/.csv first.\n"
            "Input file: %1").arg(inputPath);
        *error = pythonError.isEmpty() ? fallback : QStringLiteral("%1\n%2").arg(fallback, pythonError);
    }
    return false;
}

bool ImportService::convertXlsxToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const
{
    const QString pythonCode = QStringLiteral(R"PY(
import csv
import sys
import zipfile
import xml.etree.ElementTree as ET

in_path, out_path = sys.argv[1], sys.argv[2]
ns = {'m': 'http://schemas.openxmlformats.org/spreadsheetml/2006/main'}

with zipfile.ZipFile(in_path, 'r') as zf:
    shared = []
    if 'xl/sharedStrings.xml' in zf.namelist():
        root = ET.fromstring(zf.read('xl/sharedStrings.xml'))
        for si in root.findall('m:si', ns):
            text = ''.join(t.text or '' for t in si.findall('.//m:t', ns))
            shared.append(text)

    sheet_name = 'xl/worksheets/sheet1.xml'
    if sheet_name not in zf.namelist():
        sheets = [n for n in zf.namelist() if n.startswith('xl/worksheets/sheet') and n.endswith('.xml')]
        if not sheets:
            raise RuntimeError('No worksheet found in xlsx file.')
        sheet_name = sorted(sheets)[0]

    root = ET.fromstring(zf.read(sheet_name))
    rows = []
    for row in root.findall('.//m:sheetData/m:row', ns):
        cells = {}
        max_col = -1
        for c in row.findall('m:c', ns):
            ref = c.attrib.get('r', '')
            letters = ''.join(ch for ch in ref if ch.isalpha())
            col = 0
            for ch in letters:
                col = col * 26 + (ord(ch.upper()) - 64)
            col = max(col - 1, 0)
            max_col = max(max_col, col)

            t = c.attrib.get('t', '')
            v = c.find('m:v', ns)
            val = ''
            if t == 'inlineStr':
                it = c.find('m:is/m:t', ns)
                if it is not None and it.text:
                    val = it.text
            elif t == 's' and v is not None and v.text and v.text.isdigit():
                idx = int(v.text)
                if 0 <= idx < len(shared):
                    val = shared[idx]
            elif v is not None and v.text:
                val = v.text
            cells[col] = val

        if max_col >= 0:
            line = [''] * (max_col + 1)
            for k, v in cells.items():
                line[k] = v
            rows.append(line)

with open(out_path, 'w', encoding='utf-8', newline='') as fp:
    csv.writer(fp).writerows(rows)
)PY");

    QString stdErr;
    QString launchError;
    if (!runPythonInline(pythonCode, {inputPath, outputPath}, &stdErr, &launchError)) {
        if (error) {
            *error = QStringLiteral("Python is not available; cannot parse .xlsx in built-in mode. %1").arg(launchError);
        }
        appendImportLog(QStringLiteral("convertXlsxToCsvWithPython launch/run failed: %1, stderr=%2").arg(launchError, stdErr));
        return false;
    }

    const bool ok = QFile::exists(outputPath);
    if (!ok && error) {
        *error = stdErr.isEmpty()
            ? QStringLiteral("Built-in .xlsx parsing failed.")
            : QStringLiteral("Built-in .xlsx parsing failed: %1").arg(stdErr);
    }
    if (!ok) {
        appendImportLog(QStringLiteral("convertXlsxToCsvWithPython output file missing: %1").arg(outputPath));
    }
    return ok;
}

bool ImportService::convertExcelToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const
{
    if (inputPath.endsWith(QStringLiteral(".xlsx"), Qt::CaseInsensitive)) {
        return convertXlsxToCsvWithPython(inputPath, outputPath, error);
    }

    const QString pythonCode = QStringLiteral(R"PY(
import csv
import sys

in_path, out_path = sys.argv[1], sys.argv[2]

try:
    import xlrd
except Exception as exc:
    raise RuntimeError(f"Missing 'xlrd' dependency for .xls parsing: {exc}")

book = xlrd.open_workbook(in_path)
if book.nsheets <= 0:
    raise RuntimeError('No worksheet found in xls file.')

sheet = book.sheet_by_index(0)
rows = []
for r in range(sheet.nrows):
    line = []
    for c in range(sheet.ncols):
        cell = sheet.cell_value(r, c)
        if isinstance(cell, float) and cell.is_integer():
            line.append(str(int(cell)))
        else:
            line.append(str(cell))
    rows.append(line)

with open(out_path, 'w', encoding='utf-8', newline='') as fp:
    csv.writer(fp).writerows(rows)
)PY");

    QString stdErr;
    QString launchError;
    if (!runPythonInline(pythonCode, {inputPath, outputPath}, &stdErr, &launchError)) {
        if (error) {
            *error = QStringLiteral("Python is not available; cannot parse .xls. %1").arg(launchError);
        }
        appendImportLog(QStringLiteral("convertExcelToCsvWithPython launch/run failed: %1, stderr=%2").arg(launchError, stdErr));
        return false;
    }

    const bool ok = QFile::exists(outputPath);
    if (!ok && error) {
        const QString stderrMsg = stdErr;
        if (stderrMsg.contains(QStringLiteral("xlrd"), Qt::CaseInsensitive)) {
            *error = QStringLiteral(".xls import failed: missing Python package 'xlrd'. Run `python -m pip install xlrd`, or convert to .xlsx/.csv first.\n%1")
                .arg(stderrMsg);
        } else {
            *error = stderrMsg.isEmpty()
                ? QStringLiteral(".xls parsing failed.")
                : QStringLiteral(".xls parsing failed: %1").arg(stderrMsg);
        }
    }
    if (!ok) {
        appendImportLog(QStringLiteral("convertExcelToCsvWithPython output file missing: %1").arg(outputPath));
    }
    return ok;
}
