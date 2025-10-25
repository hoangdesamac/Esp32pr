#pragma once
#include <stdio.h>
#include <string.h>

class SerialClass {
public:
    void begin(int baudrate) {
        // Trên ESP-IDF, UART0 mặc định kết nối với cổng USB (UART console)
        printf("Serial started at %d baud\n", baudrate);
    }

    void print(const char* text) {
        printf("%s", text);
    }

    void println(const char* text) {
        printf("%s\n", text);
    }

    void print(int number) {
        printf("%d", number);
    }

    void println(int number) {
        printf("%d\n", number);
    }

    void print(float number) {
        printf("%f", number);
    }

    void println(float number) {
        printf("%f\n", number);
    }
};

// Tạo đối tượng toàn cục giống Arduino
extern SerialClass Serial;
