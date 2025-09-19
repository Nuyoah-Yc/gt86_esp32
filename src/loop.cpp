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
        afr = 14.0 + 0.5 * sin(t / 5.0); // 空燃比 13.5~14.5
        voltage = 12.0 + 0.5 * cos(t / 7.0); // 电压 11.5~12.5
        lastUpdate = millis();
    }
}

void loop(void) {
    // 看门狗喂狗操作（防止系统复位）
    esp_task_wdt_reset();
    // 优先处理双键
    if (doubleButtonPressedFlag) {
        handleModeButton();
        doubleButtonPressedFlag = false;
        button1PressedFlag = false;
        button2PressedFlag = false;
    } else {
        // 独立处理按钮1事件
        if (button1PressedFlag) {
            handleModeButton(); // 直接处理按钮逻辑
            button1PressedFlag = false;
        }
        // 独立处理按钮2事件
        if (button2PressedFlag) {
            handleModeButton3(); // 直接处理按钮逻辑
            button2PressedFlag = false;
        }
        // 独立处理按钮3事件
        if (button3PressedFlag) {
            handleModeButton3(); // 处理第三个按钮逻辑
            button3PressedFlag = false;
        }
    }

    // 实时时钟更新
    now = RTC.now();

    // 网络服务处理
    server.handleClient(); // 处理Web客户端请求
    wifiManager.process(); // WiFi连接维护

    // RTC时间同步触发（来自设置界面）
    if (syncRTCFlag) {
        // 使用当前存储的小时/分钟更新RTC
        DateTime nowTime = RTC.now();
        RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(), clockHour, clockMinute, 0));
        syncRTCFlag = false; // 清除同步标志
        Serial.println("[DEBUG] RTC时间已同步");
        Serial.printf("RTC时间: %02d:%02d\n", now.hour(), now.minute());
    }

    if (saveConfigFlag) {
        writeConfig(); // 写入EEPROM
        saveConfigFlag = false; // 清除保存标志
        Serial.println("[DEBUG] 配置已保存");
    }

    // NTP网络时间同步触发
    if (syncNTPFlag) {
        syncNTP(); // 执行NTP同步（需确保非阻塞）
        syncNTPFlag = false; // 清除同步标志
    }

    // ADC传感器数据读取（替代CAN总线）
    readSensor1(); // 读取传感器1（油温）
    readSensor2(); // 读取传感器2（水温）
    readSensor3(); // 读取传感器3（油压）

    // // 模拟数据方案（示例）：
    // oilPressureOld = 500; // 固定模拟值（范围根据传感器校准值调整）
    // // 或动态模拟（如正弦波动）：
    // static unsigned long lastTime = 0;
    // if (millis() - lastTime > 1000)
    // {                                                        // 每秒变化一次
    //     oilPressureOld = 300 + 200 * sin(millis() / 1000.0); // 范围 100~500
    //     lastTime = millis();
    // }

    // 读取A0引脚模拟值（油压传感器原始数据）
    oilPressureOld = analogRead(A0);
    // 原始值下限保护（防止负压值）
    if (oilPressureOld < oilPressureOffset) {
        oilPressureOld = oilPressureOffset;
    }
    // 油压值转换公式：(原始值 - 零点偏移) × 比例系数
    oilPressure = float((oilPressureOld - oilPressureOffset) * oilPressureScalingFactor);

    // 固定使用bar（公制单位）处理
    // PSI转BAR公式：1 psi = 0.0689476 bar
    oilPressure = oilPressure * 0.0689476;
    // 保留1位小数（四舍五入）
    oilPressure = float(round(oilPressure * 10)) / 10;
    // BAR单位最大值限制（9.9 bar）
    if (oilPressure >= 10) {
        oilPressure = 9.9;
    }

    if (modeCurrent == CLOCK && (clockHour != now.hour() || clockMinute != now.minute())) {
        clockHour = now.hour();
        clockMinute = now.minute();

        // 小时有效性检查（>24时归零）
        if (clockHour > 24 && clockMinute > 0) {
            clockHour = 0;
            DateTime nowTime = RTC.now();
            RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(), clockHour, clockMinute, 0));
        }

        // 分钟有效性检查（>60分归零）
        if (clockMinute > 60) {
            clockMinute = 0;
            DateTime nowTime = RTC.now();
            RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(), clockHour, clockMinute, 0));
        }

        // 标记需要刷新时钟显示
        clockRefresh = true;
    }

    // no dot, if we have recent data
    // 温度数据新鲜度指示器（10秒有效期）
    if (millis() - lastTempUpdate < 10 * 1000) {
        u8g2.setDrawColor(0); // 使用背景色绘制（擦除模式）
        u8g2.drawCircle(1, 1, 1); // 在左上角绘制小圆点指示器
        u8g2.setDrawColor(1); // 恢复前景色
    } else
        u8g2.drawCircle(1, 1, 1);

    // check for complete refresh or just updated values
    // 检查显示刷新需求：模式切换或时钟更新时需要完全刷新
    if (modeOld != modeCurrent || clockRefresh == true) {
        updateCompleteDisplay = true; // 标记需要全屏刷新
        modeOld = modeCurrent; // 同步模式状态
    } else
        updateCompleteDisplay = false; // 局部更新即可

    // 根据当前模式刷新OLED显示
    switch (modeCurrent) {
        case LOGO: // 品牌LOGO界面
            drawLogo(updateCompleteDisplay);
            break;
        case CLOCK: // 主时钟界面
            drawClock(updateCompleteDisplay);
            break;
        case OILTEMP: // 油温单显界面
            drawOilTemp(updateCompleteDisplay);
            break;
        case COOLANTTEMP: // 水温单显界面
            drawCoolantTemp(updateCompleteDisplay);
            break;
        case OILCOOLANTTEMP: // 油温水温双显界面
            drawCoolantOilTemp(updateCompleteDisplay);
            break;
        case OILPRESSURE: // 油压显示界面
            drawOilPressure(updateCompleteDisplay);
            break;
        case O2: // 空燃比/电压双显界面
            drawAfrAndVoltage(updateCompleteDisplay);
            break;
        case SETTINGSWIFI: // WiFi设置界面
            drawSettingsWifi(updateCompleteDisplay);
            break;
        case SETTINGSCLOCK: // 时钟设置界面
            drawSettingsClock(updateCompleteDisplay);
            break;
        case SETTINGSUNITS: // 单位设置界面
            drawSettingsUnits(updateCompleteDisplay);
            break;
        case SETTINGSO2: // 空燃比显示模式设置界面
            drawSettingsO2(updateCompleteDisplay);
            break;
        default: // 异常情况默认显示LOGO
            drawLogo(updateCompleteDisplay);
    }

    // 每10秒追加一次传感器数据到JSON文件
    if (millis() - lastJsonAppend > 10000) {
        appendJsonFile(jsonFile); // 执行数据追加
        lastJsonAppend = millis(); // 重置计时器
    }

    readyForModeChange = true; // 允许进行下一次模式切换
    updateSimulatedSensorData(); // 每次循环都更新模拟数据
}

