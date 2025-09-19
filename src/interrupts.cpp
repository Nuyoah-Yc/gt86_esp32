// 中断处理代码
#include "my_functions.h"
#include "gt86clock.h"

// 新增全局变量声明（建议放在my_functions.h或gt86clock.h中）
extern volatile bool doubleButtonPressedFlag;

// 按钮1中断服务程序  测试
void ICACHE_RAM_ATTR handleInterruptPin1() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    // 消抖：50ms内不重复响应
    if (interrupt_time - last_interrupt_time > 50) {
        button1PressedFlag = true;
        buttonPin1Pressed = millis(); // 新增此行，记录按下时间戳
        // 检查双键
        if (digitalRead(buttonPin2) == LOW) {
            doubleButtonPressedFlag = true;
        }
    }
    last_interrupt_time = interrupt_time;
}

// 按钮2中断服务程序（同理） 测试
void ICACHE_RAM_ATTR handleInterruptPin2() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 50) {
        button2PressedFlag = true;
        buttonPin2Pressed = millis(); // 新增此行，记录按下时间戳
        // 检查双键
        if (digitalRead(buttonPin1) == LOW) {
            doubleButtonPressedFlag = true;
        }
    }
    last_interrupt_time = interrupt_time;
}

// 按钮3中断服务程序（必须存放在IRAM）
void ICACHE_RAM_ATTR handleInterruptPin3() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 50) {
        button3PressedFlag = true;
        buttonPin3Pressed = millis();
    }
    last_interrupt_time = interrupt_time;
}
