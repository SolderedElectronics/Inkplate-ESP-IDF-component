#pragma once
// Minimal Print class for Adafruit_GFX under ESP-IDF (no Arduino framework)
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef const char __FlashStringHelper;

// Minimal String class (only what Adafruit_GFX needs: length() and c_str())
class String
{
  public:
    String() : _str(nullptr), _len(0) {}
    String(const char *s)
    {
        if (s) { _len = strlen(s); _str = (char *)malloc(_len + 1); if (_str) memcpy(_str, s, _len + 1); }
        else   { _str = nullptr; _len = 0; }
    }
    ~String() { if (_str) free(_str); }
    const char *c_str()  const { return _str ? _str : ""; }
    size_t      length() const { return _len; }
    char operator[](size_t i) const { return _str ? _str[i] : '\0'; }
  private:
    char  *_str;
    size_t _len;
};

class Print
{
  public:
    virtual ~Print() {}

    virtual size_t write(uint8_t c) = 0;

    size_t write(const char *str)
    {
        if (!str) return 0;
        size_t n = 0;
        while (*str) n += write((uint8_t)*str++);
        return n;
    }

    size_t write(const uint8_t *buf, size_t size)
    {
        size_t n = 0;
        while (size--) n += write(*buf++);
        return n;
    }

    size_t print(const char *str)  { return write(str ? str : ""); }
    size_t print(char c)           { return write((uint8_t)c); }
    size_t print(const String &s)  { return write(s.c_str()); }

    size_t print(int n, int base = 10)
    {
        char buf[32]; snprintf(buf, sizeof(buf), base == 16 ? "%x" : "%d", n); return write(buf);
    }
    size_t print(unsigned int n, int base = 10)
    {
        char buf[32]; snprintf(buf, sizeof(buf), base == 16 ? "%x" : "%u", n); return write(buf);
    }
    size_t print(long n, int base = 10)
    {
        char buf[32]; snprintf(buf, sizeof(buf), base == 16 ? "%lx" : "%ld", n); return write(buf);
    }
    size_t print(unsigned long n, int base = 10)
    {
        char buf[32]; snprintf(buf, sizeof(buf), base == 16 ? "%lx" : "%lu", n); return write(buf);
    }
    size_t print(double n, int digits = 2)
    {
        char buf[32]; snprintf(buf, sizeof(buf), "%.*f", digits, n); return write(buf);
    }

    size_t println(const char *str)  { size_t r = print(str);  r += write('\n'); return r; }
    size_t println(char c)           { size_t r = print(c);    r += write('\n'); return r; }
    size_t println(int n, int b = 10){ size_t r = print(n, b); r += write('\n'); return r; }
    size_t println(unsigned int n, int b = 10) { size_t r = print(n, b); r += write('\n'); return r; }
    size_t println(long n, int b = 10)         { size_t r = print(n, b); r += write('\n'); return r; }
    size_t println(unsigned long n, int b = 10){ size_t r = print(n, b); r += write('\n'); return r; }
    size_t println(double n, int d = 2)        { size_t r = print(n, d); r += write('\n'); return r; }
    size_t println(const String &s)            { size_t r = print(s);    r += write('\n'); return r; }
    size_t println()                           { return write('\n'); }
};
