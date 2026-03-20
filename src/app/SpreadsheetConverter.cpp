#include "SpreadsheetConverter.h"
#include "AppLogger.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <optional>

namespace {
struct PythonCommand {
    QString program;
    QStringList prefixArgs;
};

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

bool SpreadsheetConverter::toCsv(const QString &inputPath, QString *outputCsvPath, QString *error) const
{
    if (inputPath.isEmpty()) {
        if (error) {
            *error = QStringLiteral("File path is empty.");
        }
        return false;
    }

    if (inputPath.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) {
        if (outputCsvPath) {
            *outputCsvPath = inputPath;
        }
        return true;
    }

    return convertSpreadsheetToCsv(inputPath, outputCsvPath, error);
}

bool SpreadsheetConverter::convertSpreadsheetToCsv(const QString &inputPath, QString *outputCsvPath, QString *error) const
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
        AppLogger::warn(QStringLiteral("Python conversion failed: %1").arg(pythonError));
    }

    auto runConverter = [&](const QString &program, const QStringList &args) -> bool {
        QProcess process;
        process.start(program, args);
        if (!process.waitForStarted(3000)) {
            AppLogger::warn(QStringLiteral("Converter start failed: %1 %2").arg(program, args.join(' ')));
            return false;
        }
        process.waitForFinished(40000);
        const bool ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
        if (!ok) {
            AppLogger::warn(QStringLiteral("Converter failed: %1 exit=%2 stderr=%3")
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

bool SpreadsheetConverter::convertXlsxToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const
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
        AppLogger::warn(QStringLiteral("convertXlsxToCsvWithPython launch/run failed: %1, stderr=%2")
                            .arg(launchError, stdErr));
        return false;
    }

    const bool ok = QFile::exists(outputPath);
    if (!ok && error) {
        *error = stdErr.isEmpty()
            ? QStringLiteral("Built-in .xlsx parsing failed.")
            : QStringLiteral("Built-in .xlsx parsing failed: %1").arg(stdErr);
    }
    if (!ok) {
        AppLogger::warn(QStringLiteral("convertXlsxToCsvWithPython output file missing: %1").arg(outputPath));
    }
    return ok;
}

bool SpreadsheetConverter::convertExcelToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const
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
        AppLogger::warn(QStringLiteral("convertExcelToCsvWithPython launch/run failed: %1, stderr=%2")
                            .arg(launchError, stdErr));
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
        AppLogger::warn(QStringLiteral("convertExcelToCsvWithPython output file missing: %1").arg(outputPath));
    }
    return ok;
}
