#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <INA226_WE.h>

#define DISPLAY_REFRESH_MILLIS 500
#define SENSE_MILLIS 100
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

template<typename T>
void print_labeled(const char* label, T value, bool newline=true) {
  display.print(label);
  display.print(": ");
  display.print(value);
  if (newline)
    break_line();
}

void print_milli(const char* label, char unit, float val, bool newline=true) {
  static char final[50];
  if (val > 1000) {
    sprintf(final, "%s [%c]", label, unit);
    print_labeled(final, val / 1000, newline);
  } else {
    sprintf(final, "%s [m%c]", label, unit);
    print_labeled(final, val, newline);
  }
}

void gui_update() {
  display.firstPage();
  do {
    display.setFont(u8g2_font_t0_11_tf);
    display.setCursor(0, 10);

    // print_labeled("Shunt [mV]", shuntVoltage_mV);
    // print_labeled("Bus [V]", busVoltage_V);
    print_labeled("Load [V]", loadVoltage_V);
    print_milli("Current", 'A', current_mA);
    print_milli("Power", 'W', power_mW);
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
  ina226.setAverage(AVERAGE_64);
  ina226.setConversionTime(CONV_TIME_588);
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
