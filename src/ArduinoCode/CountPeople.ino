#include <EEPROM.h>
#include <FreqMeasure.h>

#define ANALOG_IN 0
double sum;
int count;
int walker;
int cyclist;
boolean samePerson;

void setup() {
    Serial.begin(9600);
    FreqMeasure.begin();

    sum, count, walker, cyclist = 0;

    
}

void loop() {
    //TODO: add timer for going to sleep
    //TODO: add timer for idle radar

    
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
  
}

