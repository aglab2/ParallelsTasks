#include <QCoreApplication>
#include "rwthread.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QReadWriteLock lock;
    int data = 356;

    RThread *r1 = new RThread(&lock, &data);
    RThread *r2 = new RThread(&lock, &data);
    RThread *r3 = new RThread(&lock, &data);
    RThread *r4 = new RThread(&lock, &data);
    WThread *w = new WThread(&lock, &data);

    r1->start();
    r2->start();
    r3->start();
    r4->start();
    w->start();

    return a.exec();
}
