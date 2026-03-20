#pragma once

#include <QObject>
#include <QString>

class StatusHub : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)

public:
    explicit StatusHub(QObject *parent = nullptr);

    QString status() const;
    void setStatus(const QString &status);

signals:
    void statusChanged();

private:
    QString m_status;
};
