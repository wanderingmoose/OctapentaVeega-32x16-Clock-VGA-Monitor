/*Octapentaveega_XTRA_LARGE_DIGIT_CLOCK_V4
 * This is a test of larger numbers using the OctaPentaVeega board. 5 wide x 7 down characters.
 * Simple but shows some of the good things one can do with the simple 32x16 character screen.
 * Wanderingmoose Tinkering March 2026.
 *Arduino Nano with Octapentaveega module.
 Simple clock format with simple roll up of numbers.
 Date and temperature.
 DS1307 RTC I2C A4 and A5
 DS18B20 temperature on the D2 pin.
 Octapentaveega module on pin D8.
 monitor control out D4
 Control of time on/off of monitor using a switch D3
 Addin border around the time.
 Added mechincal roll and update order of digit order of roll.
 Added display boxes and arranged the dates and temperatures.
 Added Centered Time. Failed in V3, so we are attemping here again.
 Added a relay control for 7 am to 5pm on time and a switch to enable or disable this funciotn

 ///////////////////////////////
Sketch uses 15974 bytes (49%) of program storage space. Maximum is 32384 bytes.
Global variables use 1154 bytes (56%) of dynamic memory, leaving 894 bytes for local variables. Maximum is 2048 bytes.
 //////////////////////////////
 Works 
//********************************************************
//********************************************************
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>

SoftwareSerial vga(9, 8);
RTC_DS1307 rtc;

#define ONE_WIRE_BUS 2
#define RELAY_PIN 4
#define SWITCH_PIN 3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//////////////////////////////////////////////////////
// TERMINAL CONTROL
//////////////////////////////////////////////////////

const byte ESC[2] = {27,'['};
const byte CLR[4] = {27,'[','2','J'};

void SetCursor(int r,int c){
  vga.write(ESC,2);
  vga.print(r);
  vga.write(';');
  vga.print(c);
  vga.write('H');
}

void NoWrap(){
  vga.write(ESC,2);
  vga.print("?7l");
}

//////////////////////////////////////////////////////
// DIGITS 7x5
//////////////////////////////////////////////////////

const byte digits[11][7][5] = {
{{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,32,32,32,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128}},
{{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32},{32,32,128,32,32}},
{{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{128,128,128,128,128},{128,32,32,32,32},{128,32,32,32,32},{128,128,128,128,128}},
{{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{128,128,128,128,128}},
{{128,32,32,32,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128}},
{{128,128,128,128,128},{128,32,32,32,32},{128,32,32,32,32},{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{128,128,128,128,128}},
{{128,32,32,32,32},{128,32,32,32,32},{128,32,32,32,32},{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128}},
{{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128}},
{{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128}},
{{128,128,128,128,128},{128,32,32,32,128},{128,32,32,32,128},{128,128,128,128,128},{32,32,32,32,128},{32,32,32,32,128},{32,32,32,32,128}},
{{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32},{32,32,32,32,32}}
};

//////////////////////////////////////////////////////
// BOX GRAPHICS
//////////////////////////////////////////////////////

const byte UPBOX[32] = { 137, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 151 };
const byte MIDBOX[32] = { 138, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 138 };
const byte LOWBOX[32] = { 136, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 135 };
const byte CROSSBOX[32] = { 144, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 133, 140 };

//////////////////////////////////////////////////////
// COLOR
//////////////////////////////////////////////////////

void SetColor(int fore, int back) {
  vga.write(ESC, 2);
  vga.print(fore);
  vga.write(";");
  vga.print(back);
  vga.write("m");
}

//////////////////////////////////////////////////////
// DRAW DIGIT + ROLL
//////////////////////////////////////////////////////

void drawDigit(byte d,int row,int col){
  for(int r=0;r<7;r++){
    SetCursor(row+r,col);
    vga.write(digits[d][r],5);
  }
}

void rollDigit(byte oldD, byte newD, int row, int col){
  for(int step=0;step<=7;step++){
    for(int r=0;r<7;r++){
      int src=r+step;
      SetCursor(row+r,col);
      if(src<7) vga.write(digits[oldD][src],5);
      else vga.write(digits[newD][src-7],5);
    }
    delay(30);
  }
}

//////////////////////////////////////////////////////
// COLON
//////////////////////////////////////////////////////

void drawColon(int row,int col,bool state){
  SetCursor(row+2,col); vga.write(state?128:32);
  SetCursor(row+4,col); vga.write(state?128:32);
}

//////////////////////////////////////////////////////
// CLEAR CLOCK AREA
//////////////////////////////////////////////////////

void clearClockArea(int row){
  for(int r=0;r<7;r++){
    SetCursor(row+r,1);
    for(int c=0;c<30;c++) vga.write(32);
  }
}

//////////////////////////////////////////////////////
// CLOCK
//////////////////////////////////////////////////////

int prevDigits[4]={-1,-1,-1,-1};
bool prevFourDigit=false;

void drawClock(int h,int m,bool colonState){

  int row=1;

  int d[4];
  d[0]=h/10;
  d[1]=h%10;
  d[2]=m/10;
  d[3]=m%10;

  bool fourDigit=(d[0]!=0);
  if(!fourDigit) d[0]=10;

  // detect layout change
  if(prevFourDigit!=fourDigit){
    clearClockArea(row);
    for(int i=0;i<4;i++) prevDigits[i]=-1;
  }

  int digitW=5, gap=1, colonW=1;

  int start3=(30-(digitW*3+gap*3+colonW))/2+1;
  int start4=(30-(digitW*4+gap*4+colonW))/2+1;

  int start=fourDigit?start4:start3;

  int col[4];

  if(fourDigit){
    col[0]=start;
    col[1]=col[0]+digitW+gap;
  }else{
    col[0]=-1;
    col[1]=start;
  }

  int colonCol=col[1]+digitW+gap;
  col[2]=colonCol+colonW+gap;
  col[3]=col[2]+digitW+gap;

  for(int i=3;i>=0;i--){
    if(col[i]<0) continue;

    if(prevDigits[i]==-1)
      drawDigit(d[i],row,col[i]);
    else if(prevDigits[i]!=d[i]){
      if(prevDigits[i]==10 || d[i]==10)
        drawDigit(d[i],row,col[i]);
      else
        rollDigit(prevDigits[i],d[i],row,col[i]);
      delay(90);
    }
  }

  drawColon(row,colonCol,colonState);

  for(int i=0;i<4;i++) prevDigits[i]=d[i];
  prevFourDigit=fourDigit;
}

//////////////////////////////////////////////////////
// DATE
//////////////////////////////////////////////////////

void drawDate(DateTime now) {
  const char* days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  const char* months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  char buf[20];
  sprintf(buf, "%s %s %d %d",
          days[now.dayOfTheWeek()],
          months[now.month() - 1],
          now.day(),
          now.year());
  int col = (32 - strlen(buf)) / 2 + 1;
  SetCursor(12, col);
  vga.print(buf);
}

//////////////////////////////////////////////////////
// INFO PANEL
//////////////////////////////////////////////////////

void drawInfo(DateTime now, bool pm) {
  char buf[20];
  SetCursor(9, 29);
  vga.print(pm ? "PM" : "AM");
  sprintf(buf, "SEC %02d", now.second());
  SetCursor(9, 22);
  vga.print(buf);
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);
  float f = sensors.getTempFByIndex(0);
  SetCursor(11, 2);
  vga.print("TEMP ");
  vga.print(t, 2);
  vga.print(" C / ");
  SetCursor(11, 18);
  //vga.print("TEMP ");
  vga.print(f, 2);
  vga.print(" F ");
  sprintf(buf, "%04d-%02d-%02d", now.year(), now.month(), now.day());
  SetCursor(9, 1);
  vga.print(buf);
  SetCursor(9,12);
if(digitalRead(SWITCH_PIN)==LOW)
  vga.print("Disabled");
else if(now.hour()>=7 && now.hour()<17) //time(24 hour clock) on off of display also look at controlMonitor()
  vga.print(" Enabled");
else
  vga.print("   OFF  ");
}

//////////////////////////////////////////////////////
// TICKER
//////////////////////////////////////////////////////

String ticker = " OCTAPENTAVEEGA LARGE DIGIT CLOCK ";
int tickerPos = 0;
unsigned long tickerTimer = 0;
int tickerDelay = 200;

void drawTicker() {
  if (millis() - tickerTimer < tickerDelay)
    return;
  tickerTimer = millis();
  SetCursor(14, 1);
  for (int i = 0; i < 30; i++) {
    int index = (tickerPos + i) % ticker.length();
    vga.write(ticker[index]);
  }
  tickerPos++;
}

//////////////////////////////////////////////////////
// INIT SCREEN
//////////////////////////////////////////////////////

void InitScreen() {
  vga.write(CLR, 4);
  SetCursor(0, 0);
  SetColor(37, 40);
  vga.write(UPBOX, 32);
  for (int f = 1; f < 15; f++) {
    SetCursor(f, 0);
    vga.write(MIDBOX, 32);
  }
  SetCursor(15, 0);
  vga.write(LOWBOX, 32);
  SetCursor(10, 0);
  vga.write(CROSSBOX, 32);
  SetCursor(8, 0);
  vga.write(CROSSBOX, 32);
  SetCursor(13, 0);
  vga.write(CROSSBOX, 32);
}

/////////////////////////////////////////////
////Splash Screen
/////////////////////////////////////////////

void SplashScreen() {
  vga.write(CLR, 4);
  SetCursor(0, 0);
  SetColor(37, 40);
  vga.write(UPBOX, 32);
  for (int f = 1; f < 15; f++) {
    SetCursor(f, 0);
    vga.write(MIDBOX, 32);
  }
  SetCursor(15, 0);
  vga.write(LOWBOX, 32);
  SetCursor(4, 5);
  vga.print("OCTAPENTAVEEGA");
  SetCursor(6, 5);
  vga.print("March 2026");
  SetCursor(8, 5);
  vga.print("Xtra Large Digit");
  SetCursor(10, 5);
  vga.print("Clock Test");
  delay(500);
}

//////////////////////////////////////////////
//Monitor control 
//////////////////////////////////////////////

void controlMonitor(DateTime now)
{
  bool enableSchedule = digitalRead(SWITCH_PIN) == HIGH;
  // HIGH = switch NOT pressed = schedule ENABLED

  if(!enableSchedule)
  {
    // manual override: keep monitor ON
    digitalWrite(RELAY_PIN, HIGH);
    return;
  }

  int h = now.hour();

  if(h >= 7 && h < 17)  //display contol times(24 hour clock) also change in drawinfo()
  {
    // ON between 7:00–16:59
    digitalWrite(RELAY_PIN, HIGH);
  }
  else
  {
    // OFF otherwise
    digitalWrite(RELAY_PIN, LOW);
  }
}

//////////////////////////////////////////////////////
// SETUP + LOOP
//////////////////////////////////////////////////////

void setup(){
  vga.begin(9600);
//relay control info  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
// assume OFF at startup
  digitalWrite(RELAY_PIN, LOW);
  Wire.begin();
  rtc.begin();
  sensors.begin();
  delay(500);
  NoWrap();
SplashScreen();
  delay(5000);
  if(!rtc.isrunning())
    rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));

  vga.write(CLR,4);
  InitScreen();

}

int lastSecond=-1;

///////////////////////////////////////////////////
//Loop
//////////////////////////////////////////////////

void loop(){

  DateTime now=rtc.now();
  controlMonitor(now);  

  if(now.second()!=lastSecond){

    int hour=now.hour();
    bool pm=(hour>=12);

    hour%=12;
    if(hour==0) hour=12;

    drawClock(hour,now.minute(),now.second()%2==0);
    drawInfo(now,pm);

    lastSecond=now.second();
  }
  drawDate(now);
  drawTicker();
}