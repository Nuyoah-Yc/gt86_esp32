// 主循环代码
#include "gt86clock.h"
#include "my_functions.h"
#include "esp_task_wdt.h"

void loop() {
    esp_task_wdt_reset();

    // --- 按钮1：切换显示模式 ---
    if (button1PressedFlag) {
        button1PressedFlag = false;
        modeCurrent = (modeCurrent % (MAXSCREENS - 1)) + 1;
        clockRefresh = true;
        Serial.printf("[DEBUG] 切换模式 -> %d\n", modeCurrent);
    }

    // --- 按钮2：调小时（仅在 CLOCK 模式） ---
    if (button2PressedFlag) {
        button2PressedFlag = false;
        if (modeCurrent == CLOCK) {
            clockHour = (clockHour + 1) % 24;
            const DateTime nowTime = RTC.now();
            RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(),
                                clockHour, clockMinute, 0));
            clockRefresh = true;
            Serial.printf("[DEBUG] 调整小时 -> %02d:%02d\n", clockHour, clockMinute);
        }
    }

    // --- 按钮3：调分钟（仅在 CLOCK 模式） ---
    if (button3PressedFlag) {
        button3PressedFlag = false;
        if (modeCurrent == CLOCK) {
            clockMinute = (clockMinute + 1) % 60;
            const DateTime nowTime = RTC.now();
            RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(),
                                clockHour, clockMinute, 0));
            clockRefresh = true;
            Serial.printf("[DEBUG] 调整分钟 -> %02d:%02d\n", clockHour, clockMinute);
        }
    }

    // --- RTC 时间更新 ---
    now = RTC.now();
    if (modeCurrent == CLOCK &&
        (clockHour != now.hour() || clockMinute != now.minute())) {
        clockHour = now.hour();
        clockMinute = now.minute();
        clockRefresh = true;
    }

    // --- 调用传感器更新 ---
    readSensor1();
    readSensor2();
    readSensor3();

    // --- 显示刷新逻辑 ---
    if (modeOld != modeCurrent || clockRefresh) {
        updateCompleteDisplay = true;
        modeOld = modeCurrent;
        clockRefresh = false;
    } else {
        updateCompleteDisplay = false;
    }

    switch (modeCurrent) {
        case LOGO:          drawLogo(updateCompleteDisplay); break;
        case CLOCK:         drawClock(updateCompleteDisplay); break;
        case OILTEMP:       drawOilTemp(updateCompleteDisplay); break;
        case COOLANTTEMP:   drawCoolantTemp(updateCompleteDisplay); break;
        case OILCOOLANTTEMP:drawCoolantOilTemp(updateCompleteDisplay); break;
        case OILPRESSURE:   drawOilPressure(updateCompleteDisplay); break;
        default:            drawLogo(updateCompleteDisplay);
    }
}

void customDelay(int reqDelay)
{
    unsigned long startTime = millis();

    while (millis() - startTime < reqDelay) {
        if (modeOld != modeCurrent)
            break;

        esp_task_wdt_reset();
        delay(50);
    }
}

// ============================================
// 注意：传感器读取函数 readSensor1/2/3 的实现
// 已移至 sensors.cpp 文件中，使用真实的ADC采样
// ============================================
