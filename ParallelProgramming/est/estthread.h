#ifndef ESTTHREAD_H
#define ESTTHREAD_H

#include <QThread>
#include <QObject>

#include <iostream>

using namespace std;
class Counter : public QObject
{
   Q_OBJECT
public:
   Counter() { m_value = 0; }
   int value() const { return m_value; }
public slots:
   void setValue(int value);
signals:
   void valueChanged(int newValue);
private:
   int m_value;
};


#endif // ESTTHREAD_H
