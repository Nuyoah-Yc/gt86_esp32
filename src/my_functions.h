#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H
#include <Arduino.h>

// 中断处理函数
void handleInterruptPin1();

void handleInterruptPin2();

void handleInterruptPin3();

// 工具函数
void customDelay(int ms);

// 添加模式按钮处理函数声明
void handleModeButton();

void handleModeButton3();

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

// ADC传感器读取函数声明（预留给用户开发）
void readSensor1(); // 油温传感器读取
void readSensor2(); // 水温传感器读取
void readSensor3(); // 预留传感器读取
