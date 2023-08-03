#ifndef MY_WS2812B_H
#define MY_WS2812B_H
#include <Arduino.h>
class myWs2812b {
public:
  void SetLedColor(uint8_t ledid, uint8_t r, uint8_t g, uint8_t b);
  void Reset();
  void Clear();
  void Show();
  void Writebit(uint8_t bit);
  void WriteColor(uint8_t r, uint8_t g, uint8_t b);
private:
  uint8_t brightness;
};

#endif
