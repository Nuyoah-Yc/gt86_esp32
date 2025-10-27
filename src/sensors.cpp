// 传感器读取和处理函数
#include "gt86clock.h"
#include "my_functions.h"
#include <Arduino.h>

// ============================================
// ADC读取函数（带移动平均滤波）
// ============================================
/**
 * @brief 读取ADC值并进行滤波
 * @param pin ADC引脚编号
 * @param samples 采样次数（默认10次）
 * @return 滤波后的ADC平均值 (0-4095)
 */
int readADC_Filtered(int pin, int samples = ADC_SAMPLES) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delayMicroseconds(100); // 短暂延时避免ADC过快采样
    }
    return sum / samples;
}

// ============================================
// ADC值转电压
// ============================================
/**
 * @brief 将ADC值转换为电压
 * @param adcValue ADC原始值 (0-4095)
 * @return 电压值 (V)
 */
float adcToVoltage(int adcValue) {
    return (adcValue / ADC_MAX_VALUE) * ADC_VREF;
}

// ============================================
// 电压转NTC电阻值
// ============================================
/**
 * @brief 根据分压电路计算NTC阻值
 * 电路: 3.3V -> R_pullup -> [Vout测量点] -> R_NTC -> GND
 * 分压公式: Vout = 3.3V × R_NTC / (R_pullup + R_NTC)
 * 反推: R_NTC = Vout × R_pullup / (3.3 - Vout)
 *
 * @param voltage 测量电压 (V)
 * @param pullupR 上拉电阻 (Ω)
 * @return NTC阻值 (Ω)
 */
float voltageToNTC_Resistance(float voltage, float pullupR) {
    // 防止除零错误
    if (voltage >= ADC_VREF - 0.01) {
        voltage = ADC_VREF - 0.01;
    }
    if (voltage <= 0.01) {
        voltage = 0.01;
    }

    return (voltage * pullupR) / (ADC_VREF - voltage);
}

// ============================================
// NTC电阻值转温度（Steinhart-Hart简化方程）
// ============================================
/**
 * @brief 使用β值方程将NTC阻值转换为温度
 * 公式: 1/T = 1/T0 + (1/β) × ln(R/R0)
 * 其中: T0 = 293.15K (20℃), R0 = 673Ω
 *
 * @param resistance NTC阻值 (Ω)
 * @return 温度 (℃)
 */
int ntcResistanceToTemperature(float resistance) {
    // 限制阻值范围，避免异常值
    if (resistance < 10.0) resistance = 10.0;
    if (resistance > 10000.0) resistance = 10000.0;

    // β值方程: 1/T = 1/T0 + (1/β) × ln(R/R0)
    float lnRatio = log(resistance / NTC_R_AT_20C);
    float tempKelvin = 1.0 / (1.0 / NTC_T0_KELVIN + lnRatio / NTC_BETA);

    // 转换为摄氏度
    int tempCelsius = (int)round(tempKelvin - 273.15);

    // 合理性检查：温度范围 -40℃ ~ 150℃
    if (tempCelsius < -40) tempCelsius = -40;
    if (tempCelsius > 150) tempCelsius = 150;

    return tempCelsius;
}

// ============================================
// 电压转油压传感器电阻值
// ============================================
/**
 * @brief 根据分压电路计算油压传感器阻值
 * @param voltage 测量电压 (V)
 * @return 传感器阻值 (Ω)
 */
float voltageToOilPressure_Resistance(float voltage) {
    // 防止除零错误
    if (voltage >= ADC_VREF - 0.01) {
        voltage = ADC_VREF - 0.01;
    }
    if (voltage <= 0.01) {
        voltage = 0.01;
    }

    return (voltage * OIL_PRESSURE_PULLUP_R) / (ADC_VREF - voltage);
}

// ============================================
// 电阻值转油压值
// ============================================
/**
 * @brief 将传感器阻值线性映射到压力值
 * 映射关系: 10Ω -> 0 bar, 184Ω -> 10 bar
 * 公式: pressure = (R - R_min) / (R_max - R_min) × P_max
 *
 * @param resistance 传感器阻值 (Ω)
 * @return 油压值 (bar, 保留1位小数)
 */
float oilPressureResistanceToPressure(float resistance) {
    // 线性映射
    float pressure = (resistance - OIL_PRESSURE_R_MIN) /
                     (OIL_PRESSURE_R_MAX - OIL_PRESSURE_R_MIN) *
                     OIL_PRESSURE_MAX;

    // 限制范围
    if (pressure < 0.0) pressure = 0.0;
    if (pressure > OIL_PRESSURE_MAX) pressure = OIL_PRESSURE_MAX;

    // 保留1位小数
    pressure = round(pressure * 10.0) / 10.0;

    return pressure;
}

