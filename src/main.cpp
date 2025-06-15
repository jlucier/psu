#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

U8G2_SSD1309_128X64_NONAME2_1_HW_I2C display(U8G2_R0);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  display.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Hello...");
  display.firstPage();
  do {
    display.setFont(u8g2_font_6x13_tf);
    display.drawStr(0,20,"Hello World!");
  } while ( display.nextPage() );
  delay(1000);
}
