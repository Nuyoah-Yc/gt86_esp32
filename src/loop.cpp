// 主循环代码
#include "gt86clock.h"
#include "my_functions.h"
#include "esp_task_wdt.h"

// 模拟数据生成函数 测试
void updateSimulatedSensorData() {
    static unsigned long lastUpdate = 0;
    static float t = 0;
    if (millis() - lastUpdate > 500) {
        // 每0.5秒更新一次
        t += 0.5;
        oilTemp = 70 + 20 * sin(t / 10.0); // 油温 70~90
        coolantTemp = 80 + 10 * cos(t / 8.0); // 水温 70~90
        oilPressure = 2.0 + 1.5 * fabs(sin(t / 6.0)); // 油压 2.0~3.5
        // afr和voltage模拟数据已移除
        lastUpdate = millis();
    }
}

void loop(void) {
    esp_task_wdt_reset();

    // --- 按钮1：切换显示模式 ---
    if (button1PressedFlag) {
        button1PressedFlag = false;
        modeCurrent = (modeCurrent % (MAXSCREENS - 1)) + 1; // 循环 1~5
        saveConfigFlag = true; // 保存模式到EEPROM
        clockRefresh = true;
        Serial.printf("[DEBUG] 切换模式 -> %d\n", modeCurrent);
    }

    // --- 按钮2：调小时（仅在 CLOCK 模式） ---
    if (button2PressedFlag) {
        button2PressedFlag = false;
        if (modeCurrent == CLOCK) {
            clockHour = (clockHour + 1) % 24;
            DateTime nowTime = RTC.now();
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
            DateTime nowTime = RTC.now();
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


// 自定义非阻塞延时函数，用于模式切换时的延迟
void customDelay(int reqDelay) // 参数：reqDelay-需要延时的毫秒数
{
    unsigned long startTime = millis(); // 记录延时开始时间

    // 非阻塞延时循环（Web服务器处理已移除）
    while (millis() - startTime < reqDelay) {
        if (modeOld != modeCurrent) // 检测模式切换请求
            break; // 立即退出延时循环

        esp_task_wdt_reset(); // 喂狗（维护看门狗）
        delay(50);
    }
}

// 0.5s 更新一次模拟数据
void readSensor1() { // 油温
    static unsigned long lastUpdate = 0;
    static float t = 0;
    if (millis() - lastUpdate >= 500) {
        t += 0.5;
        oilTemp = 70 + 20 * sin(t / 10.0); // 70~90℃ 波动
        lastTempUpdate = millis();
        lastUpdate = millis();
    }
}

void readSensor2() { // 水温
    static unsigned long lastUpdate = 0;
    static float t = 0;
    if (millis() - lastUpdate >= 500) {
        t += 0.5;
        coolantTemp = 80 + 10 * cos(t / 8.0); // 70~90℃ 波动
        lastTempUpdate = millis();
        lastUpdate = millis();
    }
}

void readSensor3() { // 油压
    static unsigned long lastUpdate = 0;
    static float t = 0;
    if (millis() - lastUpdate >= 500) {
        t += 0.5;
        oilPressure = 2.0 + 1.5 * fabs(sin(t / 6.0)); // 2.0~3.5 bar
        // 保留1位小数
        oilPressure = float(round(oilPressure * 10)) / 10;
        if (oilPressure >= 10) oilPressure = 9.9;
        lastUpdate = millis();
    }
}
