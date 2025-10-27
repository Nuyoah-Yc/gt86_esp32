#include "gt86clock.h"

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/22, /* data=*/23);
RTC_DS3231 RTC;
RTC_DS3231 Clock;
DateTime dateTime;
DateTime now;

int buttonPin1 = 5; // 按钮引脚为 5
int buttonPin2 = 18; // 按钮引脚为 18
int buttonPin3 = 19; // 按钮引脚为 19
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
bool clockRefresh = false;
bool clock24h = true;
bool pm;
bool drawDots = false;
bool pressureBar = false;
bool temperatureCelsius = false;
bool readyForModeChange = true;
bool updateCompleteDisplay = true;
float oilPressure;
float oilPressureOld;
float oilPressureOffset = 114;
float oilPressureScalingFactor = 0.172018348623853;

// 定义全局变量（分配内存）
volatile bool button1PressedFlag = false;
volatile bool button2PressedFlag = false;
volatile bool syncRTCFlag = false;

// 双键标志定义
volatile bool doubleButtonPressedFlag = false;
volatile bool button3PressedFlag = false;

// 传感器变量初始化
int turboPressure = 0;
