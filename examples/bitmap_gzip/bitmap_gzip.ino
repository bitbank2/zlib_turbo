//
// gzip bitmap example
//
// by Larry Bank (bitbank@pobox.com)
// This example program shows how to keep compressed art assets on your MCU
// and decompress them quickly into usable images
// The source image is from LVGL and is a 100x100xRGB565 raw image of a cog wheel
// The test platform is a "CYD - cheap yellow display, ESP32 + ILI9341 LCD"
//
#include <bb_spi_lcd.h>
#include <zlib_turbo.h>
#include "cogwheel_100x100.h"

BB_SPI_LCD lcd;
zlib_turbo zt;

void setup()
{
  int rc, iUncompSize;
  uint8_t *pUncompressed;
  long l;

  lcd.begin(DISPLAY_CYD); // customize for your device
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN);
  lcd.setFont(FONT_12x16);
  lcd.println("gzip Bitmap example");
  lcd.printf("compressed size = %d\n", sizeof(cogwheel_100x100));

// Ask the zlib_turbo library to tell us how big the uncompressed data will be
// a non-zero size means the gzip data is valid
  iUncompSize = zt.gzip_info((uint8_t *)cogwheel_100x100, sizeof(cogwheel_100x100));
  if (iUncompSize > 0) {
    lcd.printf("uncompressed size = %d\n", iUncompSize);
    // Allocate a buffer to hold the data; the extra 4 bytes is because of an optimization
    // in zlib_turbo that writes unaligned longs when possible to speed up decoding
    pUncompressed = (uint8_t *)malloc(iUncompSize+4);
    // Measure the decode time in microseconds
    l = micros();
    rc = zt.gunzip((uint8_t *)cogwheel_100x100, sizeof(cogwheel_100x100), pUncompressed);
    l = micros() - l;
    lcd.printf("decoded in %d microsecs\n", (int)l);
    if (rc == ZT_SUCCESS) { 
        uint16_t *p16 = (uint16_t *)pUncompressed;
        for (int i=0; i<iUncompSize/sizeof(uint16_t); i++) { // convert to big endian for bb_spi_lcd
          p16[i] = __builtin_bswap16(p16[i]);
        }
        // Display raw RGB565 pixels (x, y, width, height, source_pointer)
        lcd.pushImage(0, 100, 100, 100,(uint16_t *)pUncompressed);
    } // successful decode
    free(pUncompressed);
  } // valid gzip
} // setup

void loop()
{
}