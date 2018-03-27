#define _Digole_Serial_UART_

#include "DigoleGeo.h"
#include "spark-dallas-temperature.h"

SYSTEM_THREAD(ENABLED);

DallasTemperature dallas(new OneWire(A0));

//Brew probes
// DeviceAddress mashSensor = {0x28, 0xB8, 0x4E, 0x74, 0x6, 0x0, 0x0, 0x84};
// DeviceAddress coilSensor = {0x28, 0xB7, 0x3A, 0x74, 0x6, 0x0, 0x0, 0xD2};
// DeviceAddress hltSensor = {0x28, 0xE7, 0xC, 0x74, 0x6, 0x0, 0x0, 0xE4};
// DeviceAddress boilSensor = {0x28, 0xB3, 0xB5, 0x73, 0x6, 0x0, 0x0, 0x2C};


//Test probes
DeviceAddress mashSensor = {0x28, 0xFF, 0x5E, 0xB6, 0x0, 0x17, 0x4, 0x4};
DeviceAddress coilSensor = {0x28, 0xFF, 0x63, 0xA8, 0x0, 0x17, 0x5, 0x1F};
DeviceAddress hltSensor = {0x28, 0x5B, 0xC4, 0xAC, 0x9, 0x0, 0x0, 0xC0};
DeviceAddress boilSensor = {0x28, 0xFF, 0xA8, 0xAF, 0x0, 0x17, 0x4, 0x95};

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
int prevBoilPower = -1;
int prevPump1Power = -1;
int prevPump2Power = -1;
double prevHltSet = -0.1;
double prevMashTemp = 1.0;
double prevBoilTemp = 1.0;
double prevCoilTemp = 1.0;
double prevHltTemp = 1.0;

char degree = 0xB0;

bool buttonArmed = false;
bool longPressActive = false;
bool timeToPublishFlag = false;

unsigned long buttonTimer = 0;
unsigned long longPressTime = 2000;

bool wifiState = false;
volatile bool debounced = true;

STARTUP(WiFi.selectAntenna(ANT_INTERNAL));
SYSTEM_MODE(AUTOMATIC);

DigoleSerialDisp mydisp(&Serial1, 115200);

// 10 second Time Proportional Output window
unsigned long WindowSize = 10000;
unsigned long windowStartTime;
volatile unsigned long boilOnTime = 0;

//Functions run on timer
Timer boilElementTimer(15, driveBoil);
Timer publish(6000, timeToPublish); //This is used to fire webhook. Webhook limit is 10 per minute
Timer runDebounce(10, debounce);

void setup() {
    dallas.begin();
    // Particle.variable("mashTemp", mashTemp);
    // Particle.variable("boilTemp", boilTemp);

    boilElementTimer.start();
    runDebounce.start();
    publish.start();
    mydisp.begin();

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

    analogWrite(redLed, 0);
    analogWrite(greenLed, 255);
    analogWrite(blueLed, 255);
    attachInterrupt(button, setButtonPressed, RISING);
    attachInterrupt(encoderA, doEncoderA, CHANGE);
    attachInterrupt(encoderB, doEncoderB, CHANGE);
    delay(3500);

    displayStaticText();

    boilPower = 0;

}

