#include "relarracer.h"

#include <iostream>
using namespace std;

RelayRacer::RelayRacer(QString name) : job(0), name(name)
{
}

void RelayRacer::run()
{
    cout << name.toStdString() << ": Waiting to start" << endl;
    jobSignal.acquire();
    cout << name.toStdString() << ": Working..." << endl;
    sleep(job);
    cout << name.toStdString() << ": Work ended" << endl;
    emit finishJob(job + qrand() % 2);
}

void RelayRacer::startJob(int job)
{
    this->job = job;
    jobSignal.release();
}
