#include "UiSettingsStore.h"

#include <QSettings>

QHash<QString, QVariantList> UiSettingsStore::loadBomWidthRatios() const
{
    QHash<QString, QVariantList> ratiosByLayout;

    QSettings settings;
    const QVariantMap savedMap = settings.value(QStringLiteral("bom/customWidthRatios")).toMap();
    for (auto it = savedMap.constBegin(); it != savedMap.constEnd(); ++it) {
        const QVariantList list = it.value().toList();
        if (!list.isEmpty()) {
            ratiosByLayout.insert(it.key(), list);
        }
    }
    return ratiosByLayout;
}

void UiSettingsStore::saveBomWidthRatios(const QHash<QString, QVariantList> &ratiosByLayout) const
{
    QVariantMap toSave;
    for (auto it = ratiosByLayout.constBegin(); it != ratiosByLayout.constEnd(); ++it) {
        toSave.insert(it.key(), it.value());
    }

    QSettings settings;
    settings.setValue(QStringLiteral("bom/customWidthRatios"), toSave);
}
