// 主循环代码
#include "gt86clock.h"
#include "my_functions.h"
#include "esp_task_wdt.h"

// 模拟数据生成函数 测试
void updateSimulatedSensorData()
{
    static unsigned long lastUpdate = 0;
    static float t = 0;
    if (millis() - lastUpdate > 500)
    { // 每0.5秒更新一次
        t += 0.5;
        oilTemp = 70 + 20 * sin(t / 10.0);            // 油温 70~90
        coolantTemp = 80 + 10 * cos(t / 8.0);         // 水温 70~90
        oilPressure = 2.0 + 1.5 * fabs(sin(t / 6.0)); // 油压 2.0~3.5
        afr = 14.0 + 0.5 * sin(t / 5.0);              // 空燃比 13.5~14.5
        voltage = 12.0 + 0.5 * cos(t / 7.0);          // 电压 11.5~12.5
        lastUpdate = millis();
    }
}

void loop(void)
{
    // 看门狗喂狗操作（防止系统复位）
    esp_task_wdt_reset();
    // 优先处理双键
    if (doubleButtonPressedFlag)
    {
        handleModeButton();
        doubleButtonPressedFlag = false;
        button1PressedFlag = false;
        button2PressedFlag = false;
    }
    else
    {
        // 独立处理按钮1事件
        if (button1PressedFlag)
        {
            handleModeButton(); // 直接处理按钮逻辑
            button1PressedFlag = false;
        }
        // 独立处理按钮2事件
        if (button2PressedFlag)
        {
            handleModeButton3(); // 直接处理按钮逻辑
            button2PressedFlag = false;
        }
        // 独立处理按钮3事件
        if (button3PressedFlag)
        {
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
    if (syncRTCFlag)
    {
        // 使用当前存储的小时/分钟更新RTC
        DateTime nowTime = RTC.now();
        RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(), clockHour, clockMinute, 0));
        syncRTCFlag = false; // 清除同步标志
        Serial.println("[DEBUG] RTC时间已同步");
        Serial.printf("RTC时间: %02d:%02d\n", now.hour(), now.minute());
    }

    if (saveConfigFlag)
    {
        writeConfig();          // 写入EEPROM
        saveConfigFlag = false; // 清除保存标志
        Serial.println("[DEBUG] 配置已保存");
    }

    // NTP网络时间同步触发
    if (syncNTPFlag)
    {
        syncNTP();           // 执行NTP同步（需确保非阻塞）
        syncNTPFlag = false; // 清除同步标志
    }

    // CAN总线数据处理（来自汽车ECU）
    if (CAN_MSGAVAIL == CAN0.checkReceive())
    {
        Serial.println("[DEBUG] CAN数据可用");
        CAN0.readMsgBuf(&rxId, &len, buf); // 读取CAN数据

        // 0x360报文：油温/水温数据
        if (rxId == 0x360)
        {
            // 油温处理（原始值-40得到实际温度）
            if (buf[2] > 0 && buf[2] < 200)
            {
                oilTemp = buf[2] - 40;
                // 华氏度转换（需要时）
                if (!temperatureCelsius)
                    oilTemp = round(oilTemp * 1.8 + 32);
                lastTempUpdate = millis(); // 记录最后更新时间
            }

            // 水温处理（同油温逻辑）
            if (buf[3] > 0 && buf[3] < 200)
            {
                coolantTemp = buf[3] - 40;
                if (!temperatureCelsius)
                    coolantTemp = round(coolantTemp * 1.8 + 32);
                lastTempUpdate = millis();
            }
        }
        // 0x134报文：空燃比数据
        else if (rxId == 0x134)
        {
            Serial.print("ID: ");
            Serial.print(rxId, HEX);
            Serial.print("  Data: ");
            for (int i = 0; i < len; i++)
            {
                if (buf[i] < 0x10)
                    Serial.print("0");
                Serial.print(buf[i], HEX);
                Serial.print(" ");
            }

            // 空燃比模式处理
            if (o2afr) // AFR模式
            {
                if (afr < -40 || afr > 320) // 合理性检查
                    afr = 999;
            }
            else // O2模式
            {
                // 转换为lambda值（14.7为基准）
                afr = float(round((afr / 14.7) * 100)) / 100;
                if (afr < -0.2 || afr > 2) // 范围检查
                    afr = -1;
            }
        }
        // 0x142报文：电压数据
        else if (rxId == 0x142) // 电压报文处理
        {
            // 原始数据打印（调试用）
            Serial.print("ID: ");
            Serial.print(rxId, HEX);
            Serial.print("  Data: ");
            // 遍历数据字节（十六进制格式输出）
            for (int i = 0; i < len; i++)
            {
                // 单字节补零（保持两位十六进制显示）
                if (buf[i] < 0x10)
                {
                    Serial.print("0");
                }
                Serial.print(buf[i], HEX);
                Serial.print(" ");
            }
            Serial.println();

            // 关键字节调试输出
            Serial.print("buf[3]: "); // 电压高字节
            Serial.print(buf[3]);
            Serial.print(" buf[4]: "); // 电压低字节
            Serial.println(buf[4]);

            // 电压计算公式：（高字节 * 256 + 低字节）/ 1000
            voltage = (buf[3] * 256 + buf[4]) / 1000;
            Serial.print("Voltage: ");
            Serial.println(voltage);

            // 精度处理（保留1位小数）
            voltage = float(round(voltage * 10)) / 10;

            // 合理性检查（5-16V有效范围）
            if (voltage < 5 || voltage > 16)
                voltage = 0; // 异常值清零
        }

        // 在耗时操作后再次喂狗
        esp_task_wdt_reset();
    }

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
    if (oilPressureOld < oilPressureOffset)
    {
        oilPressureOld = oilPressureOffset;
    }
    // 油压值转换公式：(原始值 - 零点偏移) × 比例系数
    oilPressure = float((oilPressureOld - oilPressureOffset) * oilPressureScalingFactor);

    // 压力单位处理（bar/psi 自动转换）
    if (pressureBar)
    {
        // PSI转BAR公式：1 psi = 0.0689476 bar
        oilPressure = oilPressure * 0.0689476;
        // 保留1位小数（四舍五入）
        oilPressure = float(round(oilPressure * 10)) / 10;
        // BAR单位最大值限制（9.9 bar）
        if (oilPressure >= 10)
        {
            oilPressure = 9.9;
        }
    }
    else
    {
        // PSI模式直接取整
        oilPressure = round(oilPressure);
        // PSI单位最大值限制（150 psi）
        if (oilPressure > 150)
        {
            oilPressure = 150;
        }
    }

    if (modeCurrent == CLOCK && (clockHour != now.hour() || clockMinute != now.minute()))
    {
        clockHour = now.hour();
        clockMinute = now.minute();

        // 小时有效性检查（>24时归零）
        if (clockHour > 24 && clockMinute > 0)
        {
            clockHour = 0;
            DateTime nowTime = RTC.now();
            RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(), clockHour, clockMinute, 0));
        }

        // 分钟有效性检查（>60分归零）
        if (clockMinute > 60)
        {
            clockMinute = 0;
            DateTime nowTime = RTC.now();
            RTC.adjust(DateTime(nowTime.year(), nowTime.month(), nowTime.day(), clockHour, clockMinute, 0));
        }

        // 标记需要刷新时钟显示
        clockRefresh = true;
    }

    // no dot, if we have recent data
    // 温度数据新鲜度指示器（10秒有效期）
    if (millis() - lastTempUpdate < 10 * 1000)
    {
        u8g2.setDrawColor(0);     // 使用背景色绘制（擦除模式）
        u8g2.drawCircle(1, 1, 1); // 在左上角绘制小圆点指示器
        u8g2.setDrawColor(1);     // 恢复前景色
    }
    else
        u8g2.drawCircle(1, 1, 1);

    // check for complete refresh or just updated values
    // 检查显示刷新需求：模式切换或时钟更新时需要完全刷新
    if (modeOld != modeCurrent || clockRefresh == true)
    {
        updateCompleteDisplay = true; // 标记需要全屏刷新
        modeOld = modeCurrent;        // 同步模式状态
    }
    else
        updateCompleteDisplay = false; // 局部更新即可

    // 根据当前模式刷新OLED显示
    switch (modeCurrent)
    {
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
    if (millis() - lastJsonAppend > 10000)
    {
        appendJsonFile(jsonFile);  // 执行数据追加
        lastJsonAppend = millis(); // 重置计时器
    }

    readyForModeChange = true;   // 允许进行下一次模式切换
    updateSimulatedSensorData(); // 每次循环都更新模拟数据
}
// 自定义非阻塞延时函数，用于模式切换时的延迟
void customDelay(int reqDelay) // 参数：reqDelay-需要延时的毫秒数
{
    unsigned long startTime = millis(); // 记录延时开始时间

    // 非阻塞延时循环（保持Web服务器响应能力）
    while (millis() - startTime < reqDelay)
    {
        server.handleClient();      // 处理Web客户端请求
        if (modeOld != modeCurrent) // 检测模式切换请求
            break;                  // 立即退出延时循环

        esp_task_wdt_reset(); // 喂狗（维护看门狗）
        delay(50);
    }
}
