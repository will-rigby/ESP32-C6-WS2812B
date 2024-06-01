#define PIN_WS2812B 4     // The ESP32 pin GPIO16 connected to WS2812B
#define NUM_PIXELS 1024   // Number of pixels in strips or display panels

uint32_t leds[NUM_PIXELS];  // Array to store pixel rgb data

TaskHandle_t LEDTask;       // LED Task Handler

uint32_t drawBoolean = 0;

uint32_t set_color(uint8_t r, uint8_t g, uint8_t b){
  return (((uint32_t)g)<<16)|(((uint32_t)r)<<8)|((uint32_t)b);
}

/* Clear all the pixels */
void clear_pixels(void){
  for(int i = 0; i < NUM_PIXELS; i++){
    leds[i] = 0;
  }
}

/* Function called by LED task to write the panel colours */
void neopixelWritePixels(uint8_t pin, uint32_t* data, uint32_t pixels){
#if SOC_RMT_SUPPORTED
  rmt_data_t led_data[24*NUM_PIXELS];

  // Verify if the pin used is RGB_BUILTIN and fix GPIO number
#ifdef RGB_BUILTIN
  pin = pin == RGB_BUILTIN ? pin - SOC_GPIO_PIN_COUNT : pin;
#endif
  if (!rmtInit(pin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000)) {
    log_e("RGB LED driver initialization failed for GPIO%d!", pin);
    return;
  }
  rmtSetEOT(4, 0);

  
  for(int pixel = 0; pixel < pixels; pixel++){
    int color[] = {(data[pixel]>>16)&0xFF, (data[pixel]>>8)&0xFF, data[pixel]&0xFF};  // Color coding is in order GREEN, RED, BLUE
    int i = 0;
    for (int col = 0; col < 3; col++ ) {
      for (int bit = 0; bit < 8; bit++) {
        int bitIndex = pixel*24 + (i%24);
        if ((color[col] & (1 << (7 - bit)))) {
          // HIGH bit
          led_data[bitIndex].level0 = 1; // T1H
          led_data[bitIndex].duration0 = 8; // 0.8us
          led_data[bitIndex].level1 = 0; // T1L
          led_data[bitIndex].duration1 = 4; // 0.4us
        } else {
          // LOW bit
          led_data[bitIndex].level0 = 1; // T0H
          led_data[bitIndex].duration0 = 4; // 0.4us
          led_data[bitIndex].level1 = 0; // T0L
          led_data[bitIndex].duration1 = 8; // 0.8us
        }
        i++;
      }
    }
  }
  rmtWrite(pin, led_data, RMT_SYMBOLS_OF(led_data), RMT_WAIT_FOR_EVER);
#else
    log_e("RMT is not supported on " CONFIG_IDF_TARGET);
#endif /* SOC_RMT_SUPPORTED */
}

/* Set a pixel on a NeoPixel Strip */
void draw_pixel_strip(int pixel, int r, int g, int b){
  leds[pixel] = set_color(r,g,b);
}

/* Set a pixel on a NeoPixel 16 x 16 panel.
  displayNum is the display panel number if wiring a number in series
  x and y are the pixel coordinates, these panels are wired to the serial data
  transfer snakes left and right with each alternating row, this function compensates for that */
void draw_pixel(int displayNum, int x, int y, int r, int g, int b) {
  if ((y % 2) == 1) {
    x = 15 - x;
  }
  int pixel = (256 * displayNum) + (16 * y) + x;
  leds[pixel] = set_color(r,g,b);
}

/* Draw a horizontal line on a display panel */
void draw_h_line(int displayNum, int x, int y, int len, int r, int g, int b) {
  for (int i = 0; i < len; i++) {
    draw_pixel(displayNum, x + i, y, r, g, b);
  }
}

/* Draw a vertical line on a display panel */
void draw_v_line(int displayNum, int x, int y, int len, int r, int g, int b) {
  for (int i = 0; i < len; i++) {
    draw_pixel(displayNum, x, y + i, r, g, b);
  }
}

/* Fill in a rectangle on a display panel 
  x,y origin corner of rectangle
  w,h width and height
*/
void fill_rect(int displayNum, int x, int y, int w, int h, int r, int g, int b) {
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      draw_pixel(displayNum, x + i, y + j, r, g, b);
    }
  }
}

/* Example method being called by led task to draw some pixels based on a global variable */
void drawDisplay(void){
  if(drawBoolean){
    draw_pixel(0, 0, 0, 1,1,1);
  }
}

void setup() {
  /* Runs this NeoPixel LED handler task on the second core so it does not cause
  watch dog resets due to interrupting WiFi tasks etc. */
  xTaskCreatePinnedToCore(
                  LEDTaskCode,   /* Task function. */
                  "LEDTask",     /* name of task. */
                  100000,       /* Stack size of task */
                  NULL,        /* parameter of the task */
                  1,           /* priority of the task */
                  &LEDTask,      /* Task handle to keep track of created task */
                  1);          /* pin task to core 1 */

}

void LEDTaskCode( void * parameter) {
  for(;;) {
    clear_pixels();
    drawDisplay();
    yield();
    neopixelWritePixels(PIN_WS2812B, leds, NUM_PIXELS);
  }
}

void loop() {
  /* alternate global variable on and off to set display */
  drawBoolean = 0;
  delay(1000);
  drawBoolean = 1;
  delay(1000);
}
