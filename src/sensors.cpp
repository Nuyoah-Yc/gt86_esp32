// MCP2515 CAN 总线数据读取模块
// 适用于: ESP32 + MCP2515 模块
// 目标车辆: Toyota GT86 / Subaru BRZ / Scion FRS

#include "gt86clock.h"
#include "my_functions.h"
#include <Arduino.h>

// ============================================
// CAN 总线全局变量定义
// ============================================
MCP_CAN CAN0(CAN0_CS);              // CAN 总线对象，使用 CS 引脚初始化
long unsigned int canRxId = 0;       // 接收到的 CAN ID
unsigned char canLen = 0;            // CAN 数据长度
unsigned char canBuf[8] = {0};       // CAN 数据缓冲区

float afr = 14.7;                    // 空燃比 (默认理论空燃比)
float voltage = 12.0;                // 电压值

// ============================================
// CAN 过滤器配置
// ============================================
/**
 * @brief 配置 CAN 过滤器，只接收需要的 CAN ID
 * MCP2515 有 2 个掩码和 6 个过滤器
 */
void setupCANFilters() {
    // 掩码 0 - 应用于过滤器 0 和 1
    CAN0.init_Mask(0, 0, 0x07FF0000);
    CAN0.init_Filt(0, 0, 0x03600000);    // 0x360 - 温度数据
    CAN0.init_Filt(1, 0, 0x01340000);    // 0x134 - AFR

    // 掩码 1 - 应用于过滤器 2-5
    CAN0.init_Mask(1, 0, 0x07FF0000);
    CAN0.init_Filt(2, 0, 0x01420000);    // 0x142 - 电压
    CAN0.init_Filt(3, 0, 0x03600000);    // 0x360 - 温度 (备用)
    CAN0.init_Filt(4, 0, 0x01340000);    // 0x134 - AFR (备用)
    CAN0.init_Filt(5, 0, 0x01420000);    // 0x142 - 电压 (备用)

    // 设置为正常模式开始接收数据
    CAN0.setMode(MCP_NORMAL);

    Serial.println("CAN 过滤器配置完成");
}

// ============================================
// 禁用过滤器（接收所有消息，用于调试）
// ============================================
void disableCANFilters() {
    CAN0.init_Mask(0, 0, 0x00000000);
    CAN0.init_Mask(1, 0, 0x00000000);
    CAN0.setMode(MCP_NORMAL);
    Serial.println("CAN 过滤器已禁用 - 接收所有消息");
}

// ============================================
// CAN 初始化函数
// ============================================
/**
 * @brief 初始化 MCP2515 CAN 总线模块
 * @return true 初始化成功, false 初始化失败
 */
bool setupCAN() {
    Serial.println("正在初始化 MCP2515...");

    // 初始化 MCP2515
    // 参数: MCP_STDEXT (标准帧+扩展帧), CAN_500KBPS (OBD2标准速率), MCP_8MHZ (晶振频率)
    if (CAN0.begin(MCP_STDEXT, CAN_SPEED, CAN_CLOCK) == CAN_OK) {
        Serial.println("MCP2515 初始化成功!");

        // 配置过滤器
        setupCANFilters();

        return true;
    } else {
        Serial.println("MCP2515 初始化失败!");
        return false;
    }
}

// ============================================
// 温度数据解析 (CAN ID: 0x360)
// ============================================
/**
 * @brief 解析温度数据包
 * buf[2] = 机油温度 (原始值 - 40 = 摄氏度)
 * buf[3] = 冷却液温度 (原始值 - 40 = 摄氏度)
 */
void parseTemperature() {
    // 机油温度
    if (canBuf[2] > 0 && canBuf[2] < 200) {
        oilTemp = canBuf[2] - 40;

        #ifdef DEBUG_CAN
        Serial.printf("[CAN] 机油温度: %d°C\n", oilTemp);
        #endif
    }

    // 冷却液温度
    if (canBuf[3] > 0 && canBuf[3] < 200) {
        coolantTemp = canBuf[3] - 40;

        #ifdef DEBUG_CAN
        Serial.printf("[CAN] 冷却液温度: %d°C\n", coolantTemp);
        #endif
    }
}

