#include "my_functions.h"
#include "gt86clock.h"
#include <ArduinoJson.h>
#include "esp_task_wdt.h"

void setup(void)
{
    // 初始化看门狗定时器
    esp_task_wdt_init(60, true);
    esp_task_wdt_add(NULL);
    Serial.println("看门狗初始化完成");
    u8g2.begin(); // Oled display begins
    Serial.println("OLED初始化完成");
    u8g2.setFlipMode(1);

    Wire.begin();

    Serial.begin(115200);

    Serial.println("Serial OK");

    pinMode(A0, INPUT);                // oil pressure
    pinMode(buttonPin1, INPUT_PULLUP); // muxed for the 3 buttons
    pinMode(buttonPin2, INPUT_PULLUP); // muxed for the 3 buttons
    pinMode(buttonPin3, INPUT_PULLUP); // 新增第三个按钮

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

    if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    {
        Serial.println("Can1 MCP2515 Initialized Successfully!");
        customDelay(500);
    }
    else
    {
        Serial.println("Error Initializing MCP2515...");
        customDelay(4000);
    }

    Serial.println("CAN ok");

    // Can Hardware Filter
    CAN0.init_Mask(0, 0, 0x0FF0000);
    CAN0.init_Filt(0, 0, 0x07DF0000);
    CAN0.init_Filt(1, 0, 0x07DF0000);

    CAN0.init_Mask(1, 0, 0x07FF0000);
    CAN0.init_Filt(2, 0, 0x07E80000); // OBD
    CAN0.init_Filt(3, 0, 0x03600000); // Water / Oil
    CAN0.init_Filt(4, 0, 0x01340000); // AFR
    CAN0.init_Filt(5, 0, 0x01420000); // Voltage

    CAN0.setMode(MCP_NORMAL);

    // --------------------- WiFi 连接优化 ---------------------
    WiFi.setHostname("gt86clock");
    const char *defaultSSID = "orange";        // 替换为预设的SSID
    const char *defaultPassword = "123456789"; // 替换为预设的密码

    // 优先尝试连接预设的 WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(defaultSSID, defaultPassword);

    Serial.print("Connecting to WiFi");
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
        esp_task_wdt_reset();
        int8_t rssi = WiFi.RSSI();
        if (rssi < -80)
        {
            Serial.printf("\nWeak signal: %ddBm", rssi);
        }
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nConnected to external WiFi!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\nExternal WiFi connection failed. Starting AP...");

        // 配置 WiFiManager 参数
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.setConfigPortalTimeout(180); // AP 模式超时时间

        // 启动 AP 模式（配置门户）
        if (!wifiManager.startConfigPortal("orangeAP"))
        { // AP 名称为 orangeAP
            Serial.println("Failed to start config portal");
        }
        else
        {
            Serial.println("AP Mode Started. SSID: orangeAP");
            Serial.print("AP IP: ");
            Serial.println(WiFi.softAPIP());
        }
    }

    // SPIFFS.begin(); // 初始化 SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS初始化失败!");
        return;
    }
    else
    {
        Serial.println("SPIFFS初始化成功!");
    }
    server.begin(); // 启动 Web 服务器  默认的端口号是 80
    Serial.println("server初始化成功!");

    server.on("/config", handleSpecificArg);
    server.on("/json.js", handleJson);
    server.on("/date.js", handleDateJs);
    server.on("/config.js", handleConfigJs);
    server.on("/temperature.js", handleTemperatureJs);
    server.onNotFound([]()
                      {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound"); });

    // 显式初始化 RTC
    if (!RTC.begin())
    {
        Serial.println("RTC初始化失败!");
        while (1)
            ;
    }
    now = RTC.now();
    char temp[100];
    sprintf(temp, "/json/gt86clock_%04d%02d%02d%02d%02d.js", now.year(), now.month(), now.day(), now.hour(), now.minute());
    jsonFile += temp;
    // createJsonFile(jsonFile.c_str()); // 将 String 转换为 C 风格字符串
    // 增强错误处理
    if (!createJsonFile(jsonFile.c_str()))
    {
        Serial.println("JSON 文件创建失败，系统停止!");
        while (1)
            ;
    }
    else
    {
        Serial.println("JSON 文件创建成功!");
    }
}
// 创建新的JSON数据文件并写入初始数据结构
bool createJsonFile(String jsonFile)
{
    File file = SPIFFS.open(jsonFile, "w"); // 参数 jsonFile: 要创建的文件路径

    if (!file)
    {
        Serial.println("There was an error creating a new json file");
        return false;
    }

    if (!file.println("{\"Time\":[\"Oil Temperature\",\"Coolant Temperature\",\"Oil Pressure\",\"O2\",\"Voltage\"]}"))
    {
        Serial.println("File append failed");
        return false;
    }

    file.close();
    return true;
}
// 向现有JSON文件追加实时传感器数据
bool appendJsonFile(String jsonFile)
{
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    // Serial.printf("Total space: %u bytes, Used space: %u bytes\n", totalBytes, usedBytes); // 打印结果
    if ((totalBytes - usedBytes) < 50)
    {
        // 使用现代文件遍历方法
        File root = SPIFFS.open("/json");
        File file = root.openNextFile();
        if (file)
        {
            SPIFFS.remove(file.path());
            Serial.println("Removing " + String(file.path()) + ". SPIFFS is too full.");
        }
    }
    // 以追加模式打开目标文件
    File file = SPIFFS.open(jsonFile, "a");

    if (!file)
    {
        Serial.println("There was an error opening the file for appending");
        return false;
    }
    // 构建JSON数据条目
    char temp[75];
    String message;
    sprintf(temp, ",{\"%d\":[\"%d\",\"%d\",\"%.2f\",\"%.2f\",\"%.1f\"]}", millis(), oilTemp, coolantTemp, oilPressure, afr, voltage);
    message += temp;
    // 写入文件并检查结果
    if (!file.println(message))
    {
        Serial.println("File append failed");
        return false;
    }
    file.close();
    return true;
}
void readConfig()
{
    byte tmp;     // 临时存储字节数据
    int addr = 0; // EEPROM地址指针初始化
    EEPROM.get(addr, tmp);
    temperatureCelsius = bitRead(tmp, 0);

    addr++; // 地址递增到1
    // 读取压力单位设置（0-bar，1-psi）
    EEPROM.get(addr, tmp);
    pressureBar = bitRead(tmp, 0); // 获取第0位的值

    addr++; // 地址递增到2
    // 读取时间格式设置（0-24小时制，1-12小时制）
    EEPROM.get(addr, tmp);
    clock24h = bitRead(tmp, 0); // 获取第0位的值

    addr++; // 地址递增到3
    // 读取空燃比显示模式（0-O2，1-AFR）
    EEPROM.get(addr, tmp);
    o2afr = bitRead(tmp, 0); // 获取第0位的值

    addr++; // 地址递增到4
    // 读取屏幕模式设置（16位整型，占用地址4和5）
    modeSaved = eepromReadInt(addr); // 调用自定义函数读取整型值

    // 有效性检查：确保保存的模式在合法范围内
    if (modeSaved < CLOCK || modeSaved > MAXSCREENS - 3)
        modeSaved = CLOCK; // 默认恢复时钟模式
    // 新增串口输出
    Serial.println("\n配置读取成功：");
    Serial.printf("温度单位: %s\n", temperatureCelsius ? "华氏度" : "摄氏度");
    Serial.printf("压力单位: %s\n", pressureBar ? "psi" : "bar");
    Serial.printf("时间格式: %s\n", clock24h ? "24小时制" : "12小时制");
    Serial.printf("空燃比模式: %s\n", o2afr ? "AFR" : "O2");
    Serial.printf("保存的屏幕模式: %d\n", modeSaved);
    Serial.println("----------------------");
}
// 将系统配置写入EEPROM持久化存储
void writeConfig()
{
    // 将温度单位设置写入地址0（1字节存储）
    EEPROM.put(0, temperatureCelsius); // 温度单位（0-摄氏度，1-华氏度）

    // 将压力单位设置写入地址1（1字节存储）
    EEPROM.put(1, pressureBar); // 压力单位（0-bar，1-psi）

    // 将时间格式设置写入地址2（1字节存储）
    EEPROM.put(2, clock24h); // 时间格式（0-24小时制，1-12小时制）

    // 将空燃比显示模式写入地址3（1字节存储）
    EEPROM.put(3, o2afr); // 空燃比模式（0-O2，1-AFR）

    // 将当前屏幕模式写入地址4（16位整型，占用地址4和5）
    eepromWriteInt(4, modeCurrent); // 使用自定义函数写入整型值

    // 提交EEPROM更改（确保数据实际写入闪存）
    EEPROM.commit(); // 必须调用commit才能使更改生效
}
// 从EEPROM读取16位整型数据 (小端格式)
int eepromReadInt(int adr)
{
    byte low, high; // 存储高低位字节

    // 读取小端格式存储的16位整型
    low = EEPROM.read(adr);      // 读取低位字节（地址adr）
    high = EEPROM.read(adr + 1); // 读取高位字节（地址adr+1）

    return low + ((high << 8) & 0xFF00);
}

// 向EEPROM写入16位整型数据 (小端格式)
void eepromWriteInt(int adr, int wert)
{
    byte low, high; // 存储高低位字节

    low = wert & 0xFF;         // 获取低位字节（0x00-0xFF）
    high = (wert >> 8) & 0xFF; // 右移8位获取高位字节

    // 将分解后的字节写入EEPROM
    EEPROM.write(adr, low);      // 低位写入地址adr
    EEPROM.write(adr + 1, high); // 高位写入地址adr+1

    return;
}
