// 主程序文件，包含所有库引用和全局变量定义
#ifndef GT86CLOCK_H
#define GT86CLOCK_H

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;

#include <mcp_can.h>
#include <SPI.h>
#define CAN0_INT 4 // ESP32: CAN INT引脚
extern MCP_CAN CAN0;

#include <EEPROM.h>
// #include "DS3231.h"   // 不使用这个库

#include <base64.h>
#include <RTClib.h> // 如果 DateTime 来自 RTClib
extern RTC_DS3231 RTC;

// DS3231 Clock;

#include "logos.h"

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
extern WebServer server;
extern WiFiManager wifiManager;

#include <FS.h>
#include <SPIFFS.h>

#include <NTPClient.h> // NTP 客户端库
#include <WiFiUdp.h>   // UDP 依赖库

// 全局声明 NTP 客户端和 DateTime 变量
extern WiFiUDP ntpUDP;
extern NTPClient NTPch;
extern RTC_DS3231 Clock;
extern DateTime dateTime;
extern DateTime now;

#define MAXSCREENS 13
#define LOGO 0
#define CLOCK 1
#define OILTEMP 2
#define COOLANTTEMP 3
#define OILCOOLANTTEMP 4
#define OILPRESSURE 5
#define O2 6
#define SETTINGSWIFI 7
#define SETTINGSCLOCK 8
#define SETTINGSUNITS 9
#define SETTINGSO2 10

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
extern int lastJsonAppend;
// extern int eepromReadInt(int adr);

extern long unsigned int rxId;

extern bool o2afr;
extern bool clockRefresh;
extern bool clock24h;
extern bool pm;
extern bool drawDots;
extern bool pressureBar;
extern bool temperatureCelsius;
extern bool readyForModeChange;
extern bool updateCompleteDisplay;

extern float afr;
extern float voltage;
extern float oilPressure;
extern float oilPressureOld;
extern float oilPressureOffset;
extern float oilPressureScalingFactor;

extern WebServer server; // 声明外部全局变量

extern String jsonFile;

extern unsigned char len;
extern unsigned char buf[12];

extern String jsonFile; // 声明全局变量

// 添加全局标志位
extern volatile bool button1PressedFlag;
extern volatile bool button2PressedFlag;
extern volatile bool syncRTCFlag;
extern volatile bool saveConfigFlag;
extern volatile bool syncNTPFlag;

extern int buttonPin3;
extern int buttonPin3Pressed;
extern volatile bool button3PressedFlag;
#endif
