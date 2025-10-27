// 主程序文件，包含所有库引用和全局变量定义
#ifndef GT86CLOCK_H
#define GT86CLOCK_H

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
extern U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2;

#include <base64.h>
#include <RTClib.h>
extern RTC_DS3231 RTC;


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

extern bool clockRefresh;
extern bool clock24h;
extern bool pm;
extern bool drawDots;
extern bool pressureBar;
extern bool temperatureCelsius;
extern bool readyForModeChange;
extern bool updateCompleteDisplay;

extern float oilPressure;
extern float oilPressureOld;
extern float oilPressureOffset;
extern float oilPressureScalingFactor;

// 添加全局标志位
extern volatile bool button1PressedFlag;
extern volatile bool button2PressedFlag;
extern volatile bool syncRTCFlag;

extern int buttonPin3;
extern int buttonPin3Pressed;
extern volatile bool button3PressedFlag;

// ============================================
// 传感器引脚定义
// ============================================
#define OIL_PRESSURE_PIN    33  // GPIO33 (ADC1_CH5) - 油压传感器
#define OIL_TEMP_PIN        34  // GPIO34 (ADC1_CH6) - 油温传感器
#define TURBO_PRESSURE_PIN  32  // GPIO32 (ADC1_CH4) - 涡轮压力传感器
#define COOLANT_TEMP_PIN    35  // GPIO35 (ADC1_CH7) - 水温传感器

// ============================================
// ADC 采样参数
// ============================================
#define ADC_RESOLUTION      12          // 12位ADC (0-4095)
#define ADC_MAX_VALUE       4095.0      // ADC最大值
#define ADC_VREF            3.3         // ESP32参考电压 3.3V
#define ADC_SAMPLES         10          // 滤波采样次数

// ============================================
// NTC温度传感器校准参数 (油温/水温)
// ============================================
#define NTC_PULLUP_R        470.0       // 上拉电阻 470Ω
#define NTC_R_AT_20C        673.0       // 20℃时阻值 673Ω
#define NTC_R_AT_120C       24.0        // 120℃时阻值 24Ω
#define NTC_BETA            3380.0      // β值 (根据实际传感器调整)
#define NTC_T0_KELVIN       293.15      // 参考温度 20℃ = 293.15K

// ============================================
// 油压传感器校准参数
// ============================================
#define OIL_PRESSURE_PULLUP_R   180.0   // 上拉电阻 180Ω
#define OIL_PRESSURE_R_MIN      10.0    // 0 bar时阻值 10Ω
#define OIL_PRESSURE_R_MAX      184.0   // 10 bar时阻值 184Ω
#define OIL_PRESSURE_MIN        0.0     // 最小压力值 bar
#define OIL_PRESSURE_MAX        10.0    // 最大压力值 bar

// ============================================
// 传感器全局变量
// ============================================
extern int turboPressure;               // 涡轮压力值

#endif
