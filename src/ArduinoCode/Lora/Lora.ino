/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses ABP (Activation-by-personalisation), where a DevAddr and
 * Session keys are preconfigured (unlike OTAA, where a DevEUI and
 * application key is configured, while the DevAddr and session keys are
 * assigned/generated in the over-the-air-activation procedure).
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!
 *
 * To use this sketch, first register your application and device with
 * the things network, to set or generate a DevAddr, NwkSKey and
 * AppSKey. Each device should have their own unique values for these
 * fields.
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Sleep_n0m.h>
#include <EEPROM.h>
#include <FreqMeasure.h>

#define intPin 2      //WakeUp pin
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

// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the early prototype TTN
// network.
static const PROGMEM u1_t NWKSKEY[16] = { 0xBD, 0xCC, 0xED, 0x07, 0xD2, 0x7F, 0x82, 0x1A, 0x77, 0x19, 0xFA, 0xE4, 0x5F, 0x8E, 0x89, 0x5E };

// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the early prototype TTN
// network.
static const u1_t PROGMEM APPSKEY[16] = { 0x30, 0xD2, 0x05, 0x1F, 0x41, 0x09, 0x87, 0x4D, 0x92, 0x85, 0x70, 0x77, 0xD5, 0xE9, 0x6A, 0xAA };

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x26011261; // <-- Change this address for every node!

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

//The data which will be send using LoRaWAN
static uint8_t mydata[] = "0 0";


static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 120;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,
    .dio = {4, 5, 7},
};

/*
 * We don't need every case in the following Method, but since it is only 
 * commented, it will not be tranferred to the arduino.
 */
void onEvent (ev_t ev) {

    switch(ev) {
       /* case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
            */
        case EV_JOINED:
           // Serial.println(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
       /* case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
            */
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
           //   Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
                Serial.println(F("Received "));
           //   Serial.println(LMIC.dataLen);
            //  Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
            /*
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
            */
         default:
          //  Serial.println(F("Unknown event"));
            break;
    }
}

/*
 * This is the send method. Here we set the data to be the number of walker and cyclists.
 * After it sent the data, it will reset the count of walker & cyclist to 0.
 */
void do_send(osjob_t* j){

    mydata[0] = walker;
    mydata[2] = cyclist;
    //END TESTING HEEEEEEERE!!!!

     if (LMIC.opmode & OP_TXRXPEND) {
        //Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
        walker, cyclist = 0;
    }
    
    

}

/*
 * CheckIObject will check if it is a Person or not. If the speed is high enough, it
 * will reset the sleeptimer.
 */
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

/*
 * This method can be removed if the product is going to be used for production.
 * This is only for outputting information about speed, count of people and the
 * state of the PIRs.
 * This method was also used for the Demonstation using ProcessingIDE.
 */
void OutputResults(float nspeed) {

  /*
   * Demonstration output (send data to ProcessingIDE)
  for (int i = 0; i < 5; i++) {
    Serial.println((String)nspeed + ';' + (String)walker + ';' + (String)cyclist);
  }
  */
  
  //Serial.print("Speed: ");
  //Serial.println(nspeed);
  //Serial.println(" km/h");
  //Serial.print("Walker: ");
  Serial.println(walker);
  //Serial.print("Cyclist: ");
  Serial.println(cyclist);
  //Serial.print("PIR: ");
  //Serial.println(digitalRead(intPin));
  Serial.println();
  
}

/*
 * This method will initiate the sleepmode. After the sleepPinInterrupt()
 * it will fall asleep and will only wake up after one of the PIRs detected movement
 * and send a high signal.
 */
void ActivateDeepSleep() {
    
    sleep.pwrDownMode();
    sleep.sleepPinInterrupt(intPin, HIGH);
    
    seconds = millis();     //reset seconds
}

/*
 * This method checks if the amount of time exceeds the specified 10 seconds. If this is the case,
 * it will trigger the ActivateDeepSleep() method. 
 * It is also checking every time if the amount of people passed by exceeds the specified number, 
 * in this case 20, to send the data using LoRaWAN.
 */
void CheckTimer() {

  long tmpSecs = millis();
  
  if (tmpSecs > (seconds + (10 * 1000))) {
    ActivateDeepSleep();
  }  
  else if (tmpSecs > (seconds + (0.5 * 1000)))
      samePerson = false;

  if (!samePerson && avgSpeed > 0)
      DeterminePerson();

  if ((walker + cyclist) >= 20) {
      // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    do_send(&sendjob);
  }
}

/*
 * This method will determine if the detected person is a pedestrian or a cyclist.
 * It will 'put' the counted people into the EEPROM to be stored even if the device is shut down.
 * After this is done, it will use the debugging method 'OutputResult' to print the collected data we got.
 */
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

/*
 * This is a standard method.
 * Here it initializes the objects and variables if its turned on.
 */
void setup() {
    Serial.begin(9600);
    FreqMeasure.begin();
  //  Serial.println(F("Starting"));
    pinMode(intPin, INPUT);

    sum, count, walker, cyclist, seconds, avgSpeed, countOfDetection = 0;
    detectionEnd, samePerson = false;

    seconds = millis();
    
    EEPROM.get(0, walker);
    EEPROM.get(1, cyclist);
     
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    #if defined(CFG_eu868)
    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set.
    // NA-US channels 0-71 are configured automatically
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    // TTN defines an additional channel at 869.525Mhz using SF9 for class B
    // devices' ping slots. LMIC does not have an easy way to define set this
    // frequency and support for class B is spotty and untested, so this
    // frequency is not configured here.
    #endif
}

/*
 * This is a standard method.
 * Here it loops every cycle the code.
 * We are checking if the radar is powered on and check if it detected speed using the different methods
 * above. 
 * Also the loop method is executing 'CheckTimer' every cycle to make sure how much time passed.
 */
void loop() {
    //os_runloop_once();
    
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


