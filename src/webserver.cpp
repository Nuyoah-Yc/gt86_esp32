// Web服务器代码
#include "gt86clock.h"
#include "my_functions.h"

void handleDateJs() {
    char temp[75];
    String message;
    sprintf(temp, "dataDateJson = '[{ \"date\": \"%d-%02d-%02d %02d:%02d:%02d\" }]';", now.year(), now.month(),
            now.day(), now.hour(), now.minute(), now.second());
    message += temp;

    server.send(200, "application/javascript", message);
}

void handleTemperatureJs() {
    char temp[75];
    String message;
    server.send(200, "application/json", message);
}

bool handleJson() {
    // 打开目录
    File root = SPIFFS.open("/json");
    if (!root || !root.isDirectory()) {
        return false; // 目录打开失败，返回 false
    }

    // 遍历目录中的文件
    String message;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            message += file.name();
            message += "\n";
        }
        file = root.openNextFile();
    }

    server.send(200, "text/plain", message);
    return true; // 操作成功，返回 true
}

bool handleFileRead(String path) {
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/")) {
        path += "index.html";
    }
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
        if (SPIFFS.exists(pathWithGz))
            path += ".gz";
        if (path == "index.html")
            server.sendHeader("Cache-Control", "no-cache");
        else
            server.sendHeader("Cache-Control", "max-age=3600");
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

String getContentType(String filename) {
    if (server.hasArg("download")) {
        return "application/octet-stream";
    } else if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".gz")) {
        return "application/x-gzip";
    }
    return "text/plain";
}

void syncNTP() {
    NTPch.begin(); // 初始化 NTP 客户端
    NTPch.update(); // 强制时间同步

    // 获取 Unix 时间戳（UTC 时间）
    unsigned long epochTime = NTPch.getEpochTime();

    // 转换为 RTClib 的 DateTime 对象（自动处理时区）
    dateTime = DateTime(epochTime + 8 * 3600); // 示例：UTC+8

    // 验证时间有效性（例如年份是否合理）
    if (dateTime.year() >= 2023) {
        // 设置 RTC 时间（通过成员函数访问字段）
        Clock.adjust(dateTime); // 用 adjust 方法

        // 调试输出
        Serial.print("NTP Time: ");
        Serial.print(dateTime.year());
        Serial.print("-");
        Serial.print(dateTime.month());
        Serial.print("-");
        Serial.print(dateTime.day());
        Serial.print(" ");
        Serial.print(dateTime.hour());
        Serial.print(":");
        Serial.print(dateTime.minute());
        Serial.print(":");
        Serial.println(dateTime.second());

        modeOld = 0;
    }
}

boolean setIfBool(String varName) {
    if (server.arg(varName) == "1") {
        modeOld = 0;
        return true;
    }
    if (server.arg(varName) == "0") {
        modeOld = 0;
        return false;
    }
    return false;
}

void handleConfigJs() {
    char temp[200];
    String message;
    sprintf(temp, "dataConfigJson = '[{ \"modeCurrent\": \"%d\", \"clock24h\": \"%d\", \"o2afr\": \"%d\"  }]';",
            modeCurrent, clock24h, o2afr);
    message += temp;

    server.send(200, "application/javascript", message);
}

// Web参数处理函数（处理来自设置页面的表单提交）
void handleSpecificArg() {
    // 处理模式切换参数（示例URL：/config?mode=3）
    if (isDigit(server.arg("mode").charAt(0)) &&
        server.arg("mode").toInt() >= 0 &&
        server.arg("mode").toInt() <= MAXSCREENS) {
        Serial.println(server.arg("mode").toInt());
        modeCurrent = server.arg("mode").toInt(); // 更新当前显示模式
    }

    // 处理配置参数（单位已固定为公制）
    if (server.arg("clock24h")) // 24小时制开关
        clock24h = setIfBool("clock24h");
    if (server.arg("o2afr")) // 空燃比显示模式（AFR/Lambda）
        o2afr = setIfBool("o2afr");

    // 处理NTP同步请求
    if (server.arg("ntp") == "true") {
        syncNTP(); // 执行NTP时间同步
        modeOld = 0; // 重置模式状态（触发显示刷新）
    }

    writeConfig(); // 保存配置到EEPROM
    server.send(200, "text/plain", "Ok");
}