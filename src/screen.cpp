// 显示屏逻辑处理
#include "my_functions.h"
#include "gt86clock.h"

void drawAfrAndVoltage(bool updateCompleteDisplay) {
    if (o2afr) {
        // AFR显示模式
        // 转换AFR值为字符串（保留1位小数）
        char afrChar[4];
        dtostrf(afr, 1, 1, afrChar);

        // 转换电压值为字符串（保留1位小数）
        char voltageChar[4];
        dtostrf(voltage, 1, 1, voltageChar);

        // 调用双参数绘制函数（带百分比计算）
        drawTwoValues(o2Width, o2Height, o2Bits,
                      afrChar, countDigits(round((afr / 14.7) * 100) / 100), "",
                      batteryWidth, batteryHeight, batteryBits,
                      voltageChar, 4, "V", updateCompleteDisplay);
    } else {
        // 原始值显示模式
        // 转换AFR值为字符串（保留2位小数）
        char afrChar[4];
        dtostrf(afr, 1, 2, afrChar);

        // 转换电压值为字符串（保留1位小数）
        char voltageChar[4];
        dtostrf(voltage, 1, 1, voltageChar);

        // 调用双参数绘制函数（直接显示原始值）
        drawTwoValues(o2Width, o2Height, o2Bits,
                      afrChar, countDigits(afr), "",
                      batteryWidth, batteryHeight, batteryBits,
                      voltageChar, 4, "V", updateCompleteDisplay);
    }
}

void drawCoolantOilTemp(bool updateCompleteDisplay) {
    // 固定使用摄氏度显示
    drawTwoValues(iconCoolantWidth, iconCoolantHeight, iconCoolantBits, (char *) String(coolantTemp).c_str(),
                  countDigits(coolantTemp), "C",
                  iconOilCanWidth, iconOilCanHeight, iconOilCanBits, (char *) String(oilTemp).c_str(),
                  countDigits(oilTemp), "C", updateCompleteDisplay);
}

void drawCoolantTemp(bool updateCompleteDisplay) {
    // 固定使用摄氏度显示
    drawSingleValue(iconCoolantWidth, iconCoolantHeight, iconCoolantBits, (char *) String(coolantTemp).c_str(),
                    countDigits(coolantTemp), "C", updateCompleteDisplay);
}

void drawOilTemp(bool updateCompleteDisplay) {
    // 固定使用摄氏度显示
    drawSingleValue(iconOilCanWidth, iconOilCanHeight, iconOilCanBits, (char *) String(oilTemp).c_str(),
                    countDigits(oilTemp), "C", updateCompleteDisplay);
}

void drawOilPressure(bool updateCompleteDisplay) {
    // 固定使用bar（公制单位）显示
    char oilPressureChar[4];
    dtostrf(oilPressure, 1, 1, oilPressureChar);
    drawSingleValue(iconOilCanWidth, iconOilCanHeight, iconOilCanBits, oilPressureChar, 3, "bar",
                    updateCompleteDisplay);

    customDelay(250);
}

void drawLogo(bool updateCompleteDisplay) {
    if (updateCompleteDisplay == true) {
        u8g2.clearDisplay();
        u8g2.drawXBM(0, 0, logoWidth, logoHeight, logoBits);
        u8g2.sendBuffer();
        modeCurrent = modeSaved;
        delay(1000);
    }
}

int countDigits(int n) {
    if (n < 0)
        return floor(log10(abs(n)) + 2);
    else if (n > 0)
        return floor(log10(abs(n)) + 1);
    else
        return 1;
}

void drawSingleValue(int iconWidth, int iconHeight, const unsigned char *iconBits, const char *value, int valueLength,
                     const char *unit, bool updateCompleteDisplay) {
    if (updateCompleteDisplay == true) {
        u8g2.clearDisplay();
        u8g2.drawXBM(2, round((32 - iconHeight) / 2), iconWidth, iconHeight, iconBits);

        // if (unit == "C" || unit == "F")
        if (strcmp(unit, "C") == 0 || strcmp(unit, "F") == 0) {
            u8g2.drawCircle(118, 3, 3);
            u8g2.setFont(u8g2_font_crox3cb_mf);
            u8g2.drawStr(116, 32, unit);
        } else {
            u8g2.setFont(u8g2_font_pxplusibmvga9_mf);
            u8g2.drawStr(120, 32, &unit[2]);
            u8g2.drawStr(120, 21, &unit[1]);
            u8g2.drawStr(120, 10, &unit[0]);
        }
    }

    u8g2.setDrawColor(0);
    u8g2.drawBox(55, 0, 60, 32);
    u8g2.setDrawColor(1);

    u8g2.setFont(u8g2_font_logisoso32_tn);
    u8g2.drawStr(115 - valueLength * 20, 32, value);

    u8g2.sendBuffer();
}

// 修改后的基准函数（与目标函数参数兼容）
void drawTwoValues(
    int iconUpWidth, int iconUpHeight, const unsigned char *iconUpBits,
    const char *valueUp, int valueUpLength, const char *unitUp,
    int iconDownWidth, int iconDownHeight, const unsigned char *iconDownBits,
    const char *valueDown, int valueDownLength, const char *unitDown,
    bool updateCompleteDisplay) {
    if (updateCompleteDisplay == true) {
        u8g2.clearDisplay();
        u8g2.drawXBM(4, -17, iconUpWidth, iconUpHeight, iconUpBits);
        u8g2.drawXBM(2, 18, iconDownWidth, iconDownHeight, iconDownBits);

        // if (unitUp == "C" || unitDown == "F")
        if (strcmp(unitUp, "C") == 0 || strcmp(unitDown, "F") == 0) {
            u8g2.setFont(u8g2_font_crox3cb_mf);
            u8g2.drawCircle(112, 18, 2);
            u8g2.drawCircle(112, 2, 2);
            u8g2.drawStr(116, 16, unitUp);
            u8g2.drawStr(116, 32, unitDown);
        } else if (strcmp(unitDown, "V") == 0) {
            u8g2.setFont(u8g2_font_crox3cb_mf);
            u8g2.drawStr(116, 32, unitDown);
        }
    }

    u8g2.setDrawColor(0);
    u8g2.drawBox(70, 0, 39, 32);
    u8g2.setDrawColor(1);

    u8g2.setFont(u8g2_font_crox3cb_mf);

    u8g2.drawStr(90 - valueUpLength * 13, 16, valueUp);
    u8g2.drawStr(90 - valueDownLength * 13, 32, valueDown);

    u8g2.sendBuffer();
}
