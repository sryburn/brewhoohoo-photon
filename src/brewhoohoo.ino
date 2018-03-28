#include "spark-dallas-temperature.h"
#include <OneWire.h>
#include "display.h"

SYSTEM_THREAD(ENABLED);

DallasTemperature dallas(new OneWire(A0));
Display display;

#define testing //uncomment for test board

#ifdef testing
    DeviceAddress mashSensor = {0x28, 0xFF, 0x5E, 0xB6, 0x0, 0x17, 0x4, 0x4};
    DeviceAddress coilSensor = {0x28, 0xFF, 0x63, 0xA8, 0x0, 0x17, 0x5, 0x1F};
    DeviceAddress hltSensor = {0x28, 0x5B, 0xC4, 0xAC, 0x9, 0x0, 0x0, 0xC0};
    DeviceAddress boilSensor = {0x28, 0xFF, 0xA8, 0xAF, 0x0, 0x17, 0x4, 0x95};
#else
    DeviceAddress mashSensor = {0x28, 0xB8, 0x4E, 0x74, 0x6, 0x0, 0x0, 0x84};
    DeviceAddress coilSensor = {0x28, 0xB7, 0x3A, 0x74, 0x6, 0x0, 0x0, 0xD2};
    DeviceAddress hltSensor = {0x28, 0xE7, 0xC, 0x74, 0x6, 0x0, 0x0, 0xE4};
    DeviceAddress boilSensor = {0x28, 0xB3, 0xB5, 0x73, 0x6, 0x0, 0x0, 0x2C};
#endif

double mashTemp = 0.0;
double boilTemp = 0.0;
double coilTemp = 0.0;
double hltTemp = 0.0;
double boilElementState;
double hltElementState;

int encoderA = D4;
int encoderB = D5;
int button = D6;
int redLed = D3;
int greenLed = D2;
int blueLed = D1;
int pump1 = WKP;
int pump2 = D0;
int boilElement = A2;
int hltElement = A1;
int mode = 0;

volatile int modePos = 4000;
volatile int boilPower = 0; //mode0
volatile double hltSet = 0; //mode1
volatile int pump1Power = 0; //mode2
volatile int pump2Power = 0; //mode3
volatile bool buttonPressed = false;
volatile bool modeChanged = false;
volatile bool A_set = false;
volatile bool B_set = false;

int prevMode = -1;

bool buttonArmed = false;
bool longPressActive = false;
bool timeToPublishFlag = false;

unsigned long buttonTimer = 0;
unsigned long longPressTime = 2000;

bool wifiState = false;
volatile bool debounced = true;

STARTUP(WiFi.selectAntenna(ANT_INTERNAL));
SYSTEM_MODE(AUTOMATIC);

// 10 second Time Proportional Output window
unsigned long WindowSize = 10000;
unsigned long windowStartTime;
volatile unsigned long boilOnTime = 0;

//Functions run on timer
Timer boilElementTimer(15, driveBoil);
Timer publish(6000, timeToPublish); //This is used to fire webhook. Webhook limit is 10 per minute
Timer runDebounce(10, debounce);

//######################################################################/
void setup() {
    dallas.begin();
    boilElementTimer.start();
    runDebounce.start();
    publish.start();
    display.Begin();
    setPinModes();
    initialiseLed();
    attachInterrupts();
    delay(3500); //Give the OLED time to fire up
    display.renderStaticText();
    boilPower = 0; //Cos sometimes it gets incremented during startup
}

void loop() {
    getTemperatures();
    drivePumps();
    driveHLT();
    display.renderUpdatedTemperatures(boilTemp, hltTemp, mashTemp, coilTemp);  //update display if  temp changed
    display.renderUpdatedSetpoints(boilPower, hltSet, pump1Power, pump2Power); //update display if setpoints changed
    dealWithButtonPress();
    dealWithModeChange();
    //note: some additional functions run on timer
}
//######################################################################/

//***SETUP FUNCTIONS***//

