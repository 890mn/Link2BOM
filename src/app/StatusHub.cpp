#include "StatusHub.h"
#include "AppLogger.h"

StatusHub::StatusHub(QObject *parent)
    : QObject(parent)
{
}

QString StatusHub::status() const
{
    return m_status;
}

void StatusHub::setStatus(const QString &status)
{
    if (status == m_status) {
        return;
    }
    m_status = status;

    const QString lower = status.toLower();
    if (lower.contains(QStringLiteral("failed")) || lower.contains(QStringLiteral("error"))) {
        AppLogger::error(QStringLiteral("Status: %1").arg(status));
    } else if (lower.contains(QStringLiteral("warning")) || lower.contains(QStringLiteral("please"))) {
        AppLogger::warn(QStringLiteral("Status: %1").arg(status));
    } else {
        AppLogger::info(QStringLiteral("Status: %1").arg(status));
    }

    emit statusChanged();
}
