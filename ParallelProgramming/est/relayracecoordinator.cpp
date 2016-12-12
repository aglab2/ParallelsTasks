#include "relayracecoordinator.h"

RelayRaceCoordinator::RelayRaceCoordinator()
{
}

RelayRaceCoordinator::~RelayRaceCoordinator()
{
    for (int i = 0; i < racers.size(); ++i) {
        delete racers.at(i);
    }
}

void RelayRaceCoordinator::addRacer(QString name){
    RelayRacer *newRacer = new RelayRacer(name);
    if (!racers.isEmpty()) {
        RelayRacer *lastRacer = racers.back();
        connect(lastRacer, SIGNAL(finishJob(int)), newRacer, SLOT(startJob(int)), Qt::DirectConnection);
    }
    racers.append(newRacer);
}

void RelayRaceCoordinator::startRace(){
    if (racers.isEmpty()) return;
    for (int i = 0; i < racers.size(); ++i) {
        racers.at(i)->start();
    }
    racers.at(0)->startJob(1);
    racers.back()->wait();
}