void setPinModes(){
    pinMode(encoderA, INPUT_PULLUP);
    pinMode(encoderB, INPUT_PULLUP);
    pinMode(redLed, OUTPUT);
    pinMode(greenLed, OUTPUT);
    pinMode(blueLed, OUTPUT);
    pinMode(button, INPUT_PULLDOWN);
    pinMode(pump1, OUTPUT);
    pinMode(pump2, OUTPUT);
    pinMode(boilElement, OUTPUT);
    pinMode(hltElement, OUTPUT);
}

void initialiseLed(){
    analogWrite(redLed, 0);
    analogWrite(greenLed, 255);
    analogWrite(blueLed, 255);
}

void attachInterrupts(){
    attachInterrupt(button, setButtonPressed, RISING);
    attachInterrupt(encoderA, doEncoderA, CHANGE);
    attachInterrupt(encoderB, doEncoderB, CHANGE);
}

//***LOOP FUNCTIONS***//
void getTemperatures(){
    dallas.requestTemperaturesByAddress(mashSensor);
    dallas.requestTemperaturesByAddress(boilSensor);
    dallas.requestTemperaturesByAddress(hltSensor);
    dallas.requestTemperaturesByAddress(coilSensor);
    saveTemperature(mashSensor, mashTemp);
    saveTemperature(boilSensor, boilTemp);
    saveTemperature(hltSensor, hltTemp);
    saveTemperature(coilSensor, coilTemp);
}

void drivePumps(){
    analogWrite(pump1, pump1Power*2.55, 65000);
    analogWrite(pump2, pump2Power*2.55, 65000);

    if (timeToPublishFlag){
        publishTemperatures();
        timeToPublishFlag = false;
    }
}

void driveHLT(){
    if (hltTemp < hltSet - 0.5){
        digitalWrite(boilElement,LOW);
        boilElementState = 0;
        display.renderElementIndicator(false, 1);
        digitalWrite(hltElement, HIGH);
        hltElementState = 1;
        display.renderElementIndicator(true, 2);
    } else if (hltTemp > hltSet + 0.5){
        digitalWrite(hltElement, LOW);
        hltElementState = 0;
        display.renderElementIndicator(false, 2);
    }
}

void dealWithButtonPress() {
    if (!modeChanged && buttonPressed) {
        if (pinReadFast(button) == HIGH) {
            if (buttonArmed == false) {
                buttonArmed = true;
                buttonTimer = millis();
            } else {
                if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
                    longPressActive = true;
                    //DO LONGPRESS STUFF
        //     		wifiState = !wifiState;
        // 			if (wifiState){
        // 			    mydisp.setTrueColor(255,0,0);
        //                 mydisp.drawCircle(70,3,3,1);
        // 			    connect();
        // 			    mydisp.setTrueColor(0,255,255);
        //                 mydisp.drawCircle(70,3,3,1);
        //             }
        //             else{
        //                 disconnect();
        //                 mydisp.setTrueColor(0,0,0);
        //                 mydisp.drawCircle(70,3,3,1);
        //             }
                }
            }
        } else {
            if (longPressActive) {
    		    longPressActive = false;
    		} else {
    		    switch(mode){
        		    case 0: {
            	        if (boilPower > 0){boilPower = 0;}
            	        else {boilPower = 100;}
            	        break;
        		    }
        		    case 1: {
            	        if (hltSet > 0){hltSet = 0;}
            	        else {hltSet = 70;}
            	        break;
        		    }
        		    case 2: {
            	        if (pump2Power > 0){pump2Power = 0;}
            	        else {pump2Power = 100;}
            	        break;
        		    }
        		    case 3: {
            	        if (pump1Power > 0){pump1Power = 0;}
            	        else {pump1Power = 100;}
            	        break;
        		    }
    		    }
    		}
        }
    }
    if (pinReadFast(button) == LOW) {
        modeChanged = false;
        buttonArmed = false;
    	buttonPressed = false;
    }
}

