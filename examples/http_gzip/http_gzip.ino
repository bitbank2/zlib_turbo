//
// zlib_turbo example showing how to decompress gzip
// data received from a HTTP request
// written by Larry Bank (bitbank@pobox.com)
// July 8, 2024
//
// This particular example pings a US grocery store chain (Publix)
// Their website returns a relatively small html response and accepts
// gzip as a Content-Encoding option
//
#include <WiFi.h>
#include <HTTPClient.h>
#include <zlib_turbo.h>

zlib_turbo zt;
HTTPClient http;
const char *url = "https://www.publix.com";
const char *SSID = "your_ssid_name";
const char *PW = "your_wifi_password";
// 10 seconds WIFI connection timeout
#define TIMEOUT 20

void setup()
{
int iTimeout, httpCode;
uint8_t *pCompressed, *pUncompressed;
long l, iCount, iPayloadSize, iUncompSize;
WiFiClient * stream;

   Serial.begin(115200);
   delay(2000); // give a moment for serial to start
   Serial.println("HTTP GZIP Example");
   // Connect to wifi
    WiFi.begin(SSID, PW);
    iTimeout = 0;
    Serial.print("Connecting to wifi");
    while (WiFi.status() != WL_CONNECTED && WiFi.status() != WL_CONNECT_FAILED && iTimeout < TIMEOUT)
    {
      delay(500); // allow up to 10 seconds to connect
      iTimeout++;
      Serial.print(".");
    }
    if (iTimeout == TIMEOUT) {
      Serial.println("\nConnection timed out!");
    } else {
      Serial.println("\Connected!");
      Serial.printf("Sending GET request to %s\n", url);
      http.begin(url);
      http.setAcceptEncoding("gzip"); // ask for response to be compressed
      httpCode = http.GET();  //send GET request
      if (httpCode != 200) {
          Serial.print("Error on HTTP request: ");
          Serial.println(httpCode);
          http.end();
      } else {
          Serial.println("GET request succeeded (return code = 200)");
          iPayloadSize = http.getSize();
          Serial.printf("payload size = %d\n", iPayloadSize);
          // Allocate a buffer to receive the compressed (gzip) response
          pCompressed = (uint8_t *)malloc(iPayloadSize+8); // allow a little extra for reading past the end
          l = millis();
          stream = http.getStreamPtr();
          iCount = 0;
          // Allow 4 seconds to receive the compressed data
          while (iCount < iPayloadSize && (millis() - l) < 4000) {
             if (stream->available()) {
                 char c = stream->read();
                 pCompressed[iCount++] = c;
             } else {
                 vTaskDelay(5); // allow time for data to receive
             }
          } // while
          http.end(); // we're done, close the connection
          // I tried parsing the HTTP response headers, but the HTTPClient library
          // seemed to crash when I did that, so we'll just look for the gzip
          // file signature
          if (pCompressed[0] == 0x1f && pCompressed[1] == 0x8b) {
            Serial.println("It's a gzip file!");
            iUncompSize = zt.gzip_info(pCompressed, iPayloadSize);
            if (iUncompSize > 0) {
               pUncompressed = (uint8_t *)malloc(iUncompSize+8);
               int rc = zt.gunzip(pCompressed, iPayloadSize, pUncompressed);
               if (rc == ZT_SUCCESS) {
                  Serial.printf("Uncompressed size = %d bytes\n", iUncompSize);
                  Serial.println("First 200 bytes of data:");
                  pUncompressed[200] = 0;
                  Serial.println((char *)pUncompressed);
                  free(pUncompressed);
               }
            }
          } else {
            Serial.println("It's not a gzip file, something went wrong :(");
          }
          free(pCompressed);
      } // http connection succeeded
      WiFi.disconnect();
    } // wifi connection
} /* setup() */

void loop()
{
} /* loop() */