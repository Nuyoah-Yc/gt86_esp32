// Settings
// 设置界面绘制代码
#include "my_functions.h"
#include "gt86clock.h"
// WiFi设置界面显示函数
// 显示内容：
// 1. 当前连接WiFi的SSID
// 2. 设备IP地址
// 特性：显示2秒后自动切换至下一设置项
void drawSettingsWifi(bool updateCompleteDisplay)
{
    u8g2.clearDisplay();                // 清空显示缓冲区
    u8g2.setDrawColor(1);               // 设置绘制颜色为白色（前景色）
    u8g2.setFont(u8g2_font_crox2tb_tf); // 使用中等粗细的等宽字体

    // 获取并显示网络信息
    String ipaddress = WiFi.localIP().toString();
    u8g2.drawStr(1, 16, ("SSID:" + WiFi.SSID()).c_str()); // 第一行显示WiFi名称
    u8g2.drawStr(1, 32, ipaddress.c_str());               // 第二行显示IP地址

    u8g2.sendBuffer(); // 刷新OLED显示

    // 界面切换控制
    modeCurrent++;         // 切换到下一个设置项（按枚举顺序）
    modeOld = modeCurrent; // 记录当前模式状态
    customDelay(2000);     // 保持界面显示2秒（用户可读时间）
}
// 时钟设置界面显示函数
// 当前时间格式（12/24小时制）
void drawSettingsClock(bool updateCompleteDisplay)
{
    char temp[20];       // 时间格式化缓冲区
    u8g2.clearDisplay(); // 清空显示缓冲区

    // 格式化日期时间（YYYY-MM-DD HH:MM:SS）
    sprintf(temp, "%d-%02d-%02d %02d:%02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());

    u8g2.drawStr(1, 16, temp); // 首行显示完整时间（坐标：x=1,y=16）

    // 时间格式状态显示（第二行）
    if (clock24h)
        u8g2.drawStr(1, 32, "CLOCK: 24h"); // 24小时制
    else
        u8g2.drawStr(1, 32, "CLOCK: 12h"); // 12小时制

    u8g2.sendBuffer(); // 发送缓冲区内容到OLED

    // 界面切换逻辑
    modeCurrent++;         // 切换到下一个设置项
    modeOld = modeCurrent; // 同步模式状态
    customDelay(2000);     // 维持2秒显示（用户可读时间）
}

void drawSettingsUnits(bool updateCompleteDisplay)
{
    u8g2.clearDisplay(); // 清空显示缓冲区

    // 温度单位显示（第一行）
    if (temperatureCelsius)
        u8g2.drawStr(1, 16, "TEMP.: C"); // 摄氏温度
    else
        u8g2.drawStr(1, 16, "TEMP.: F"); // 华氏温度

    // 压力单位显示（第二行）
    if (pressureBar)
        u8g2.drawStr(1, 32, "PRESSURE: bar"); // 公制单位
    else
        u8g2.drawStr(1, 32, "PRESSURE: psi"); // 英制单位

    u8g2.sendBuffer(); // 发送缓冲区内容到OLED

    // 界面切换逻辑
    modeCurrent++;         // 切换到下一个设置项
    modeOld = modeCurrent; // 同步模式状态
    customDelay(2000);     // 维持2秒显示
}

void drawSettingsO2(bool updateCompleteDisplay)
{
    u8g2.clearDisplay(); // 清空显示缓冲区

    // 空燃比显示模式切换
    if (o2afr)
        u8g2.drawStr(1, 16, "O2: AFR"); // 空燃比模式（实际比值）
    else
        u8g2.drawStr(1, 20, "O2: Lambda"); // λ值模式（理论比值）

    u8g2.sendBuffer(); // 发送缓冲区内容到OLED

    // 界面循环逻辑
    // modeCurrent = SETTINGSWIFI; // 返回WiFi设置首选项
    modeOld = modeCurrent; // 同步模式状态
    customDelay(2000);     // 维持2秒显示
}