void dealWithModeChange(){
    if (prevMode != mode) {
        prevMode = mode;

        switch(mode) {
            case 0: //boil
            analogWrite(redLed, 0);
            analogWrite(greenLed, 255);
            analogWrite(blueLed, 255, 65000);
            display.renderBoilMode();
            break;

            case 1: //HLT
            analogWrite(redLed, 125);
            analogWrite(greenLed, 0);
            analogWrite(blueLed, 255, 65000);
            display.renderHLTMode();
            break;

            case 2: //Pump1
            analogWrite(redLed, 125);
            analogWrite(greenLed, 255);
            analogWrite(blueLed, 0, 65000);
            display.renderPump1Mode();
            break;

            case 3: //Pump2
            analogWrite(redLed, 255);
            analogWrite(greenLed, 0);
            analogWrite(blueLed, 255, 65000);
            display.renderPump2Mode();
            break;

        }

        //redrew the values
        display.renderAllSetpoints(boilPower, hltSet, pump1Power, pump2Power);
    }
}

//***INTERRUPT FUNCTIONS***//

void doEncoderA() {
  if(digitalRead(encoderA) != A_set ) {  // debounce once more
    A_set = !A_set;
    // adjust counter + if A leads B
    if (A_set && !B_set && debounced){
      if (digitalRead(button) == HIGH){
        modePos += 1;
        modeChanged = true;
        mode = (abs(modePos) % 4);
      } else if (mode == 0 && boilPower < 100){
          boilPower += 1;
      } else if (mode == 1 && hltSet < 100){
          hltSet += 0.1;
      } else if (mode == 2 && pump2Power < 100){
          pump2Power += 1;
      } else if (mode == 3 && pump1Power < 100){
          pump1Power += 1;
      }
      debounced = false;
    }
  }
}

void doEncoderB() {
    if(digitalRead(encoderB) != B_set) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
        if(B_set && !A_set && debounced){
          if (digitalRead(button) == HIGH){
            modePos -= 1;
            modeChanged = true;
            mode = (abs(modePos) % 4);
          } else if (mode == 0 && boilPower > 0){
              boilPower -= 1;
          } else if (mode == 1 && hltSet > 0.05){
              hltSet -= 0.1;
          } else if (mode == 2 && pump2Power > 0){
              pump2Power -= 1;
          } else if (mode == 3 && pump1Power > 0){
              pump1Power -= 1;
          }
          debounced = false;
        }
    }
}

//***TIMER FUNCTIONS***//

void driveBoil(){
    boilOnTime = boilPower * 100;
    unsigned long now = millis();
    if(now - windowStartTime>WindowSize){ //time to shift the Relay Window
        windowStartTime += WindowSize;
    }

    if((boilOnTime > 100) && (boilOnTime > (now - windowStartTime))){
        if ((digitalRead(boilElement) == LOW) && (digitalRead(hltElement) == LOW)) {
            digitalWrite(boilElement,HIGH);
            boilElementState=1;
            display.renderElementIndicator(true, 1);
        }
    }
    else if (digitalRead(boilElement) == HIGH){
        digitalWrite(boilElement,LOW);
        boilElementState=0;
        display.renderElementIndicator(false, 1);
    }
}

void timeToPublish(){
    timeToPublishFlag = true;
}

void debounce(){
    debounced = true;
}

//***HELPER FUNCTIONS***//

void saveTemperature(uint8_t* device, double &value){
    double temp = dallas.getTempC(device);
    if ( temp > 0 && temp < 120){
        value = round(temp*10)/10;
    }
}


void setButtonPressed() {
    buttonPressed = true;
}

void publishTemperatures() {
        char data[256];
        snprintf(data, sizeof(data), "{\"mashTemp\":%f, \"boilTemp\":%f, \"hltTemp\":%f, \"coilTemp\":%f, \"boilElementState\":%f, \"hltElementState\":%f, \"pump1Power\":%d, \"pump2Power\":%d, \"boilPower\":%d, \"hltSetPoint\":%f}", mashTemp, boilTemp, hltTemp, coilTemp, boilElementState, hltElementState, pump1Power, pump2Power, boilPower, hltSet);
        Particle.publish("brewbot_data", data, PRIVATE);
}

// void connect() {
//     if (Particle.connected() == false) {
//         Particle.connect();
//     }
//     publish.start();
// }

// void disconnect() {
//     publish.stop();
//     WiFi.off();
// }
