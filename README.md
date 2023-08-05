# Air001_Music_Spectrum
## 材料 
1. 麦克风：MAX4466
2. 亚克力板：67 x 67 x 2 mm 黑茶色 透明非磨砂
3. Ws2812灯板： 65 x 65 mm  8 x 8 = 64 LEDs
4. 均光：普通硫酸纸或者用A4纸
5. MCU：合宙Air001
## 注意事项
 1. ws2812B灯珠的排列方向不是蛇形走位，如果买到蛇形走位的，需要在程序中稍作修改
 2. 软件顶部工具栏，clock source and Frequency把Air001主频调到HSI 24 MHz，HCLK 48MHz的组合。
 3. 更改代码优化等级，顶部工具栏optimize修改为第二项（smallest（-OS）with LTO），不然会flash over flow
 4. ws2812b链接vbus或5v，不要连接3.3v
## 关于亮度的问题
> 可以在myws2812.cpp文件中等比例修改除数的值，从而改变灯的亮度,例如:
```cpp
void myWs2812b::SetLedColor(uint8_t ledid, uint8_t r, uint8_t g, uint8_t b) {
  // 这里可以更改后面的数值调节亮度，不除的话，亮度比较高，发热比较严重
  LED_MATRIX[ledid][0] = r/5;
  LED_MATRIX[ledid][1] = g/5;
  LED_MATRIX[ledid][2] = b/5;
}
```
