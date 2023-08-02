#include "myws2812b.h"
#include "SPI.h"
#include <arduinoFFT.h>
#define MIC_PIN PA_0
#define SCREEN_WIDTH 8
#define SCREEN_HEIGHT 8
// 采样点数量
#define SAMPLES 32
// 采样频率
#define SAMPLINGFREQUENCY 4000  // 意味着声音频率只能采到5khz
// 幅值，512---1000，256---500
#define AMPLITUDE 15
// 噪声
#define NOISE 80
// 采样周期 
unsigned int sampling_period_us;
// 定义复数虚部和实部数组
double vReal[SAMPLES];
double vImag[SAMPLES];
// mic模拟变量
unsigned int micvalue;
// 保存先前时间
unsigned long pretime;
// 用来存储每个频段的总幅值
static double bandFrenquency[8] = { 0 };
// 用来存储上一次每隔频段的峰值
static uint8_t prebandpeak[8] = { 0 };
// 峰值，用于封顶下落动画
static uint8_t peak_temp[8] = { 0 };
uint8_t peak_color[3][3] = { { 0xff, 0xff, 0xff }, { 0xff, 0x61, 0 }, { 0, 0xc9, 0x57 } };
uint8_t band_color[8][3] = { { 0, 0xff, 0x7f }, { 0x7c, 0xfc, 0 }, { 0x22, 0x8b, 0x22 }, { 0, 0, 0xff },
                               { 0, 0xc7, 0x8c }, { 0xff, 0x45, 0 },{ 0xff, 0, 0 },{ 0xff, 0x63, 0x47 }};
unsigned char ledid;
uint8_t k = 0, m = 0, n = 0;  //用来模拟计时
void get_band_peak();
void drawBandwithoutpeak(int band, int bandheight);
void drawBandpeak(int band);
uint8_t LED_MATRIX[64][3] = { 0 };
myWs2812b WS2812b_Panel;
arduinoFFT fft = arduinoFFT(vReal, vImag, SAMPLES, SAMPLINGFREQUENCY);
void setup() {
  Serial.begin(115200);
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  sampling_period_us = round(1000000 * (1.0 / SAMPLINGFREQUENCY));
}

void loop() {
  n++;
  if (n == 200) {
    n = 0;m++;
    if (m == 3) m = 0;
  }

  WS2812b_Panel.Clear();
  get_band_peak();
  WS2812b_Panel.Show();
}
void get_band_peak() {
  for (int i = 0; i < 8; i++) {
    bandFrenquency[i] = 0;
  }
  // ADC采样
  for (int i = 0; i < SAMPLES; i++) {
    pretime = micros();
    analogReadResolution(12);
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0.0;
    // 如果ADC转化时间大于采样时间，就进行下一次采样，否则就等待
    while (micros() < (pretime + sampling_period_us))
      ;
  }
  fft.DCRemoval();
  // 窗函数，降低弥散影响
  fft.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  // 计算fft
  fft.Compute(FFT_FORWARD);
  // 使用虚值和实值计算幅值,幅值会存储到vReal数组中
  fft.ComplexToMagnitude();
  // fft.MajorPeak(vReal, SAMPLES, SAMPLINGFREQUENCY);
  // 进行频率划分，划分策略
  // 8 bands, 5kHz top band
  for (int i = 2; i < SAMPLES / 2; i++) {
    if (vReal[i] > NOISE) {

      //8 bands, 2kHz top band
      if (i <= 3) bandFrenquency[0] += (int)vReal[i];
      if (i > 3 && i <= 4) bandFrenquency[1] += (int)vReal[i];
      if (i > 4 && i <= 5) bandFrenquency[2] += (int)vReal[i];
      if (i > 5 && i <= 6) bandFrenquency[3] += (int)vReal[i];
      if (i > 6 && i <= 8) bandFrenquency[4] += (int)vReal[i];
      if (i > 8 && i <= 11) bandFrenquency[5] += (int)vReal[i];
      if (i > 11 && i <= 14) bandFrenquency[6] += (int)vReal[i];
      if (i > 14) bandFrenquency[7] += (int)vReal[i];
    }
  }
  for (uint8_t i = 0; i < 8; i++) {
    // 获取每隔频段的幅值，并缩放
    int bandheight = bandFrenquency[i] / AMPLITUDE;
    // 因为第一个band的振幅比较大，这里进行单独的缩放
    if (i == 0){
      bandheight = bandheight/3;
      if(bandheight>=1)bandheight-=1;
    }
    // if(i==0&&bandheight>=2) bandheight -=2;
    // if(i==1&&bandheight>=2) bandheight -=1;
    // 注意这里必须是SCREEN_HEIGHT，否则bandheight永远达不到顶峰
    if (bandheight > SCREEN_HEIGHT)
      bandheight = SCREEN_HEIGHT;
    // 使用两次的峰值，来平滑Band动画
    bandheight = (prebandpeak[i] + bandheight) / 2;
    if (bandheight > peak_temp[i]) {
      peak_temp[i] = min(SCREEN_HEIGHT, bandheight);
    }
    // 绘制band,但不绘制band的顶部
    drawBandwithoutpeak(i, bandheight);
    // 更新peak值,只有大于上次的峰值才会更新峰值
    // 绘制每条band的封顶
    drawBandpeak(i);

    // 使封顶跳动起来，需要保存两次峰值之间的最大值
    prebandpeak[i] = bandheight;
  }
  k++;
  if (k == 2) {
    k = 0;
    for (uint8_t i = 0; i < SCREEN_WIDTH; i++)
      if (peak_temp[i] > 0) peak_temp[i] -= 1;
  }
}
void drawBandwithoutpeak(int band, int bandheight) {
  // 绘制每一个条带，不包含每个条带的峰值
  if (bandheight != 0) {
    // 非蛇形走位，需要判断是奇数列，还是偶数列
    // uint8_t r = random(0,256);
    // uint8_t g = random(0,256);
    // uint8_t b = random(0,256);
    for (uint8_t i = 0; i < bandheight; i++) {
      // 计算每一个LED对应的顺序号（0~63）
      ledid = band * SCREEN_HEIGHT + i;
      WS2812b_Panel.SetLedColor(ledid, band_color[band][0],band_color[band][1], band_color[band][2]);
    }
  }
}
void drawBandpeak(int band) {
  // 计算每个峰顶LED的编号
  ledid = band * SCREEN_HEIGHT + peak_temp[band];
  WS2812b_Panel.SetLedColor(ledid, 0xff, 0xff, 0xff);
}