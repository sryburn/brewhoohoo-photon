#ifndef display_h
#define display_h
#define _Digole_Serial_UART_
#include "DigoleGeo.h"

class Display : public DigoleSerialDisp {

public:
Display() : DigoleSerialDisp(&Serial1, 115200){}
void Begin();
void renderStaticText();
void renderElementIndicator(bool show, int element);
void renderUpdatedTemperatures(double boilTemp, double hltTemp, double coilTemp, double mashTemp);
void renderUpdatedSetpoints(int boilPower, double hltSet, int pump1Power, int pump2Power);
void renderUpdatedMode(int mode, int prevMode);

private:
void renderBoilTemp(double boilTemp);
void renderHltTemp(double hltTemp);
void renderCoilTemp(double coilTemp);
void renderMashTemp(double mashTemp);
void renderBoilPower(int boilPower);
void renderHltSet(double hltSet);
void renderPump1Power(int pump1Power);
void renderPump2Power(int pump2Power);
int is_equal_3decplaces(double a, double b);
void drawMode(int x, int y, int h, int w);
};

#endif