void loop() {
    //get temperatures
    dallas.requestTemperaturesByAddress(mashSensor);
    dallas.requestTemperaturesByAddress(boilSensor);
    dallas.requestTemperaturesByAddress(hltSensor);
    dallas.requestTemperaturesByAddress(coilSensor);
    saveTemperature(mashSensor, mashTemp);
    saveTemperature(boilSensor, boilTemp);
    saveTemperature(hltSensor, hltTemp);
    saveTemperature(coilSensor, coilTemp);

    //Drive Pumps
    analogWrite(pump1, pump1Power*2.55, 65000);
    analogWrite(pump2, pump2Power*2.55, 65000);

    if (timeToPublishFlag){
        publishTemperatures();
        timeToPublishFlag = false;
    }

    //Drive HLT
    if (hltTemp < hltSet - 0.5){
        digitalWrite(boilElement,LOW);
        boilElementState = 0;
        displayElementIndicator(false, 1);
        digitalWrite(hltElement, HIGH);
        hltElementState = 1;
        displayElementIndicator(true, 2);
        mydisp.drawBox(80,0,7,7);
    } else if (hltTemp > hltSet + 0.5){
        digitalWrite(hltElement, LOW);
        hltElementState = 0;
        displayElementIndicator(false, 2);
    }

    //Boil driven from timer


    //update display if  temp changed
    displayUpdatedTemperatures();


    //update display if setpoints changed
    if (boilPower != prevBoilPower){
        mydisp.setTrueColor(0,0,0);
        mydisp.drawBox(4,77,71,14);
        mydisp.setFont(18);
        mydisp.setTrueColor(255,255,255);
        mydisp.setPrintPos(10,89,1);
        mydisp.print(boilPower);
        mydisp.print('%');
        prevBoilPower = boilPower;
        boilOnTime = boilPower * 100;
    }

    if (hltSet != prevHltSet){
        mydisp.setTrueColor(0,0,0);
        mydisp.drawBox(84,77,71,14);
        mydisp.setFont(18);
        mydisp.setTrueColor(255,255,255);
        mydisp.setPrintPos(90,89,1);
        mydisp.print(hltSet,1);
        mydisp.print(degree);
        mydisp.print('C');
        prevHltSet = hltSet;
    }

    if (pump2Power != prevPump2Power){
        mydisp.setTrueColor(0,0,0);
        mydisp.drawBox(84,109,71,14);
        mydisp.setFont(18);
        mydisp.setTrueColor(255,255,255);
        mydisp.setPrintPos(90,121,1);
        mydisp.print(pump2Power);
        prevPump2Power = pump2Power;
        mydisp.print('%');
    }

    if (pump1Power != prevPump1Power){
        mydisp.setTrueColor(0,0,0);
        mydisp.drawBox(4,109,71,14);
        mydisp.setFont(18);
        mydisp.setTrueColor(255,255,255);
        mydisp.setPrintPos(10,121,1);
        mydisp.print(pump1Power);
        prevPump1Power = pump1Power;
        mydisp.print('%');
    }
    //dealing with button press
    dealWithButtonPress();

    //dealing with mode changes

    if (prevMode != mode) {
        prevMode = mode;

        switch(mode) {
            case 0: //boil
            analogWrite(redLed, 0);
            analogWrite(greenLed, 255);
            analogWrite(blueLed, 255, 65000);
            mydisp.setTrueColor(255,0,0);
            mydisp.drawBox(0,64,79,31);
            mydisp.setTrueColor(0,0,0);
            mydisp.drawBox(4,68,71,23);
            mydisp.drawBox(80,64,79,31);
            mydisp.drawBox(80,96,79,31);
            mydisp.drawBox(0,96,79,31);
            mydisp.setTrueColor(255,255,255);
            break;

            case 1: //HLT
            analogWrite(redLed, 125);
            analogWrite(greenLed, 0);
            analogWrite(blueLed, 255, 65000);
            mydisp.setTrueColor(125,255,0);
            mydisp.drawBox(80,64,79,31);
            mydisp.setTrueColor(0,0,0);
            mydisp.drawBox(84,68,71,23);
            mydisp.drawBox(0,64,79,31);
            mydisp.drawBox(80,96,79,31);
            mydisp.drawBox(0,96,79,31);
            mydisp.setTrueColor(255,255,255);
            break;

            case 2: //Pump1
            analogWrite(redLed, 125);
            analogWrite(greenLed, 255);
            analogWrite(blueLed, 0, 65000);
            mydisp.setTrueColor(125,0,255);
            mydisp.drawBox(80,96,79,31);
            mydisp.setTrueColor(0,0,0);
            mydisp.drawBox(84,100,71,23);
            mydisp.drawBox(0,64,79,31);
            mydisp.drawBox(80,64,79,31);
            mydisp.drawBox(0,96,79,31);
            mydisp.setTrueColor(255,255,255);
            break;

            case 3: //Pump2
            analogWrite(redLed, 255);
            analogWrite(greenLed, 0);
            analogWrite(blueLed, 255, 65000);
            mydisp.setTrueColor(0, 255, 0);
            mydisp.drawBox(0,96,79,31);
            mydisp.setTrueColor(0,0,0);
            mydisp.drawBox(4,100,71,23);
            mydisp.drawBox(0,64,79,31);
            mydisp.drawBox(80,64,79,31);
            mydisp.drawBox(80,96,79,31);
            mydisp.setTrueColor(255,255,255);
            break;

        }
        //redrew the values
        mydisp.setFont(18);

        mydisp.setTrueColor(255,255,255);
        mydisp.setPrintPos(10,89,1);
        mydisp.print(boilPower, 1);
        mydisp.print('%');

        mydisp.setPrintPos(90,89,1);
        mydisp.print(hltSet, 1);
        mydisp.print(degree);
        mydisp.print('C');

        mydisp.setPrintPos(90,121,1);
        mydisp.print(pump2Power, 1);
        mydisp.print('%');

        mydisp.setPrintPos(10,121,1);
        mydisp.print(pump1Power, 1);
        mydisp.print('%');

        mydisp.setFont(10);
        mydisp.setTrueColor(255,255,255);
        mydisp.setPrintPos(10,76,1);
        mydisp.print("Boil Power");
        mydisp.setPrintPos(90,76,1);
        mydisp.print("HLT Set");
        mydisp.setPrintPos(90,108,1);
        mydisp.print("Pump 2");
        mydisp.setPrintPos(10,108,1);
        mydisp.print("Pump 1");

    }

}
//----------------------------------------------------------------

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

