#include <Sleep_n0m.h>
#include <EEPROM.h>
#include <FreqMeasure.h>

#define intPin 2      //WakeUp pin
#define ANALOG_IN 0   //Radar pin
double sum;
int count;
int walker;
int cyclist;
boolean samePerson;
long seconds;
float avgSpeed;
int countOfDetection;

Sleep sleep;

void setup() {
    Serial.begin(9600);
    FreqMeasure.begin();

    sum, count, walker, cyclist, seconds, avgSpeed, countOfDetection = 0;

    seconds = millis();
}

void loop() {

    if (FreqMeasure.available() > 0) 
    {
        // average several reading together
        sum = sum + FreqMeasure.read();
        count = count + 1;
        if (count > 30) 
        {
            float frequency = FreqMeasure.countToFrequency(sum / count);
            float speed = frequency / 19.49; //conversion from frequency to km/h

            sum = 0;
            count = 0;

            //Checking if a person is detected by the radar
            CheckObject(speed);
            
            //Output the result
            OutputResults(speed);

            //reset speed to 0
            speed = 0;
            
            EEPROM.put(0, walker);
            EEPROM.put(1, cyclist);
            
        }
    }
}

void CheckObject(float speed) {

    //bool test = false;

    CheckTimer();
    if (speed >= 0.5f) {
        seconds = millis();
       // avgSpeed += speed;
        //countOfDetection++;
    }
  /*
    if (speed >= 0.5f && !samePerson) {

        if (speed < 6)
            walker++;
        else if (speed >= 6)
            cyclist++;

        samePerson = true;
    }
    else if (speed < 0.5f) {
        samePerson = false;
    }
*/
    if (!samePerson) {
        avgSpeed = avgSpeed / countOfDetection;
      
        if (avgSpeed > 1 && avgSpeed < 6) {
            walker++;
        }
        else if (avgSpeed >= 6) {
            cyclist++;
        }
    }
}

void OutputResults(float nspeed) {
    Serial.print("Speed: ");
    Serial.println(nspeed);
    Serial.print("Walker: ");
    Serial.println(walker);
    Serial.print("Cyclist: ");
    Serial.println(cyclist);
    Serial.println();
}

void ActivateDeepSleep() {
    sleep.pwrDownMode();
    sleep.sleepPinInterrupt(intPin,HIGH);
}

void CheckTimer() {

  long tmpSecs = millis();
  

  if (tmpSecs > (seconds + (10 * 1000)))
      ActivateDeepSleep();
  else if (tmpSecs > (seconds + (0.5 * 1000)))
      samePerson = false;
}

