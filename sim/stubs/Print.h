#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstddef>
class Print {
 public:
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t *b, size_t n) {
    size_t c = 0; while (n--) c += write(*b++); return c;
  }
  size_t print(const char *s) { return write((const uint8_t*)s, strlen(s)); }
  size_t print(char c) { return write((uint8_t)c); }
  size_t print(int v)  { char b[16]; int n = snprintf(b,sizeof b,"%d",v);
                         return write((const uint8_t*)b,(size_t)n); }
  size_t print(unsigned v) { char b[16]; int n = snprintf(b,sizeof b,"%u",v);
                             return write((const uint8_t*)b,(size_t)n); }
  size_t println(const char *s) { return print(s) + write((uint8_t)'\n'); }
  size_t printf(const char *f, ...) {
    char b[256]; va_list a; va_start(a,f);
    int n = vsnprintf(b,sizeof b,f,a); va_end(a);
    return write((const uint8_t*)b,(size_t)(n<0?0:n));
  }
};