// ============================================
// 空燃比数据解析 (CAN ID: 0x134)
// ============================================
/**
 * @brief 解析空燃比数据包
 * 通用公式: AFR = (A*256+B)/32768*14.7
 */
void parseAFR() {
    // AFR 解析 (根据实际 ECU 调整公式)
    int rawValue = (canBuf[0] << 8) | canBuf[1];
    if (rawValue > 0) {
        afr = (float)rawValue / 32768.0 * 14.7;

        // 有效范围检查
        if (afr < 8.0 || afr > 22.0) {
            afr = 14.7;  // 超出范围则使用默认值
        }

        #ifdef DEBUG_CAN
        Serial.printf("[CAN] AFR: %.2f\n", afr);
        #endif
    }
}

// ============================================
// 电压数据解析 (CAN ID: 0x142)
// ============================================
/**
 * @brief 解析电压数据包
 */
void parseVoltage() {
    // 电压解析 (根据实际 ECU 调整公式)
    int rawValue = canBuf[0];
    if (rawValue > 0) {
        voltage = (float)rawValue / 10.0;

        // 有效范围检查
        if (voltage < 8.0 || voltage > 16.0) {
            voltage = 12.0;  // 超出范围则使用默认值
        }

        #ifdef DEBUG_CAN
        Serial.printf("[CAN] 电压: %.1fV\n", voltage);
        #endif
    }
}

// ============================================
// 调试输出 - 打印原始 CAN 消息
// ============================================
void printCANMessage() {
    Serial.print("ID: 0x");
    Serial.print(canRxId, HEX);
    Serial.print("  Len: ");
    Serial.print(canLen);
    Serial.print("  Data: ");

    for (int i = 0; i < canLen; i++) {
        if (canBuf[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(canBuf[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// ============================================
// CAN 消息处理主函数
// ============================================
/**
 * @brief 处理接收到的 CAN 消息，根据 ID 分发到对应解析函数
 */
void processCANMessage() {
    switch (canRxId) {
        case CAN_ID_TEMP:       // 0x360 - 温度数据
            parseTemperature();
            break;

        case CAN_ID_AFR:        // 0x134 - 空燃比
            parseAFR();
            break;

        case CAN_ID_VOLTAGE:    // 0x142 - 电压
            parseVoltage();
            break;

        default:
            // 未知 ID，可用于调试
            #ifdef DEBUG_CAN
            Serial.print("[CAN] 未知ID: ");
            printCANMessage();
            #endif
            break;
    }
}

// ============================================
// CAN 数据读取主函数
// ============================================
/**
 * @brief 读取 CAN 总线数据
 * 在主循环中调用此函数以持续读取 CAN 数据
 */
void readCANData() {
    // 检查是否有可用的 CAN 消息
    if (CAN_MSGAVAIL == CAN0.checkReceive()) {
        // 读取消息到缓冲区
        CAN0.readMsgBuf(&canRxId, &canLen, canBuf);

        // 处理接收到的数据
        processCANMessage();

        // 更新时间戳
        lastTempUpdate = millis();
    }
}

// ============================================
// 兼容旧接口的传感器读取函数
// 这些函数保留以兼容 loop.cpp 中的调用
// ============================================

/**
 * @brief 读取传感器1 (兼容接口)
 * 原为油温传感器读取，现通过 CAN 获取
 */
void readSensor1() {
    // CAN 数据由 readCANData() 统一读取
    // 此函数保留以兼容旧代码调用
}

/**
 * @brief 读取传感器2 (兼容接口)
 * 原为水温传感器读取，现通过 CAN 获取
 */
void readSensor2() {
    // CAN 数据由 readCANData() 统一读取
    // 此函数保留以兼容旧代码调用
}

/**
 * @brief 读取传感器3 (兼容接口)
 * 原为油压传感器读取，现通过 CAN 获取
 */
void readSensor3() {
    // CAN 数据由 readCANData() 统一读取
    // 此函数保留以兼容旧代码调用
}
