#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <INA226_WE.h>

#define DISPLAY_REFRESH_MILLIS 500
#define SENSE_MILLIS 250
// #define PRINT_TIMING

unsigned long last_sense = 0;
unsigned long last_render = 0;

float shuntVoltage_mV = 0.0;
float loadVoltage_V = 0.0;
float busVoltage_V = 0.0;
float current_mA = 0.0;
float power_mW = 0.0; 

U8G2_SSD1309_128X64_NONAME0_2_HW_I2C display(U8G2_R0);
INA226_WE ina226;

void break_line() {
  display.setCursor(0, display.getCursorY() + display.getMaxCharHeight());
}

void print_labeled(float value, const char* label, int precision=3, bool newline=true) {
  if (value >= 0)
    display.print(" ");
  display.print(value, 3);

  int w = display.getWidth();
  display.setCursor(w - display.getStrWidth(label), display.getCursorY());
  display.print(label);
  if (newline)
    break_line();
}

void print_milli(float val, const char* unit, bool newline=true) {
  static char final[50];
  if (val > 1000) {
    print_labeled(val / 1000, unit, newline);
  } else {
    sprintf(final, "m%s", unit);
    print_labeled(val, final, newline);
  }
}

void gui_update() {
  display.firstPage();
  do {
    // display.setFont(u8g2_font_crox3hb_tf);
    display.setFont(u8g2_font_sisterserif_tr);
    display.setCursor(0, 18);

    print_labeled(loadVoltage_V, "V");
    // print_labeled(busVoltage_V, "V (Bus)");
    print_milli(current_mA, "A");
    print_milli(power_mW, "W");
    print_milli(shuntVoltage_mV, "V (sh)");
    // print_labeled("Update", millis());
  } while ( display.nextPage() );
}

void sense() {
  ina226.readAndClearFlags();
  shuntVoltage_mV = ina226.getShuntVoltage_mV();
  busVoltage_V = ina226.getBusVoltage_V();
  current_mA = ina226.getCurrent_mA();
  power_mW = ina226.getBusPower();
  loadVoltage_V  = busVoltage_V + (shuntVoltage_mV/1000);

  if(ina226.overflow){
    Serial.println("Overflow! Choose higher current range");
  }
}


void setup() {
  Serial.begin(9600);
  Wire.begin();

  // display.setBusClock(100000);
  display.begin();

  ina226.init();
  ina226.setCorrectionFactor(0.6);
  ina226.setResistorRange(0.100, 3.5);
  ina226.setAverage(AVERAGE_16);
  ina226.setConversionTime(CONV_TIME_8244);
  ina226.waitUntilConversionCompleted();
}

void loop() {
  unsigned long now = millis();
  
  if (now - last_sense >= SENSE_MILLIS) {
    last_sense = now;
    sense();
#ifdef PRINT_TIMING
    Serial.print("Sense took: ");
    Serial.println(millis() - now);
#endif
  }

  if (now - last_render >= DISPLAY_REFRESH_MILLIS) {
    last_render = now;
    gui_update();
#ifdef PRINT_TIMING
    Serial.print("Draw took: ");
    Serial.println(millis() - now);
#endif
  }

  delay(min(SENSE_MILLIS, DISPLAY_REFRESH_MILLIS));
}
