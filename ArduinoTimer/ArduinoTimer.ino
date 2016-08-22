#include "Timer.h"


Timer t;

int countEvent;
int i;
bool isitthetime;

void setup()
{
  Serial.begin(9600);
  isitthetime = false;
  i = 0;

  int afterEvent = t.every(10000, doSomething2);
  Serial.print("After event started id=");
  Serial.println(afterEvent);

}


void loop()
{
  if (isitthetime)
  {
    doSomething3();
  } else {
    Serial.println("gagal");
  }
  t.update();
}


void doSomething()
{
  Serial.print("2 second tick: millis()=");
  Serial.println(millis());
}

void doSomething2()
{
  Serial.print("It's time. Start Counting ...");
  isitthetime = !isitthetime;
}

void doSomething3()
{
  if (i == 0);
  {
    countEvent = t.every(1000, doSomething4);
  }
}

void doSomething4()
{
  i++;
  Serial.println(i);
  if (i == 10)
  {
    i = 0;
    t.stop(countEvent);
  }
}

/*void doAfter()
{

}*/
