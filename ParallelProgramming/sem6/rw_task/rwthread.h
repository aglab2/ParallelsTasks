#ifndef RWTHREAD_H
#define RWTHREAD_H

#include <QReadWriteLock>
#include <QThread>
#include <iostream>

using namespace std;

class RThread : public QThread
{
Q_OBJECT
public:
    RThread(QReadWriteLock *lock, int *data);
protected:
    void run();
private:
    QReadWriteLock *lock; int *data;
};


class WThread : public QThread
{
Q_OBJECT
public:
    WThread(QReadWriteLock *lock, int *data);
protected:
    void run();
private:
    QReadWriteLock *lock; int *data;
};


#endif // RWTHREAD_H