// Interrupt on B changing state, same as A above
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
          } else if (mode == 1 && hltSet > 0){
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

void setButtonPressed() {
    buttonPressed = true;
}

void saveTemperature(uint8_t* device, double &value){
    double temp = dallas.getTempC(device);
    if ( temp > 0 && temp < 120){
        value = round(temp*10)/10;
    }
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

void publishTemperatures() {
        char data[256];
        snprintf(data, sizeof(data), "{\"mashTemp\":%f, \"boilTemp\":%f, \"hltTemp\":%f, \"coilTemp\":%f, \"boilElementState\":%f, \"hltElementState\":%f, \"pump1Power\":%d, \"pump2Power\":%d, \"boilPower\":%d, \"hltSetPoint\":%f}", mashTemp, boilTemp, hltTemp, coilTemp, boilElementState, hltElementState, pump1Power, pump2Power, boilPower, hltSet);
        Particle.publish("brewbot_data", data, PRIVATE);
}

void timeToPublish(){
    timeToPublishFlag = true;
}

void debounce(){
    debounced = true;
}

int is_equal_3decplaces(double a, double b) {
    long long ai = a * 1000;
    long long bi = b * 1000;
    return ai == bi;
}

void driveBoil(){
    unsigned long now = millis();
    if(now - windowStartTime>WindowSize){ //time to shift the Relay Window
        windowStartTime += WindowSize;
    }

    if((boilOnTime > 100) && (boilOnTime > (now - windowStartTime))){
        if ((digitalRead(boilElement) == LOW) && (digitalRead(hltElement) == LOW)) {
            digitalWrite(boilElement,HIGH);
            boilElementState=1;
            displayElementIndicator(true, 1);
        }
    }
    else if (digitalRead(boilElement) == HIGH){
        digitalWrite(boilElement,LOW);
        boilElementState=0;
        displayElementIndicator(false, 1);
    }
}

void displayStaticText(){
    mydisp.clearScreen();
    mydisp.backLightOn();
    mydisp.setFont(18);
    mydisp.setTrueColor(255,0,0);
    mydisp.setPrintPos(0,20,1);
    mydisp.print("B");
    mydisp.setTrueColor(125,255,0);
    mydisp.setPrintPos(80,20,1);
    mydisp.print("H");
    mydisp.setTrueColor(255,255,255);
    mydisp.setPrintPos(80,50,1);
    mydisp.print("M");
    mydisp.setPrintPos(0,50,1);
    mydisp.print("C");

}

void displayElementIndicator(bool show, int element){
    if (show == true){
        mydisp.setTrueColor(255,160,0);
    } else {
        mydisp.setTrueColor(0,0,0);
    }

    switch(element){
        case 1: { //boilElement
            mydisp.drawBox(0,0,7,7);
            break;
        }
        case 2: { //hltElement
            mydisp.drawBox(80,0,7,7);
            break;
        }
    }
}

void displayUpdatedTemperatures(){
    if  (!is_equal_3decplaces(boilTemp, prevBoilTemp)){
        mydisp.setFont(120);
        mydisp.setTrueColor(255,0,0);
        mydisp.setPrintPos(10,20,1);
        mydisp.print(boilTemp, 1);
        prevBoilTemp = boilTemp;
    }

    if  (!is_equal_3decplaces(hltTemp, prevHltTemp)){
        mydisp.setFont(120);
        mydisp.setTrueColor(125,255,0);
        mydisp.setPrintPos(90,20,1);
        mydisp.print(hltTemp, 1);
        prevHltTemp = hltTemp;
    }

    if  (!is_equal_3decplaces(coilTemp, prevCoilTemp)){
        mydisp.setFont(120);
        mydisp.setTrueColor(255,255,255);
        mydisp.setPrintPos(10,50,1);
        mydisp.print(coilTemp, 1);
        prevCoilTemp = coilTemp;
    }

    if  (!is_equal_3decplaces(mashTemp, prevMashTemp)){
        mydisp.setFont(120);
        mydisp.setTrueColor(255,255,255);
        mydisp.setPrintPos(90,50,1);
        mydisp.print(mashTemp, 1);
        prevMashTemp = mashTemp;
    }
}
