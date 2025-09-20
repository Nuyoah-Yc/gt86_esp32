#include "my_functions.h"
#include "gt86clock.h"
#include "esp_task_wdt.h"

void setup() {
    esp_task_wdt_init(60, true);
    esp_task_wdt_add(nullptr);
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

    attachInterrupt(digitalPinToInterrupt(buttonPin1), handleInterruptPin1, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonPin2), handleInterruptPin2, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonPin3), handleInterruptPin3, FALLING);

    Serial.println("PINS2 ok");

    clock24h = true;
    modeSaved = CLOCK;

    // 显式初始化 RTC
    if (!RTC.begin()) {
        Serial.println("RTC初始化失败!");
        while (true);
    }
    now = RTC.now();
    Serial.println("RTC初始化完成!");
}