// 中断处理代码
#include "my_functions.h"
#include "gt86clock.h"

// 新增全局变量声明（建议放在my_functions.h或gt86clock.h中）
extern volatile bool doubleButtonPressedFlag;

// 按钮1中断服务程序  测试
void IRAM_ATTR handleInterruptPin1() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 300) {
        button1PressedFlag = true;
    }
    last_interrupt_time = interrupt_time;
}

void IRAM_ATTR handleInterruptPin2() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 300) {
        button2PressedFlag = true;
    }
    last_interrupt_time = interrupt_time;
}

// 按钮3中断服务程序（必须存放在IRAM）
void IRAM_ATTR handleInterruptPin3() {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 300) {
        button3PressedFlag = true;
    }
    last_interrupt_time = interrupt_time;
}
