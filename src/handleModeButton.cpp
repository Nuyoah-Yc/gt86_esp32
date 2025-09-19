// 按钮模式处理逻辑
#include "my_functions.h"
#include "gt86clock.h"
extern volatile bool doubleButtonPressedFlag;
// 功能：处理短按/长按操作，实现时间调整/单位切换/模式循环等功能
void handleModeButton() {
    // 2号引脚（buttonPin1）短按：切换模式
    if (buttonPin1Pressed != -1 && millis() - buttonPin1Pressed > 20 && millis() - buttonPin1Pressed < 500) {
        modeCurrent = (modeCurrent % (MAXSCREENS - 3)) + 1; // 模式循环 1-（MAXSCREENS-3）
        saveConfigFlag = true;
        buttonPin1Pressed = -1;
        lastModeChange = millis();
        return;
    }
    // 2号引脚（buttonPin1）长按：根据当前模式执行特定功能（移除了单位切换）
    if (buttonPin1Pressed != -1 && millis() - buttonPin1Pressed >= 500 && millis() - buttonPin1Pressed < 5000) {
        Serial.print("[DEBUG] 长按2号引脚，当前modeCurrent: ");
        Serial.println(modeCurrent);
        if (modeCurrent == CLOCK) {
            syncRTCFlag = true;
            Serial.println("[DEBUG] 触发NTP时间同步");
        } else if (modeCurrent == O2) {
            o2afr = !o2afr;
            modeOld = 0;
            saveConfigFlag = true;
            Serial.print("[DEBUG] 切换空燃比显示模式，当前o2afr: ");
            Serial.println(o2afr ? "AFR" : "Lambda");
        } else {
            Serial.println("[DEBUG] 长按无功能（单位已固定为公制）");
        }
        buttonPin1Pressed = -1;
        lastModeChange = millis();
        return;
    }
}

// 34号引脚（buttonPin2）短按：调整小时（仅时钟模式）
// 35号引脚（buttonPin3）短按：调整分钟（仅时钟模式）
void handleModeButton3() {
    // 34号按钮调小时
    if (buttonPin2Pressed != -1 && millis() - buttonPin2Pressed > 20 && millis() - buttonPin2Pressed < 500 &&
        modeCurrent == CLOCK && !clockRefresh) {
        clockHour = (clockHour + 1) % 24;
        DateTime nowTime = RTC.now();
        RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(), clockHour, clockMinute, 0));
        syncRTCFlag = true;
        clockRefresh = true;
        buttonPin2Pressed = -1;
        return;
    }
    // 35号按钮调分钟
    if (buttonPin3Pressed != -1 && millis() - buttonPin3Pressed > 20 && millis() - buttonPin3Pressed < 500 &&
        modeCurrent == CLOCK && !clockRefresh) {
        clockMinute = (clockMinute + 1) % 60;
        DateTime nowTime = RTC.now();
        RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(), clockHour, clockMinute, 0));
        syncRTCFlag = true;
        clockRefresh = true;
        buttonPin3Pressed = -1;
        return;
    }
}
