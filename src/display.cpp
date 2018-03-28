#include "display.h"

void Display::Begin(){
    begin();
}

void Display::renderStaticText(){
    clearScreen();
    backLightOn();
    setFont(18);
    setTrueColor(255,0,0);
    setPrintPos(0,20,1);
    print("B");
    setTrueColor(125,255,0);
    setPrintPos(80,20,1);
    print("H");
    setTrueColor(255,255,255);
    setPrintPos(80,50,1);
    print("M");
    setPrintPos(0,50,1);
    print("C");

}

void Display::renderElementIndicator(bool show, int element){
    if (show == true){
        setTrueColor(255,160,0);
    } else {
        setTrueColor(0,0,0);
    }

    switch(element){
        case 1: { //boilElement
            drawBox(0,0,7,7);
            break;
        }
        case 2: { //hltElement
            drawBox(80,0,7,7);
            break;
        }
    }
}

void Display::renderBoilMode(){
    setTrueColor(255,0,0);
    drawBox(0,64,79,31);
    setTrueColor(0,0,0);
    drawBox(4,68,71,23);
    drawBox(80,64,79,31);
    drawBox(80,96,79,31);
    drawBox(0,96,79,31);
    setTrueColor(255,255,255);
}

void Display::renderHLTMode(){
    setTrueColor(125,255,0);
    drawBox(80,64,79,31);
    setTrueColor(0,0,0);
    drawBox(84,68,71,23);
    drawBox(0,64,79,31);
    drawBox(80,96,79,31);
    drawBox(0,96,79,31);
    setTrueColor(255,255,255);
}
void Display::renderPump1Mode(){
    setTrueColor(125,0,255);
    drawBox(80,96,79,31);
    setTrueColor(0,0,0);
    drawBox(84,100,71,23);
    drawBox(0,64,79,31);
    drawBox(80,64,79,31);
    drawBox(0,96,79,31);
    setTrueColor(255,255,255);
}
void Display::renderPump2Mode(){
    setTrueColor(0, 255, 0);
    drawBox(0,96,79,31);
    setTrueColor(0,0,0);
    drawBox(4,100,71,23);
    drawBox(0,64,79,31);
    drawBox(80,64,79,31);
    drawBox(80,96,79,31);
    setTrueColor(255,255,255);
}

void Display::renderUpdatedTemperatures(double boilTemp, double hltTemp, double coilTemp, double mashTemp){
    static double prevMashTemp;
    static double prevBoilTemp;
    static double prevCoilTemp;
    static double prevHltTemp;

    if  (!is_equal_3decplaces(boilTemp, prevBoilTemp)){
        renderBoilTemp(boilTemp);
        prevBoilTemp = boilTemp;
    }

    if  (!is_equal_3decplaces(hltTemp, prevHltTemp)){
        renderHltTemp(hltTemp);
        prevHltTemp = hltTemp;
    }

    if  (!is_equal_3decplaces(coilTemp, prevCoilTemp)){
        renderCoilTemp(coilTemp);
        prevCoilTemp = coilTemp;
    }

    if  (!is_equal_3decplaces(mashTemp, prevMashTemp)){
        renderMashTemp(mashTemp);
        prevMashTemp = mashTemp;
    }
}

void Display::renderUpdatedSetpoints(int boilPower, double hltSet, int pump1Power, int pump2Power){
    static int prevBoilPower;
    static double prevHltSet;
    static int prevPump2Power;
    static int prevPump1Power;

    if (boilPower != prevBoilPower){
        renderBoilPower(boilPower);
        prevBoilPower = boilPower;
    }

    if (hltSet != prevHltSet){
        renderHltSet(hltSet);
        prevHltSet = hltSet;
    }

    if (pump2Power != prevPump2Power){
        renderPump2Power(pump2Power);
        prevPump2Power = pump2Power;
    }

    if (pump1Power != prevPump1Power){
        renderPump1Power(pump1Power);
        prevPump1Power = pump1Power;
    }
}

void Display::renderAllSetpoints(int boilPower, double hltSet, int pump1Power, int pump2Power){
    char degree = 0xB0;
    setFont(18);

    setTrueColor(255,255,255);
    setPrintPos(10,89,1);
    print(boilPower, 1);
    print('%');

    setPrintPos(90,89,1);
    print(hltSet, 1);
    print(degree);
    print('C');

    setPrintPos(90,121,1);
    print(pump2Power, 1);
    print('%');

    setPrintPos(10,121,1);
    print(pump1Power, 1);
    print('%');

    setFont(10);
    setTrueColor(255,255,255);
    setPrintPos(10,76,1);
    print("Boil Power");
    setPrintPos(90,76,1);
    print("HLT Set");
    setPrintPos(90,108,1);
    print("Pump 2");
    setPrintPos(10,108,1);
    print("Pump 1");
}

//private functions
void Display::renderBoilTemp(double boilTemp){
    setFont(120);
    setTrueColor(255,0,0);
    setPrintPos(10,20,1);
    print(boilTemp, 1);
}

void Display::renderHltTemp(double hltTemp){
    setFont(120);
    setTrueColor(125,255,0);
    setPrintPos(90,20,1);
    print(hltTemp, 1);
}

void Display::renderCoilTemp(double coilTemp){
    setFont(120);
    setTrueColor(255,255,255);
    setPrintPos(10,50,1);
    print(coilTemp, 1);
}

void Display::renderMashTemp(double mashTemp){
    setFont(120);
    setTrueColor(255,255,255);
    setPrintPos(90,50,1);
    print(mashTemp, 1);
}

void Display::renderBoilPower(int boilPower){
    setTrueColor(0,0,0);
    drawBox(4,77,71,14);
    setFont(18);
    setTrueColor(255,255,255);
    setPrintPos(10,89,1);
    print(boilPower);
    print('%');
}

void Display::renderHltSet(double hltSet){
    char degree = 0xB0;
    setTrueColor(0,0,0);
    drawBox(84,77,71,14);
    setFont(18);
    setTrueColor(255,255,255);
    setPrintPos(90,89,1);
    print(hltSet,1);
    print(degree);
    print('C');
}

void Display::renderPump1Power(int pump1Power){
    setTrueColor(0,0,0);
    drawBox(4,109,71,14);
    setFont(18);
    setTrueColor(255,255,255);
    setPrintPos(10,121,1);
    print(pump1Power);
    print('%');
}

void Display::renderPump2Power(int pump2Power){
    setTrueColor(0,0,0);
    drawBox(84,109,71,14);
    setFont(18);
    setTrueColor(255,255,255);
    setPrintPos(90,121,1);
    print(pump2Power);
    print('%');
}

int Display::is_equal_3decplaces(double a, double b) {
    long long ai = a * 1000;
    long long bi = b * 1000;
    return ai == bi;
}
