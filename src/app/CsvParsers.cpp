#include "CsvParsers.h"

#include <QFile>
#include <QRegularExpression>

#include <algorithm>

namespace {
QStringList parseCsvLine(const QString &line)
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
}

ImportResult LichuangCsvParser::parseFile(const QString &csvPath, const QString &projectName) const
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
    const auto parseByText = [](const QString &text) {
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

ImportResult GenericCsvParser::parseFile(const QString &csvPath, const QString &projectName) const
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

    const auto parseByText = [](const QString &text) {
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

    QList<QStringList> linesUtf8 = parseByText(textUtf8);
    QList<QStringList> linesLocal = parseByText(textLocal);
    QList<QStringList> lines = linesUtf8;

    auto findHeaderRow = [](const QList<QStringList> &rows) {
        for (int r = 0; r < rows.size(); ++r) {
            const QStringList row = rows[r];
            const bool hasData = std::any_of(row.begin(), row.end(), [](const QString &cell) {
                return !cell.trimmed().isEmpty();
            });
            if (hasData) {
                return r;
            }
        }
        return -1;
    };

    int headerRow = findHeaderRow(lines);
    if (headerRow < 0 && !linesLocal.isEmpty()) {
        lines = linesLocal;
        headerRow = findHeaderRow(lines);
    }

    if (headerRow < 0) {
        result.error = QStringLiteral("Cannot detect header row in CSV file.");
        return result;
    }

    auto normalize = [](const QString &text) {
        return QString(text).remove(' ').remove('\t').remove('\r').remove('\n').trimmed().toLower();
    };

    const QStringList projectKeys = {QStringLiteral("项目"), QStringLiteral("project")};
    auto detectProjectIndex = [&](const QStringList &headers) {
        for (int i = 0; i < headers.size(); ++i) {
            const QString cell = normalize(headers[i]);
            for (const QString &key : projectKeys) {
                if (cell.contains(key)) {
                    return i;
                }
            }
        }
        return -1;
    };

    QStringList headers = lines.value(headerRow);
    int projectIndex = detectProjectIndex(headers);

    if (projectIndex < 0 && !linesLocal.isEmpty()) {
        const int localHeaderRow = findHeaderRow(linesLocal);
        if (localHeaderRow >= 0) {
            const int localProjectIndex = detectProjectIndex(linesLocal.value(localHeaderRow));
            if (localProjectIndex >= 0) {
                lines = linesLocal;
                headerRow = localHeaderRow;
                headers = lines.value(headerRow);
                projectIndex = localProjectIndex;
            }
        }
    }

    int maxCols = headers.size();
    for (int r = headerRow + 1; r < lines.size(); ++r) {
        const QStringList row = lines[r];
        const bool hasData = std::any_of(row.begin(), row.end(), [](const QString &cell) {
            return !cell.trimmed().isEmpty();
        });
        if (!hasData) {
            continue;
        }
        maxCols = std::max(maxCols, static_cast<int>(row.size()));
    }

    if (projectIndex < 0) {
        headers.prepend(QStringLiteral("项目"));
        projectIndex = 0;
        maxCols += 1;
    }

    if (headers.size() < maxCols) {
        for (int i = headers.size(); i < maxCols; ++i) {
            headers.append(QStringLiteral("列%1").arg(i + 1));
        }
    }

    for (int i = 0; i < headers.size(); ++i) {
        if (headers[i].trimmed().isEmpty()) {
            headers[i] = QStringLiteral("列%1").arg(i + 1);
        }
    }

    QList<QStringList> rows;
    for (int r = headerRow + 1; r < lines.size(); ++r) {
        QStringList row = lines[r];
        const bool hasData = std::any_of(row.begin(), row.end(), [](const QString &cell) {
            return !cell.trimmed().isEmpty();
        });
        if (!hasData) {
            continue;
        }

        if (projectIndex == 0 && row.size() < headers.size()) {
            row.prepend(projectName);
        }

        if (projectIndex >= 0 && projectIndex < row.size() && row[projectIndex].trimmed().isEmpty()) {
            row[projectIndex] = projectName;
        }

        if (row.size() < headers.size()) {
            row.reserve(headers.size());
            while (row.size() < headers.size()) {
                row.append(QString());
            }
        } else if (row.size() > headers.size()) {
            while (headers.size() < row.size()) {
                headers.append(QStringLiteral("列%1").arg(headers.size() + 1));
            }
        }

        rows.append(row);
    }

    if (rows.isEmpty()) {
        result.error = QStringLiteral("No valid rows found after the header.");
        return result;
    }

    result.ok = true;
    result.headers = headers;
    result.rows = rows;
    return result;
}
