#include "gt86clock.h"

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/21);
// MCP_CAN CAN0(5); // CAN模块已移除
WebServer server(80);
WiFiManager wifiManager;
RTC_DS3231 RTC;
RTC_DS3231 Clock;
DateTime dateTime;
DateTime now;
WiFiUDP ntpUDP;
NTPClient NTPch(ntpUDP, "pool.ntp.org");

int buttonPin1 = 2; // 按钮引脚为 2
int buttonPin2 = 34; // 按钮引脚为 34
int buttonPin3 = 35; // 按钮引脚为35
int buttonPin1Pressed = -1;
int buttonPin2Pressed = -1;
int buttonPin3Pressed = -1;
int clockHour = 8;
int clockMinute = 30;
int coolantTemp = 0;
int modeCurrent = 0;
int modeOld = -1;
int modeSaved = -1;
int oilTemp = 0;
int lastModeChange = 0;
int lastTempUpdate = 0;
int lastJsonAppend = 0;
// long unsigned int rxId; // CAN相关变量已移除
bool o2afr = true;
bool clockRefresh = false;
bool clock24h = true;
bool pm;
bool drawDots = false;
bool pressureBar = false; // 固定使用bar（公制单位）
bool temperatureCelsius = false; // 固定使用摄氏度（false表示摄氏度）
bool readyForModeChange = true;
bool updateCompleteDisplay = true;
float afr = 14.6;
float voltage = 0;
float oilPressure;
float oilPressureOld;
float oilPressureOffset = 114;
float oilPressureScalingFactor = 0.172018348623853;
String jsonFile;
// unsigned char len = 0; // CAN数据缓冲区已移除
// unsigned char buf[12]; // CAN数据缓冲区已移除

// 定义全局变量（分配内存）
volatile bool button1PressedFlag = false;
volatile bool button2PressedFlag = false;
volatile bool syncRTCFlag = false;
volatile bool saveConfigFlag = false;
volatile bool syncNTPFlag = false;
// 双键标志定义
volatile bool doubleButtonPressedFlag = false;
volatile bool button3PressedFlag = false;
