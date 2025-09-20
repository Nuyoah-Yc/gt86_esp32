// 时钟界面绘制代码
#include "gt86clock.h"
#include "my_functions.h"
#include "logos.h"
// 参数 updateCompleteDisplay: 是否需要全屏刷新（模式切换时需置为true）
void drawClock(bool updateCompleteDisplay) {
    if (updateCompleteDisplay == true) // 需要全屏刷新时
    {
        u8g2.clearDisplay(); // 清空显示缓冲区
        u8g2.setDrawColor(1); // 设置绘制颜色为前景色（白色）
        // 在屏幕右上角绘制品牌LOGO（X坐标96，宽度32，高度32）
        u8g2.drawXBM(96, 0, iconGTWidth, iconGTHeight, iconGTBits);

        u8g2.setFont(u8g2_font_logisoso32_tn); // 使用等宽数字字体（便于对齐）
        // 小时显示处理（两位数补零）
        if (clockHour > 9)
        // 直接显示双位小时（37-37=0，保持原始坐标）
            u8g2.drawStr(37 - 37, 32, String(clockHour).c_str());
        else {
            // 个位小时显示方案：先显示"0"再显示数字
            u8g2.drawStr(37 - 37, 32, "0"); // 固定位置显示十位的0
            u8g2.drawStr(57 - 37, 32, String(clockHour).c_str()); // 右移20像素显示个位
        }

        // 分钟显示处理（同小时逻辑）
        if (clockMinute > 9) {
            // 直接显示双位分钟
            u8g2.drawStr(84 - 37, 32, String(clockMinute).c_str());
        } else {
            // 个位分钟显示方案
            u8g2.drawStr(84 - 37, 32, "0"); // 固定位置显示十位的0
            u8g2.drawStr(104 - 37, 32, String(clockMinute).c_str()); // 右移20像素显示个位
        }
        clockRefresh = false; // 清除刷新标志（避免重复刷新）
    }
    // 动态冒号闪烁效果（500ms间隔）
    u8g2.setDrawColor(drawDots);
    u8g2.drawBox(79 - 37, 8, 3, 3);
    u8g2.drawBox(79 - 37, 20, 3, 3);
    u8g2.setDrawColor(1);
    u8g2.sendBuffer();
    customDelay(500); // 控制闪烁频率
    drawDots = !drawDots; // 反转冒号状态

    clockRefresh = false; // 新增：重置刷新标志
    u8g2.sendBuffer();
}
