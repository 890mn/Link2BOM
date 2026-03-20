#pragma once

#include <QHash>
#include <QString>
#include <QVariant>

class UiSettingsStore
{
public:
    QHash<QString, QVariantList> loadBomWidthRatios() const;
    void saveBomWidthRatios(const QHash<QString, QVariantList> &ratiosByLayout) const;
};