// ============================================
// 传感器读取主函数
// ============================================

/**
 * @brief 读取油温传感器 (NTC)
 * 连接: GPIO 34 (ADC1_CH6)
 * 更新全局变量: oilTemp
 */
void readSensor1() {
    static unsigned long lastUpdate = 0;

    // 500ms更新一次
    if (millis() - lastUpdate >= 500) {
        // 读取ADC值（滤波）
        int adcValue = readADC_Filtered(OIL_TEMP_PIN);

        // 转换为电压
        float voltage = adcToVoltage(adcValue);

        // 电压 -> NTC阻值
        float resistance = voltageToNTC_Resistance(voltage, NTC_PULLUP_R);

        // NTC阻值 -> 温度
        oilTemp = ntcResistanceToTemperature(resistance);

        // 调试输出（可选）
        #ifdef DEBUG_SENSORS
        Serial.printf("[油温] ADC=%d, V=%.2fV, R=%.1fΩ, T=%d℃\n",
                      adcValue, voltage, resistance, oilTemp);
        #endif

        lastTempUpdate = millis();
        lastUpdate = millis();
    }
}

/**
 * @brief 读取水温传感器 (NTC)
 * 连接: GPIO 35 (ADC1_CH7)
 * 更新全局变量: coolantTemp
 */
void readSensor2() {
    static unsigned long lastUpdate = 0;

    // 500ms更新一次
    if (millis() - lastUpdate >= 500) {
        // 读取ADC值（滤波）
        int adcValue = readADC_Filtered(COOLANT_TEMP_PIN);

        // 转换为电压
        float voltage = adcToVoltage(adcValue);

        // 电压 -> NTC阻值
        float resistance = voltageToNTC_Resistance(voltage, NTC_PULLUP_R);

        // NTC阻值 -> 温度
        coolantTemp = ntcResistanceToTemperature(resistance);

        // 调试输出（可选）
        #ifdef DEBUG_SENSORS
        Serial.printf("[水温] ADC=%d, V=%.2fV, R=%.1fΩ, T=%d℃\n",
                      adcValue, voltage, resistance, coolantTemp);
        #endif

        lastTempUpdate = millis();
        lastUpdate = millis();
    }
}

/**
 * @brief 读取油压传感器 (电阻式)
 * 连接: GPIO 33 (ADC1_CH5)
 * 更新全局变量: oilPressure
 */
void readSensor3() {
    static unsigned long lastUpdate = 0;

    // 500ms更新一次
    if (millis() - lastUpdate >= 500) {
        // 读取ADC值（滤波）
        int adcValue = readADC_Filtered(OIL_PRESSURE_PIN);

        // 转换为电压
        float voltage = adcToVoltage(adcValue);

        // 电压 -> 传感器阻值
        float resistance = voltageToOilPressure_Resistance(voltage);

        // 阻值 -> 压力值
        oilPressure = oilPressureResistanceToPressure(resistance);

        // 限制显示范围（单位数显示）
        if (oilPressure >= 10.0) oilPressure = 9.9;

        // 调试输出（可选）
        #ifdef DEBUG_SENSORS
        Serial.printf("[油压] ADC=%d, V=%.2fV, R=%.1fΩ, P=%.1f bar\n",
                      adcValue, voltage, resistance, oilPressure);
        #endif

        lastUpdate = millis();
    }
}

/**
 * @brief 读取涡轮压力传感器 (预留接口)
 * 连接: GPIO 32 (ADC1_CH4)
 * 更新全局变量: turboPressure
 *
 * 注: 涡轮压力传感器的具体实现取决于传感器类型
 *     这里提供一个基本框架，需要根据实际传感器调整
 */
void readTurboPressure() {
    static unsigned long lastUpdate = 0;

    // 500ms更新一次
    if (millis() - lastUpdate >= 500) {
        // 读取ADC值（滤波）
        int adcValue = readADC_Filtered(TURBO_PRESSURE_PIN);

        // 转换为电压
        float voltage = adcToVoltage(adcValue);

        // TODO: 根据实际涡轮压力传感器特性实现转换
        // 这里仅作示例：假设线性映射 0-3.3V -> 0-200 kPa
        turboPressure = (int)(voltage / ADC_VREF * 200.0);

        // 调试输出（可选）
        #ifdef DEBUG_SENSORS
        Serial.printf("[涡轮] ADC=%d, V=%.2fV, P=%d kPa\n",
                      adcValue, voltage, turboPressure);
        #endif

        lastUpdate = millis();
    }
}
