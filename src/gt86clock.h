// 主程序文件，包含所有库引用和全局变量定义
#ifndef GT86CLOCK_H
#define GT86CLOCK_H

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;

// MCP2515 CAN模块已移除，改用ADC传感器输入

#include <EEPROM.h>
// #include "DS3231.h"   // 不使用这个库

#include <base64.h>
#include <RTClib.h> // 如果 DateTime 来自 RTClib
extern RTC_DS3231 RTC;

// DS3231 Clock;

#include "logos.h"

// WiFi相关库已移除

extern RTC_DS3231 Clock;
extern DateTime dateTime;
extern DateTime now;

#define MAXSCREENS 6
#define LOGO 0
#define CLOCK 1
#define OILTEMP 2
#define COOLANTTEMP 3
#define OILCOOLANTTEMP 4
#define OILPRESSURE 5

extern int buttonPin1;
extern int buttonPin2;
extern int buttonPin1Pressed;
extern int buttonPin2Pressed;
extern int clockHour;
extern int clockMinute;
extern int coolantTemp;
extern int modeCurrent;
extern int modeOld;
extern int modeSaved;
extern int oilTemp;
extern int lastModeChange;
extern int lastTempUpdate;
// extern int eepromReadInt(int adr);

// CAN相关变量已移除

// o2afr变量已移除
extern bool clockRefresh;
extern bool clock24h;
extern bool pm;
extern bool drawDots;
extern bool pressureBar;
extern bool temperatureCelsius;
extern bool readyForModeChange;
extern bool updateCompleteDisplay;

// afr和voltage变量已移除
extern float oilPressure;
extern float oilPressureOld;
extern float oilPressureOffset;
extern float oilPressureScalingFactor;

// WebServer相关已移除

// CAN数据缓冲区已移除

// JSON文件和NTP同步相关已移除

// 添加全局标志位
extern volatile bool button1PressedFlag;
extern volatile bool button2PressedFlag;
extern volatile bool syncRTCFlag;
extern volatile bool saveConfigFlag;

extern int buttonPin3;
extern int buttonPin3Pressed;
extern volatile bool button3PressedFlag;
#endif
