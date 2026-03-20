#pragma once

#include <QString>

#include "ImportTypes.h"

class LichuangCsvParser
{
public:
    ImportResult parseFile(const QString &csvPath, const QString &projectName) const;
};

class GenericCsvParser
{
public:
    ImportResult parseFile(const QString &csvPath, const QString &projectName) const;
};
