#include "display.h"

void Display::Begin(){
    begin();
    backLightOn();
}

void Display::renderStaticText(){
    clearScreen();
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
    setFont(10);
    setPrintPos(10,76,1);
    print("Boil Power");
    setPrintPos(90,76,1);
    print("HLT Set");
    setPrintPos(90,108,1);
    print("Pump 2");
    setPrintPos(10,108,1);
    print("Pump 1");
}

void Display::renderElementIndicator(bool show, int element){
    if (show == true){
        setTrueColor(255,160,0);
    } else {
        setTrueColor(0,0,0);
    }

    switch(element){
        case 1: //boilElement
        drawBox(0,0,7,7);
        break;

        case 2: //hltElement
        drawBox(80,0,7,7);
        break;
    }
}

void Display::renderUpdatedTemperatures(double boilTemp, double hltTemp, double coilTemp, double mashTemp){
    static double prevMashTemp;
    static double prevBoilTemp;
    static double prevCoilTemp;
    static double prevHltTemp;

    setFont(120);

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
    static int prevBoilPower = -1;
    static double prevHltSet = -1;
    static int prevPump2Power = -1;
    static int prevPump1Power = -1;

    setFont(18);

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

void Display::renderUpdatedMode(int mode, int prevMode){

    switch(mode){
        case 0:
        setTrueColor(255,0,0);
        drawMode(0,64,79,31);
        break;

        case 1:
        setTrueColor(125,255,0);
        drawMode(80,64,79,31);
        break;

        case 2:
        setTrueColor(125,0,255);
        drawMode(80,96,79,31);
        break;

        case 3:
        setTrueColor(0,255,0);
        drawMode(0,96,79,31);
        break;
    }

    setTrueColor(0,0,0);

    switch(prevMode){
        case 0:
        drawMode(0,64,79,31);
        break;

        case 1:
        drawMode(80,64,79,31);
        break;

        case 2:
        drawMode(80,96,79,31);
        break;

        case 3:
        drawMode(0,96,79,31);
        break;
    }
}

//private functions
void Display::renderBoilTemp(double boilTemp){
    setTrueColor(255,0,0);
    setPrintPos(10,20,1);
    print(boilTemp, 1);
}

void Display::renderHltTemp(double hltTemp){
    setTrueColor(125,255,0);
    setPrintPos(90,20,1);
    print(hltTemp, 1);
}

void Display::renderCoilTemp(double coilTemp){
    setTrueColor(255,255,255);
    setPrintPos(10,50,1);
    print(coilTemp, 1);
}

void Display::renderMashTemp(double mashTemp){
    setTrueColor(255,255,255);
    setPrintPos(90,50,1);
    print(mashTemp, 1);
}

void Display::renderBoilPower(int boilPower){
    setTrueColor(0,0,0);
    drawBox(4,77,71,14);
    setTrueColor(255,255,255);
    setPrintPos(10,89,1);
    print(boilPower);
    print('%');
}

void Display::renderHltSet(double hltSet){
    char degree = 0xB0;
    setTrueColor(0,0,0);
    drawBox(84,77,71,14);
    setTrueColor(255,255,255);
    setPrintPos(90,89,1);
    print(hltSet,1);
    print(degree);
    print('C');
}

void Display::renderPump1Power(int pump1Power){
    setTrueColor(0,0,0);
    drawBox(4,109,71,14);
    setTrueColor(255,255,255);
    setPrintPos(10,121,1);
    print(pump1Power);
    print('%');
}

void Display::renderPump2Power(int pump2Power){
    setTrueColor(0,0,0);
    drawBox(84,109,71,14);
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

void Display::drawMode(int x, int y, int h, int w){
    for(int i=0; i<4; i++ ){
        drawFrame(x+i,y+i,h-2*i,w-2*i);
    }
}
