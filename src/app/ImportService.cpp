#include "ImportService.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QStringConverter>
#include <QTextStream>

ImportService::ImportService(QObject *parent)
    : QObject(parent)
{
}

ImportResult ImportService::importLichuangSpreadsheet(const QString &filePath, const QString &projectName) const
{
    ImportResult result;
    if (filePath.isEmpty()) {
        result.error = QStringLiteral("文件路径为空。");
        return result;
    }

    QString csvPath = filePath;
    if (!filePath.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) {
        QString error;
        if (!convertSpreadsheetToCsv(filePath, &csvPath, &error)) {
            result.error = error;
            return result;
        }
    }

    result = parseLichuangCsv(csvPath, projectName);
    return result;
}

ImportResult ImportService::parseLichuangCsv(const QString &csvPath, const QString &projectName) const
{
    ImportResult result;
    QFile file(csvPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.error = QStringLiteral("无法打开 CSV 文件：%1").arg(csvPath);
        return result;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QList<QStringList> lines;
    while (!in.atEnd()) {
        lines.append(parseCsvLine(in.readLine()));
    }

    const auto normalized = [](QString text) {
        return text.remove(' ').remove('\t').trimmed();
    };

    int headerRow = -1;
    for (int r = 0; r < lines.size(); ++r) {
        const QString merged = normalized(lines[r].join(QString()));
        if (merged.contains(QStringLiteral("商品编号"))
            && merged.contains(QStringLiteral("厂家型号"))
            && merged.contains(QStringLiteral("订购数量（修改后）"))
            && merged.contains(QStringLiteral("商品金额"))) {
            headerRow = r;
            break;
        }
    }

    if (headerRow < 0) {
        result.error = QStringLiteral("未识别到立创表头（应包含第18行字段）。");
        return result;
    }

    QList<QStringList> rows;
    for (int r = headerRow + 1; r < lines.size(); ++r) {
        const QStringList row = lines[r];
        const auto at = [&](int i) { return i < row.size() ? row[i].trimmed() : QString(); };

        const QString itemCode = at(1);
        const QString brand = at(2);
        const QString model = at(3);
        const QString pkg = at(4);
        const QString name = at(5);
        const QString qty = at(6);
        const QString unitPrice = at(9);
        const QString amount = at(10);

        if (itemCode.isEmpty() && brand.isEmpty() && model.isEmpty() && pkg.isEmpty() && name.isEmpty() && qty.isEmpty() && unitPrice.isEmpty() && amount.isEmpty()) {
            continue;
        }

        rows.append({projectName, itemCode, brand, model, pkg, name, qty, unitPrice, amount});
    }

    if (rows.isEmpty()) {
        result.error = QStringLiteral("立创导入未找到有效数据（应从第19行开始）。");
        return result;
    }

    result.ok = true;
    result.headers = {QStringLiteral("项目"),
                      QStringLiteral("商品编号"),
                      QStringLiteral("品牌"),
                      QStringLiteral("厂家型号"),
                      QStringLiteral("封装"),
                      QStringLiteral("商品名称"),
                      QStringLiteral("订购数量（修改后）"),
                      QStringLiteral("商品单价"),
                      QStringLiteral("商品金额")};
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
            *error = QStringLiteral("无法获取临时目录。");
        }
        return false;
    }

    const QFileInfo info(inputPath);
    const QString outPath = QDir(tempDir).filePath(QStringLiteral("%1_starbom.csv").arg(info.completeBaseName()));

    if (info.suffix().compare(QStringLiteral("xlsx"), Qt::CaseInsensitive) == 0
        || info.suffix().compare(QStringLiteral("xls"), Qt::CaseInsensitive) == 0) {
        QString pyError;
        if (convertExcelToCsvWithPython(inputPath, outPath, &pyError) && QFile::exists(outPath)) {
            if (outputCsvPath) {
                *outputCsvPath = outPath;
            }
            return true;
        }
        if (error && !pyError.isEmpty()) {
            *error = pyError;
        }
    }

    auto runConverter = [&](const QString &program, const QStringList &args) -> bool {
        QProcess process;
        process.start(program, args);
        if (!process.waitForStarted(3000)) {
            return false;
        }
        process.waitForFinished(20000);
        return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
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
        *error = QStringLiteral("导入失败：未检测到可用转换器（libreoffice/soffice/ssconvert），且内置 Excel 解析不可用。\n"
                                "建议：安装 libreoffice（含 soffice 命令）或 ssconvert，或先另存为 CSV。\n文件：%1")
                     .arg(inputPath);
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
            raise RuntimeError('xlsx 中未找到工作表')
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

    QProcess process;
    process.start(QStringLiteral("python3"), {QStringLiteral("-c"), pythonCode, inputPath, outputPath});
    if (!process.waitForStarted(3000)) {
        if (error) {
            *error = QStringLiteral("未找到 python3，无法使用内置 xlsx 解析。");
        }
        return false;
    }

    process.waitForFinished(20000);
    const bool ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && QFile::exists(outputPath);
    if (!ok && error) {
        const QString stderrMsg = QString::fromUtf8(process.readAllStandardError()).trimmed();
        *error = stderrMsg.isEmpty() ? QStringLiteral("内置 xlsx 解析失败。") : QStringLiteral("内置 xlsx 解析失败：%1").arg(stderrMsg);
    }
    return ok;
}

bool ImportService::convertExcelToCsvWithPython(const QString &inputPath, const QString &outputPath, QString *error) const
{
    if (inputPath.endsWith(QStringLiteral(".xlsx"), Qt::CaseInsensitive)) {
        return convertXlsxToCsvWithPython(inputPath, outputPath, error);
    }

    QString program = QStringLiteral("python3");
    QProcess check;
    check.start(program, {QStringLiteral("--version")});
    if (!check.waitForStarted(2500)) {
        program = QStringLiteral("python");
    } else {
        check.waitForFinished(2500);
    }

    const QString pythonCode = QStringLiteral(R"PY(
import csv
import sys

in_path, out_path = sys.argv[1], sys.argv[2]

try:
    import xlrd
except Exception as exc:
    raise RuntimeError(f'缺少 xlrd 依赖，无法读取 .xls 文件: {exc}')

book = xlrd.open_workbook(in_path)
if book.nsheets <= 0:
    raise RuntimeError('xls 中未找到工作表')

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

    QProcess process;
    process.start(program, {QStringLiteral("-c"), pythonCode, inputPath, outputPath});
    if (!process.waitForStarted(3000)) {
        if (error) {
            *error = QStringLiteral("未找到 python3/python，无法执行 .xls 解析。");
        }
        return false;
    }

    process.waitForFinished(20000);
    const bool ok = process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0 && QFile::exists(outputPath);
    if (!ok && error) {
        const QString stderrMsg = QString::fromUtf8(process.readAllStandardError()).trimmed();
        if (stderrMsg.contains(QStringLiteral("xlrd"), Qt::CaseInsensitive)) {
            *error = QStringLiteral(".xls 导入失败：缺少 Python 包 xlrd。请执行 `python3 -m pip install xlrd` 后重试，或将文件另存为 .xlsx/.csv。\n%1")
                         .arg(stderrMsg);
        } else {
            *error = stderrMsg.isEmpty() ? QStringLiteral(".xls 解析失败。") : QStringLiteral(".xls 解析失败：%1").arg(stderrMsg);
        }
    }
    return ok;
}
