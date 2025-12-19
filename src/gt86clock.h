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
#include <mcp_can.h>
#include <SPI.h>

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
// MCP2515 CAN 总线引脚定义
// ============================================
#define CAN0_CS     15      // GPIO15 - MCP2515 片选引脚 (CS)
#define CAN0_INT    4       // GPIO4  - MCP2515 中断引脚 (INT)
// SPI引脚使用ESP32 HSPI默认引脚:
// MOSI: GPIO13 -> MCP2515 SI
// MISO: GPIO12 -> MCP2515 SO
// SCK:  GPIO14 -> MCP2515 SCK

// ============================================
// CAN 总线参数
// ============================================
#define CAN_SPEED       CAN_500KBPS     // OBD2 标准速率 500Kbps
#define CAN_CLOCK       MCP_8MHZ        // MCP2515 晶振频率 8MHz

// GT86/BRZ/FRS CAN ID 定义
#define CAN_ID_TEMP     0x360           // 机油温度 / 冷却液温度
#define CAN_ID_AFR      0x134           // 空燃比 (AFR)

// ============================================
// CAN 数据全局变量
// ============================================
extern MCP_CAN CAN0;                    // CAN 总线对象
extern long unsigned int canRxId;       // 接收到的 CAN ID
extern unsigned char canLen;            // CAN 数据长度
extern unsigned char canBuf[8];         // CAN 数据缓冲区

extern float afr;                       // 空燃比

#endif
