#include "myws2812b.h"
#include "SPI.h"
#define CODE_1 (0xF8)
#define CODE_0 (0xC0)
uint8_t brightness = 50;
static uint8_t LED_MATRIX[64][3] = { 0 };
void myWs2812b::Writebit(uint8_t bit) {
  
  if (bit) {
    SPI.transfer(CODE_1);
  } else {
    SPI.transfer(CODE_0);
  }
}

void myWs2812b::WriteColor(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    Writebit(g >> (7 - i));
  }
  for (uint8_t i = 0; i < 8; i++) {
    Writebit(r >> (7 - i));
  }
  for (uint8_t i = 0; i < 8; i++) {
    Writebit(b >> (7 - i));
  }
}
void myWs2812b::Show() {
  for (uint8_t i = 0; i < 64; i++) {
    WriteColor(LED_MATRIX[i][0], LED_MATRIX[i][1], LED_MATRIX[i][2]);
  }
  Reset();
}
void myWs2812b::Clear() {

  for (uint8_t i = 0; i < 64; i++) {
    LED_MATRIX[i][0] = 0;
    LED_MATRIX[i][1] = 0;
    LED_MATRIX[i][2] = 0;
    WriteColor(0, 0, 0);
  }
  Reset();
}
void myWs2812b::Reset() {
  for (int i = 0; i < 5; i++) {
    SPI.transfer(0);
  }
}
void myWs2812b::SetLedColor(uint8_t ledid, uint8_t r, uint8_t g, uint8_t b) {
  LED_MATRIX[ledid][0] = r;
  LED_MATRIX[ledid][1] = g;
  LED_MATRIX[ledid][2] = b;
}
void myWs2812b::Setbrightness(uint8_t brightness) {
}










