#pragma once

#include <QList>
#include <QStringList>

struct ImportResult {
    bool ok = false;
    QString error;
    QStringList headers;
    QList<QStringList> rows;
};
