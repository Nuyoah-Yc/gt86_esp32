#include "my_functions.h"
#include "gt86clock.h"
#include "esp_task_wdt.h"

void setup() {
    esp_task_wdt_init(60, true);
    esp_task_wdt_add(nullptr);
    u8g2.begin();
    u8g2.setFlipMode(1);

    Wire.begin();

    Serial.begin(115200);

    Serial.println("Serial OK");

    // ============================================
    // 配置ADC引脚和参数
    // ============================================
    // ESP32 ADC配置：12位分辨率 (0-4095)
    analogReadResolution(ADC_RESOLUTION);

    // ADC衰减设置：11dB衰减，测量范围 0-3.3V
    // ADC1通道（GPIO32-39）可与WiFi共存
    analogSetAttenuation(ADC_11db);

    // 配置传感器引脚为输入模式
    pinMode(OIL_PRESSURE_PIN, INPUT);    // GPIO33 - 油压传感器
    pinMode(OIL_TEMP_PIN, INPUT);        // GPIO34 - 油温传感器
    pinMode(TURBO_PRESSURE_PIN, INPUT);  // GPIO32 - 涡轮压力传感器
    pinMode(COOLANT_TEMP_PIN, INPUT);    // GPIO35 - 水温传感器

    Serial.println("传感器ADC引脚配置完成");

    // 配置按钮引脚
    pinMode(buttonPin1, INPUT_PULLUP);
    pinMode(buttonPin2, INPUT_PULLUP);
    pinMode(buttonPin3, INPUT_PULLUP);

    Serial.println("PINS1 ok");

    attachInterrupt(digitalPinToInterrupt(buttonPin1), handleInterruptPin1, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonPin2), handleInterruptPin2, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonPin3), handleInterruptPin3, FALLING);

    Serial.println("PINS2 ok");

    clock24h = true;
    modeSaved = CLOCK;

    // 显式初始化 RTC
    if (!RTC.begin()) {
        Serial.println("RTC初始化失败!");
        while (true);
    }
    now = RTC.now();
    Serial.println("RTC初始化完成!");
}