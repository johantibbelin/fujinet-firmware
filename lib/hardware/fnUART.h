/* Basically a simplified copy of the ESP Arduino library in HardwareSerial.h/HardwareSerial.cpp
*/
#ifndef FNUART_H
#define FNUART_H

#ifdef ESP_PLATFORM
#include <driver/uart.h>
#endif

#if defined (_WIN32)
#include <windows.h>
#endif

#include <string>
#include <cstdint>


class UARTManager
{
private:
#ifdef ESP_PLATFORM
    uart_port_t _uart_num;
    QueueHandle_t _uart_q;
#else
    char _device[64]; // device name or path
    uint32_t _baud;
    int _command_pin;
    int _proceed_pin;

#if defined (_WIN32)
    int _command_status;
    int _proceed_set;
    int _proceed_clear;
    HANDLE _fd;
#else
    int _command_tiocm;
    int _proceed_tiocm;
    int _fd;
#endif
    // serial port error counter
    int _errcount;
    unsigned long _suspend_time;
#endif // !ESP_PLATFORM

    bool _initialized = false; // is UART ready?

    size_t _print_number(unsigned long n, uint8_t base);

public:
#ifdef ESP_PLATFORM
    UARTManager(uart_port_t uart_num=0);

    void begin(int baud);
    void end();
    void set_baudrate(uint32_t baud);
    bool initialized() { return _initialized; }

    int available();
    int peek();
    void flush();
    void flush_input();

    int read();
    size_t readBytes(uint8_t *buffer, size_t length);
    size_t readBytes(char *buffer, size_t length) { return readBytes((uint8_t *)buffer, length); };

    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *s);

    size_t write(unsigned long n) { return write((uint8_t)n); };
    size_t write(long n) { return write((uint8_t)n); };
    size_t write(unsigned int n) { return write((uint8_t)n); };
    size_t write(int n) { return write((uint8_t)n); };

    size_t printf(const char *format, ...);

    //size_t println(const char *format, ...);
    size_t println(const char *str);
    size_t println() { return print("\r\n"); };
    size_t println(std::string str);
    size_t println(int num, int base = 10);

    //size_t print(const char *format, ...);
    size_t print(const char *str);
    size_t print(const std::string &str);
    size_t print(int n, int base = 10);
    size_t print(unsigned int n, int base = 10);
    size_t print(long n, int base = 10);
    size_t print(unsigned long n, int base = 10);
#else
    UARTManager();

    void begin(int baud);
    void end();
    bool poll(int ms);

    void suspend(int sec=5);
    bool initialized() { return _initialized; }

    void set_port(const char *device, int command_pin, int proceed_pin);
    const char* get_port(int &command_pin, int &proceed_pin);

    void set_baudrate(uint32_t baud);
    uint32_t get_baudrate() { return _baud; }

    bool command_asserted();
    bool motor_asserted() { return false; } // not pin available
    void set_proceed(bool level);
    void set_interrupt(bool level) {} // not pin available

    int available();
    void flush();
    void flush_input();

    bool waitReadable(uint32_t timeout_ms);

    int read();
    size_t readBytes(uint8_t *buffer, size_t length, bool command_mode=false);
    size_t readBytes(char *buffer, size_t length) { return readBytes((uint8_t *)buffer, length); }

    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *s);

    size_t write(unsigned long n) { return write((uint8_t)n); }
    size_t write(long n) { return write((uint8_t)n); }
    size_t write(unsigned int n) { return write((uint8_t)n); }
    size_t write(int n) { return write((uint8_t)n); }

    // size_t printf(const char *format, ...);

    // //size_t println(const char *format, ...);
    // size_t println(const char *str);
    // size_t println() { return print("\n"); };
    // size_t println(std::string str);
    // size_t println(int num, int base = 10);

    // //size_t print(const char *format, ...);
    // size_t print(const char *str);
    // size_t print(const std::string &str);
    // size_t print(int n, int base = 10);
    // size_t print(unsigned int n, int base = 10);
    // size_t print(long n, int base = 10);
    // size_t print(unsigned long n, int base = 10);
#endif // ESP_PLATFORM
};

#ifdef ESP_PLATFORM
extern UARTManager fnUartDebug;
extern UARTManager fnUartBUS;
#endif

#endif //FNUART_H
