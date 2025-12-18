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

// ============================================
// MCP2515 CAN 总线函数声明
// ============================================
bool setupCAN();            // CAN 总线初始化
void setupCANFilters();     // 配置 CAN 过滤器
void disableCANFilters();   // 禁用过滤器 (调试用)
void readCANData();         // 读取 CAN 数据 (主循环调用)
void processCANMessage();   // 处理 CAN 消息
void printCANMessage();     // 打印 CAN 消息 (调试用)

// CAN 数据解析函数
void parseTemperature();    // 解析温度数据 (0x360)
void parseAFR();            // 解析空燃比数据 (0x134)
void parseVoltage();        // 解析电压数据 (0x142)

// 兼容旧接口的传感器读取函数 (保留但为空实现)
void readSensor1();
void readSensor2();
void readSensor3();
