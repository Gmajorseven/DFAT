#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SH1106.h>
#include <DS3231.h>

#define OLED_RESET 4
#define relayPin 13
#define setPin 12
#define incPin 11
#define decPin 10


Adafruit_SH1106 display(OLED_RESET);
DS3231 clock;
RTCDateTime dt;

bool deviceStatus = false;
int onHour, onMinute, offHour, offMinute, index = 0;

bool iseditMode() {
  return index > 0;
}

void displayCursor() {
  if (iseditMode()) {
    int timeColumn = 26 + ((index - 1) % 3) * 36;
    int dateColumn = 38 + ((index - 1) % 3) * 19;
    int size = index < 4 ? 2 : 1;
    int col = index < 4 ? timeColumn : dateColumn;
    int row = index < 4 ? 10 : 28;
    display.setTextSize(size);
    display.setCursor(col, row);
    display.setTextColor(BLACK, WHITE);
    display.print("_");
    display.display();
    delay(200);
  } else {
    display.clearDisplay();
  }
}

void incTime() {
  switch (index) {
    case 1: dt.hour = (dt.hour + 1) % 24; break;
    case 2: dt.minute = (dt.minute + 1) % 60; break;
    case 3: dt.second = (dt.second + 1) % 60; break;
    case 4: dt.day = (dt.day % 31) + 1; break;
    case 5: dt.month = (dt.month % 12) + 1; break;
    case 6: dt.year = dt.year + 1; break;
  }
}

void decTime() {
  switch (index) {
    case 1: dt.hour = (dt.hour - 1) % 24; break;
    case 2: dt.minute = (dt.minute - 1) % 60; break;
    case 3: dt.second = (dt.second - 1) % 60; break;
    case 4: dt.day = (dt.day % 31) - 1; break;
    case 5: dt.month = (dt.month - 12) + 1; break;
    case 6: dt.year = dt.year - 1; break;
  }
}

void updateTime() {
  if (!iseditMode()) {
  dt = clock.getDateTime();
  }
}

void checkEditMode() {
  if (digitalRead(setPin) == LOW) {
    index = (index + 1) % 7;
    Serial.println("Set pressed!");
    if (!iseditMode()) {
      clock.setDateTime(dt.year , dt.month, dt.day, dt.hour, dt.minute, dt.second);
    }
  }
}

void checkincMode() {
  if (iseditMode() && digitalRead(incPin) == LOW) {
    Serial.println("Inc pressed!");
    incTime();
  }
  if (iseditMode() && digitalRead(decPin) == LOW) {
    Serial.println("Dec pressed!");
    decTime();
  }
}

bool autoTiming(bool status, int month, int hour, int minute) {
  if (month >= 4 && month <= 9) {
    // Summer months (April to September)
    onHour = 18; 
    onMinute = 25;
    offHour = 5; 
    offMinute = 45;
  } else {
    // Winter months (October to March)
    onHour = 17;  
    onMinute = 30;
    offHour = 5; 
    offMinute = 50;
  }
  int time_inminite = (hour * 60) + minute;
  int on_time_inminite = (onHour * 60) + onMinute;
  int off_time_inminite = (offHour * 60) + offMinute;
  if (on_time_inminite < off_time_inminite) {
    if (time_inminite >= on_time_inminite && time_inminite < off_time_inminite) {
      return status = true;
    } else {
      return status = false; // Light is off outside the on-off range
    }
  } else {
    // Handle the case where on time is after midnight
    if (time_inminite >= on_time_inminite || time_inminite < off_time_inminite) {
      return status = true;
    } else {
      return status = false; // Light is off outside the on-off range
    }
  }
}

void toggleDevice(bool status) {
  digitalWrite(relayPin, status ? HIGH : LOW);
}

void displayMain(int day, int month, int year, int hour, int minute, int second, bool status) {
  char buf[10];   // "HH:MM:SS" plus null terminator
  char buf2[11];  // Enough for "DD/MM/YYYY" + null
  char buf3[13];  // For light status
  sprintf(buf, "%2d:%02d:%02d", hour, minute, second);
  sprintf(buf2, "%02d-%02d-%04d", day, month, year);
  sprintf(buf3, "Device:%s", status ? "ON" : "OFF");

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15, 10);
  display.println(buf);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(33, 28);
  display.println(buf2);
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(34, 45);
  for (int i = 1; i <= 9; i++) {
    display.writeLine(255, 43+i, 0, 43+i, WHITE);
  }
  display.println(buf3);
  display.display();
  delay(200);
}
void setup()
{
  Serial.begin(9600);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);

  display.display();
  Serial.println("Initialize Display");
  delay(2000);
  display.clearDisplay();

  Serial.println("Initialize DS3231");
  clock.begin();

  pinMode(relayPin, OUTPUT);
  pinMode(setPin, INPUT);
  pinMode(incPin, INPUT);
  pinMode(decPin, INPUT);

  // Set sketch compiling time
  clock.setDateTime(__DATE__, __TIME__);
}

void loop()
{
  updateTime();
  toggleDevice(autoTiming(deviceStatus, dt.month, dt.hour, dt.minute));
  displayMain(dt.day, dt.month, dt.year, dt.hour, dt.minute, dt.second, autoTiming(deviceStatus, dt.month, dt.hour, dt.minute));
  displayCursor();
  checkEditMode();
  checkincMode();
}