// 自定义非阻塞延时函数，用于模式切换时的延迟
void customDelay(int reqDelay) // 参数：reqDelay-需要延时的毫秒数
{
    unsigned long startTime = millis(); // 记录延时开始时间

    // 非阻塞延时循环（保持Web服务器响应能力）
    while (millis() - startTime < reqDelay) {
        server.handleClient(); // 处理Web客户端请求
        if (modeOld != modeCurrent) // 检测模式切换请求
            break; // 立即退出延时循环

        esp_task_wdt_reset(); // 喂狗（维护看门狗）
        delay(50);
    }
}

// ADC传感器读取方法1 - 油温传感器
// 预留给用户后续开发，可连接到任意ADC引脚
void readSensor1() {
    // TODO: 用户自定义油温传感器读取逻辑
    // 示例：
    // int adcValue = analogRead(A1); // 假设连接到A1引脚
    // oilTemp = mapAdcToTemperature(adcValue);
    // lastTempUpdate = millis();

    // 调试输出
    Serial.println("[DEBUG] readSensor1() - 油温传感器读取（待用户实现）");
}

// ADC传感器读取方法2 - 水温传感器
// 预留给用户后续开发，可连接到任意ADC引脚
void readSensor2() {
    // TODO: 用户自定义水温传感器读取逻辑
    // 示例：
    // int adcValue = analogRead(A2); // 假设连接到A2引脚
    // coolantTemp = mapAdcToTemperature(adcValue);
    // lastTempUpdate = millis();

    // 调试输出
    Serial.println("[DEBUG] readSensor2() - 水温传感器读取（待用户实现）");
}

// ADC传感器读取方法3 - 空燃比/电压传感器
// 预留给用户后续开发，可连接到任意ADC引脚
void readSensor3() {
    // TODO: 用户自定义空燃比/电压传感器读取逻辑
    // 示例：
    // int adcValue = analogRead(A3); // 假设连接到A3引脚
    // afr = mapAdcToAfr(adcValue);
    // voltage = mapAdcToVoltage(adcValue);

    // 调试输出
    Serial.println("[DEBUG] readSensor3() - 空燃比/电压传感器读取（待用户实现）");
}
