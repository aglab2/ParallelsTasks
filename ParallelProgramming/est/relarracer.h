#ifndef RELARRACER_H
#define RELARRACER_H

#include <QObject>
#include <QThread>
#include <QSemaphore>

class RelayRacer : public QThread
{
Q_OBJECT
public:
    RelayRacer(QString name);
protected:
    void run();

public slots:
    void startJob(int job);
signals:
    void finishJob(int job);
private:
    QSemaphore jobSignal;
    int job;
    QString name;
};

#endif // RELARRACER_H
