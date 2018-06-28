#include <Sleep_n0m.h>
#include <EEPROM.h>
#include <FreqMeasure.h>

#define intPin 2      //WakeUp pin
#define ANALOG_IN 0   //Radar pin
double sum;
int count;
int walker = 0;
int cyclist = 0;
boolean samePerson;
long seconds;
float avgSpeed;
int countOfDetection;
boolean detectionEnd;

Sleep sleep;

void setup() {
    Serial.begin(9600);
    FreqMeasure.begin();

    pinMode(intPin, INPUT);

    sum, count, walker, cyclist, seconds, avgSpeed, countOfDetection = 0;
    detectionEnd, samePerson = false;

    seconds = millis();

    //EEPROM.get(0, walker);
    //EEPROM.get(1, cyclist);
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

            //reset speed to 0
            speed = 0;
        }
    }
    
    CheckTimer();
}

void CheckObject(float speed) {

    if (speed >= 0.5f && samePerson) {
        seconds = millis();
        avgSpeed += speed;
        countOfDetection++;
    }
  
    if (!samePerson && avgSpeed > 0) {
        avgSpeed = avgSpeed / countOfDetection;
        
        if (avgSpeed > 1 && avgSpeed < 6) {
            walker++;
        }
        else if (avgSpeed >= 6) {
            cyclist++;
        }

        seconds = millis();

        OutputResults(avgSpeed);

        avgSpeed = 0;
        countOfDetection = 0;
    }
    
    if (!samePerson && speed > 0.5f) {
        samePerson = true;
        seconds = millis();
    }
}

void OutputResults(float nspeed) {

  /*
   * Demonstration output
  for (int i = 0; i < 5; i++) {
    Serial.println((String)nspeed + ';' + (String)walker + ';' + (String)cyclist);
  }
  */
  
  Serial.print("Speed: ");
  Serial.print(nspeed);
  Serial.println(" km/h");
  Serial.print("Walker: ");
  Serial.println(walker);
  Serial.print("Cyclist: ");
  Serial.println(cyclist);
  Serial.print("PIR: ");
  Serial.println(digitalRead(intPin));
  Serial.println();
  
}

void ActivateDeepSleep() {
    
    sleep.pwrDownMode();
    sleep.sleepPinInterrupt(intPin, HIGH);
    seconds = millis();
    Serial.println(digitalRead(intPin));
    
}

void CheckTimer() {

  long tmpSecs = millis();
  
  if (tmpSecs > (seconds + (10 * 1000))) {
    Serial.println("Goodnight sweet prince");
    Serial.println(seconds);
    delay(100);
    
    ActivateDeepSleep();
  
  }  
  else if (tmpSecs > (seconds + (0.5 * 1000)))
      samePerson = false;

  if (!samePerson && avgSpeed > 0)
      DeterminePerson();
}

void DeterminePerson() {
    avgSpeed = avgSpeed / countOfDetection;
        
    if (avgSpeed > 1 && avgSpeed < 10) {
        walker++;
    }
    else if (avgSpeed >= 10) {
        cyclist++;
    }

    OutputResults(avgSpeed);

    avgSpeed = 0;
    countOfDetection = 0;

    EEPROM.put(0, walker);
    EEPROM.put(1, cyclist);
}

void SendData() {
    //This is the method for sending all of the data

    //After sending, clear eeprom data
    EEPROM.put(0, 0);
    EEPROM.put(1, 0);
}

void GetBatteryStatus() {
    //This is the method for getting the battery status
}

