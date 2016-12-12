#ifndef RELAYRACECOORDINATOR_H
#define RELAYRACECOORDINATOR_H

#include <QObject>
#include <QSemaphore>

#include "relarracer.h"

class RelayRaceCoordinator : public QObject
{
Q_OBJECT
public:
    RelayRaceCoordinator();
    ~RelayRaceCoordinator();
    void addRacer(QString name);
    void startRace();

private:
    QList<RelayRacer *> racers;
};

#endif // RELAYRACECOORDINATOR_H
