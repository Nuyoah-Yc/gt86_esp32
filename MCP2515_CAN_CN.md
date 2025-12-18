# MCP2515 CAN 总线模块 - 数据读取文档

本文档从 GT86clock 项目中提取，详细说明 MCP2515 CAN 总线模块的使用方法和数据读取逻辑。

## 目录
1. [硬件连接](#硬件连接)
2. [依赖库](#依赖库)
3. [初始化代码](#初始化代码)
4. [CAN 过滤器配置](#can-过滤器配置)
5. [数据读取逻辑](#数据读取逻辑)
6. [CAN ID 与数据解析](#can-id-与数据解析)
7. [完整示例代码](#完整示例代码)

---

## 硬件连接

MCP2515 模块通过 SPI 接口与 ESP8266 连接：

| 功能      | ESP32 引脚 | MCP2515 引脚 |
|-----------|-------------|--------------|
| CS (片选) |            | CS           |
| INT (中断)|  | INT          |
| MOSI      |  | SI           |
| MISO      |  | SO           |
| SCK       |  | SCK          |
| VCC       | 3.3V        | VCC          |
| GND       | GND         | GND          |

---

## 依赖库

### MCP_CAN 库
- **库名称**: MCP_CAN
- **测试版本**: 1.5
- **安装方式**: Arduino IDE 库管理器搜索 "MCP_CAN" 或从 GitHub 下载

```cpp
#include <mcp_can.h>
#include <SPI.h>
```

### 库下载地址
- GitHub: https://github.com/coryjfowler/MCP_CAN_lib

---

## 初始化代码

### 引脚定义与对象创建

```cpp
#include <mcp_can.h>
#include <SPI.h>

// 定义中断引脚和片选引脚
#define CAN0_INT x
MCP_CAN CAN0(x);

// 数据接收缓冲区
long unsigned int rxId;       // 接收到的 CAN ID
unsigned char len = 0;        // 数据长度
unsigned char buf[12];        // 数据缓冲区
```

### 初始化函数

```cpp
void setupCAN() {
  // 初始化 MCP2515
  // 参数说明:
  //   MCP_STDEXT  - 标准帧和扩展帧模式
  //   CAN_500KBPS - CAN 总线速率 500Kbps (OBD2 标准速率)
  //   MCP_8MHZ    - MCP2515 晶振频率 8MHz

  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("MCP2515 初始化成功!");
  } else {
    Serial.println("MCP2515 初始化失败...");
  }
}
```

### 常用波特率配置

| 波特率参数      | 速率      | 用途                    |
|----------------|-----------|------------------------|
| CAN_500KBPS    | 500 Kbps  | OBD2 标准速率           |
| CAN_250KBPS    | 250 Kbps  | 部分欧洲车型            |
| CAN_125KBPS    | 125 Kbps  | 工业设备常用            |
| CAN_1000KBPS   | 1 Mbps    | 高速 CAN               |

### 晶振频率配置

| 晶振参数    | 频率    |
|------------|---------|
| MCP_8MHZ   | 8 MHz   |
| MCP_16MHZ  | 16 MHz  |
| MCP_20MHZ  | 20 MHz  |

---

## CAN 过滤器配置

MCP2515 有 2 个掩码和 6 个过滤器，用于筛选需要接收的 CAN ID。

### 过滤器工作原理

```
接收条件: (接收ID & 掩码) == (过滤器 & 掩码)
```

### GT86 车辆的过滤器配置

```cpp
void setupCANFilters() {
  // 掩码 0 - 应用于过滤器 0 和 1
  CAN0.init_Mask(0, 0, 0x0FF0000);
  CAN0.init_Filt(0, 0, 0x07DF0000);    // 通用 OBD2 请求
  CAN0.init_Filt(1, 0, 0x07DF0000);    // 通用 OBD2 请求

  // 掩码 1 - 应用于过滤器 2-5
  CAN0.init_Mask(1, 0, 0x07FF0000);
  CAN0.init_Filt(2, 0, 0x07E80000);    // OBD2 响应
  CAN0.init_Filt(3, 0, 0x03600000);    // 水温 / 机油温度
  CAN0.init_Filt(4, 0, 0x01340000);    // 空燃比 (AFR)

  // 设置为正常模式开始接收数据
  CAN0.setMode(MCP_NORMAL);
}
```

### 过滤器参数说明

```cpp
// init_Mask(掩码编号, 扩展帧标志, 掩码值)
// init_Filt(过滤器编号, 扩展帧标志, 过滤器值)

// 掩码编号: 0 或 1
// 扩展帧标志: 0=标准帧(11位ID), 1=扩展帧(29位ID)
// 值格式: 对于标准帧，ID 需要左移 16 位
```

### 禁用过滤器（接收所有消息）

```cpp
void disableFilters() {
  CAN0.init_Mask(0, 0, 0x00000000);
  CAN0.init_Mask(1, 0, 0x00000000);
  CAN0.setMode(MCP_NORMAL);
}
```

---

## 数据读取逻辑

### 基本读取循环

```cpp
void loop() {
  // 检查是否有可用的 CAN 消息
  if (CAN_MSGAVAIL == CAN0.checkReceive()) {
    // 读取消息到缓冲区
    // 参数: &rxId=接收ID, &len=数据长度, buf=数据缓冲区
    CAN0.readMsgBuf(&rxId, &len, buf);

    // 处理接收到的数据
    processCANMessage(rxId, buf, len);
  }
}
```

### 调试输出

```cpp
void printCANMessage(long unsigned int id, unsigned char* data, unsigned char length) {
  Serial.print("ID: 0x");
  Serial.print(id, HEX);
  Serial.print("  数据: ");

  for (int i = 0; i < length; i++) {
    if (data[i] < 0x10) {
      Serial.print("0");  // 补零
    }
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
```

---

## CAN ID 与数据解析

### GT86/BRZ/FRS 车辆 CAN ID 列表

| CAN ID | 数据内容       | 数据字节位置 | 解析公式                      |
|--------|---------------|-------------|------------------------------|
| 0x360  | 机油温度       | buf[2]      | 温度 = buf[2] - 40 (°C)      |
| 0x360  | 冷却液温度     | buf[3]      | 温度 = buf[3] - 40 (°C)      |
| 0x134  | 空燃比 (AFR)   | buf[0-1]    | 需要进一步解析                |
|        |              |              |                         |

### 温度数据解析 (0x360)

```cpp
int oilTemp = 0;      // 机油温度
int coolantTemp = 0;  // 冷却液温度

void parseTemperature(unsigned char* buf) {
  if (buf[2] > 0 && buf[2] < 200) {
    oilTemp = buf[2] - 40;  // 摄氏度

    // 转换为华氏度 (可选)
    // oilTemp = round(oilTemp * 1.8 + 32);
  }

  if (buf[3] > 0 && buf[3] < 200) {
    coolantTemp = buf[3] - 40;  // 摄氏度

    // 转换为华氏度 (可选)
    // coolantTemp = round(coolantTemp * 1.8 + 32);
  }
}
```

### 空燃比数据解析 (0x134)

```cpp
float afr = 14.6;  // 默认理论空燃比

void parseAFR(unsigned char* buf) {
  // 注意: 具体解析公式可能需要根据实际 ECU 调整
  // 通用公式参考: (A*256+B)/32768*14.7

  // 有效范围检查
  if (afr < -40 || afr > 320) {
    afr = 999;  // 无效值标记
  }

  // 转换为 Lambda 值 (可选)
  // float lambda = afr / 14.7;
}
```

### 完整消息处理函数

```cpp
void processCANMessage(long unsigned int id, unsigned char* buf, unsigned char len) {
  switch (id) {
    case 0x360:
      // 温度数据
      parseTemperature(buf);
      Serial.print("机油温度: ");
      Serial.print(oilTemp);
      Serial.print("°C  冷却液温度: ");
      Serial.print(coolantTemp);
      Serial.println("°C");
      break;

    case 0x134:
      // 空燃比数据
      parseAFR(buf);
      Serial.print("AFR: ");
      Serial.println(afr);
      break;

    case 0x142:
      // 电压数据
      parseVoltage(buf);
      Serial.print("电压: ");
      Serial.print(voltage);
      Serial.println("V");
      break;

    default:
      // 未知 ID，可用于调试
      printCANMessage(id, buf, len);
      break;
  }
}
```

---

## 完整示例代码

以下是一个可独立运行的完整示例：

```cpp
/*
 * MCP2515 CAN 总线数据读取示例
 * 适用于: ESP8266 + MCP2515 模块
 * 目标车辆: Toyota GT86 / Subaru BRZ / Scion FRS
 */

#include <mcp_can.h>
#include <SPI.h>

// ============== 引脚配置 ==============
#define CAN0_INT x           
#define CAN0_CS  x           

// ============== 全局变量 ==============
MCP_CAN CAN0(CAN0_CS);

long unsigned int rxId;
unsigned char len = 0;
unsigned char buf[12];

// 解析后的数据
int oilTemp = 0;
int coolantTemp = 0;
float voltage = 0;
float afr = 14.6;

// ============== 初始化 ==============
void setup() {
  Serial.begin(115200);
  Serial.println("MCP2515 CAN 总线读取器启动...");

  // 初始化 MCP2515
  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("MCP2515 初始化成功!");
  } else {
    Serial.println("MCP2515 初始化失败!");
    while (1);  // 初始化失败则停止
  }

  // 配置 CAN 过滤器
  setupCANFilters();

  Serial.println("开始监听 CAN 总线...");
}

void setupCANFilters() {
  // 掩码和过滤器配置
  CAN0.init_Mask(0, 0, 0x0FF0000);
  CAN0.init_Filt(0, 0, 0x07DF0000);
  CAN0.init_Filt(1, 0, 0x07DF0000);

  CAN0.init_Mask(1, 0, 0x07FF0000);
  CAN0.init_Filt(2, 0, 0x07E80000);
  CAN0.init_Filt(3, 0, 0x03600000);  // 温度
  CAN0.init_Filt(4, 0, 0x01340000);  // AFR

  CAN0.setMode(MCP_NORMAL);
}

// ============== 主循环 ==============
void loop() {
  if (CAN_MSGAVAIL == CAN0.checkReceive()) {
    CAN0.readMsgBuf(&rxId, &len, buf);

    switch (rxId) {
      case 0x360:
        parseTemperature();
        break;
      case 0x134:
        parseAFR();
        break;
      case 0x142:
        parseVoltage();
        break;
    }
  }
}

// ============== 数据解析函数 ==============
void parseTemperature() {
  if (buf[2] > 0 && buf[2] < 200) {
    oilTemp = buf[2] - 40;
    Serial.print("机油温度: ");
    Serial.print(oilTemp);
    Serial.println("°C");
  }

  if (buf[3] > 0 && buf[3] < 200) {
    coolantTemp = buf[3] - 40;
    Serial.print("冷却液温度: ");
    Serial.print(coolantTemp);
    Serial.println("°C");
  }
}

void parseAFR() {
  // AFR 解析逻辑 (根据实际 ECU 调整)
  Serial.print("收到 AFR 数据包, 原始数据: ");
  for (int i = 0; i < len; i++) {
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
```

---

## 常见问题

### 1. 初始化失败
- 检查 SPI 接线是否正确
- 确认晶振频率参数与模块实际晶振匹配
- 检查 CS 引脚定义是否正确

### 2. 收不到数据
- 确认 CAN 总线波特率正确（OBD2 通常为 500Kbps）
- 检查过滤器配置是否正确
- 使用 `disableFilters()` 临时关闭过滤器进行调试
- 确认 CAN-H 和 CAN-L 接线正确

### 3. 数据解析错误
- 不同车型的 CAN ID 和数据格式可能不同
- 使用调试模式打印原始数据进行分析
- 参考车辆的 CAN 数据库文档

### 4. MCP2515 模块选择
- 建议使用带 TJA1050 收发器的模块
- 确保模块支持 3.3V 逻辑电平（ESP8266 兼容）

---

## 参考资料

- MCP_CAN 库: https://github.com/coryjfowler/MCP_CAN_lib
- MCP2515 数据手册: Microchip 官网
- OBD2 PID 列表: https://en.wikipedia.org/wiki/OBD-II_PIDs
