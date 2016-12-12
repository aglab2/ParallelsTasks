#include <QCoreApplication>

#include "estthread.h"
#include "relayracecoordinator.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Counter a, b;
    QObject::connect(&a, SIGNAL(valueChanged(int)),
            &b, SLOT(setValue(int)));
    a.setValue(12);   // a.value() == 12, b.value() == 12
    b.setValue(48);   // a.value() == 12, b.value() == 48

    cout << a.value() << b.value() << endl;

    RelayRaceCoordinator rrc;
    rrc.addRacer("Петя");
    rrc.addRacer("Вася");
    rrc.addRacer("Коля");
    rrc.addRacer("Рома");
    rrc.addRacer("Толя");

    rrc.startRace();

    cout << "Fini :]" << endl;

    return app.exec();
}
