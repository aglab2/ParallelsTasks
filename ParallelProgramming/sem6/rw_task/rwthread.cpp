#include "rwthread.h"

RThread::RThread(QReadWriteLock *lock, int *data) : lock(lock),data(data){}
void RThread::run()
{
    lock->lockForRead();
    cout << *data << endl;
    lock->unlock();
}

WThread::WThread(QReadWriteLock *lock, int *data) : lock(lock),data(data){}
void WThread::run()
{
    lock->lockForWrite();
    *data = 123;
    lock->unlock();
}
