#include "my_functions.h"
#include "gt86clock.h"
#include "esp_task_wdt.h"

void setup(void) {
    esp_task_wdt_init(60, true);
    esp_task_wdt_add(NULL);
    u8g2.begin();
    u8g2.setFlipMode(1);

    Wire.begin();

    Serial.begin(115200);

    Serial.println("Serial OK");

    pinMode(A0, INPUT); // oil pressure
    pinMode(buttonPin1, INPUT_PULLUP);
    pinMode(buttonPin2, INPUT_PULLUP);
    pinMode(buttonPin3, INPUT_PULLUP);

    Serial.println("PINS1 ok");

    // attachInterrupt(digitalPinToInterrupt(buttonPin1), handleInterruptPin1, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(buttonPin2), handleInterruptPin2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(buttonPin1), handleInterruptPin1, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonPin2), handleInterruptPin2, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonPin3), handleInterruptPin3, FALLING);

    Serial.println("PINS2 ok");

    EEPROM.begin(512);
    readConfig();

    Serial.println("EEPROM ok");

    // 显式初始化 RTC
    if (!RTC.begin()) {
        Serial.println("RTC初始化失败!");
        while (1);
    }
    now = RTC.now();
    // JSON文件创建已移除
    Serial.println("RTC初始化完成!");
}

// JSON文件相关函数已移除

void readConfig() {
    byte tmp; // 临时存储字节数据
    int addr = 2; // 跳过已移除的单位设置，从地址2开始
    // 读取时间格式设置（0-24小时制，1-12小时制）
    EEPROM.get(addr, tmp);
    clock24h = bitRead(tmp, 0); // 获取第0位的值

    addr++; // 地址递增到3
    // 空燃比显示模式已移除

    addr++; // 地址递增到4
    // 读取屏幕模式设置（16位整型，占用地址4和5）
    modeSaved = eepromReadInt(addr); // 调用自定义函数读取整型值

    // 有效性检查：确保保存的模式在合法范围内
    if (modeSaved < CLOCK || modeSaved > MAXSCREENS - 1)
        modeSaved = CLOCK; // 默认恢复时钟模式
    // 新增串口输出
    Serial.println("\n配置读取成功：");
    Serial.println("温度单位: 摄氏度（固定）");
    Serial.println("压力单位: bar（固定）");
    Serial.printf("时间格式: %s\n", clock24h ? "24小时制" : "12小时制");
    Serial.printf("保存的屏幕模式: %d\n", modeSaved);
    Serial.println("----------------------");
}

// 将系统配置写入EEPROM持久化存储
void writeConfig() {
    // 跳过已移除的单位设置（地址0和1），从地址2开始

    // 将时间格式设置写入地址2（1字节存储）
    EEPROM.put(2, clock24h); // 时间格式（0-24小时制，1-12小时制）

    // 空燃比显示模式已移除

    // 将当前屏幕模式写入地址4（16位整型，占用地址4和5）
    eepromWriteInt(4, modeCurrent); // 使用自定义函数写入整型值

    // 提交EEPROM更改（确保数据实际写入闪存）
    EEPROM.commit(); // 必须调用commit才能使更改生效
}

// 从EEPROM读取16位整型数据 (小端格式)
int eepromReadInt(int adr) {
    byte low, high; // 存储高低位字节

    // 读取小端格式存储的16位整型
    low = EEPROM.read(adr); // 读取低位字节（地址adr）
    high = EEPROM.read(adr + 1); // 读取高位字节（地址adr+1）

    return low + ((high << 8) & 0xFF00);
}

// 向EEPROM写入16位整型数据 (小端格式)
void eepromWriteInt(int adr, int wert) {
    byte low, high; // 存储高低位字节

    low = wert & 0xFF; // 获取低位字节（0x00-0xFF）
    high = (wert >> 8) & 0xFF; // 右移8位获取高位字节

    // 将分解后的字节写入EEPROM
    EEPROM.write(adr, low); // 低位写入地址adr
    EEPROM.write(adr + 1, high); // 高位写入地址adr+1
}