// 调用其他的void()函数
#ifndef MY_FUNCTIONS_H // 防止重复包含
#define MY_FUNCTIONS_H
#include <Arduino.h>

// NTP同步函数已移除
// 中断处理函数
void handleInterruptPin1();

void handleInterruptPin2();

void handleInterruptPin3();

// 配置和工具函数
void readConfig();

void customDelay(int ms);

void writeConfig();

// 添加模式按钮处理函数声明
void handleModeButton();

void handleModeButton3();

// Web服务器处理函数已移除

extern volatile bool doubleButtonPressedFlag;

#endif

int countDigits(int value);

void drawTwoValues(int w1, int h1, const unsigned char *bits1,
                   const char *val1, int digits1, const char *suffix1,
                   int w2, int h2, const unsigned char *bits2,
                   const char *val2, int digits2, const char *suffix2,
                   bool updateCompleteDisplay);

void drawSingleValue(int iconWidth, int iconHeight, const unsigned char *iconBits,
                     const char *value, int valueLength, const char *unit,
                     bool updateCompleteDisplay);

// 绘图函数声明
void drawLogo(bool updateDisplay);

void drawClock(bool updateDisplay);

void drawOilTemp(bool updateDisplay);

void drawCoolantTemp(bool updateDisplay);

void drawCoolantOilTemp(bool updateDisplay);

void drawOilPressure(bool updateDisplay);

// AFR/电压显示和设置界面绘制函数已移除

// JSON函数已移除

// readConfig() 函数前添加函数声明
int eepromReadInt(int adr);

void eepromWriteInt(int adr, int wert);

// ADC传感器读取函数声明（预留给用户开发）
void readSensor1(); // 油温传感器读取
void readSensor2(); // 水温传感器读取
void readSensor3(); // 预留传感器读取
