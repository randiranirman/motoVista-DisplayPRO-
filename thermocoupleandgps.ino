#include <TinyGPS++.h>
#include <LiquidCrystal_I2C.h>
#include "max6675.h"

#define RXD2 16
#define TXD2 17

HardwareSerial neogps(1); // GPS module serial
int thermoDO = 19;
int thermoCS = 23;
int thermoCLK = 5;

TinyGPSPlus gps; // GPS object
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO); // Thermocouple object

// LCD setup
int lcdColumns = 16;
int lcdRows = 4;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  // Address 0x27 for LCD 1
LiquidCrystal_I2C lcd2(0x26, lcdColumns, lcdRows); // Address 0x26 for LCD 2

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  Serial.println("MAX6675 test");

  // Initialize LCDs
  lcd.init();
  lcd2.init();

  // Turn on LCD backlights
  lcd.backlight();
  lcd2.backlight();

  // Initialize GPS serial communication
  neogps.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("GPS Speed Test - Waiting for GPS fix...");

  // Optional delay to allow MAX6675 to stabilize
  delay(500);
}

void loop() {
  // Clear LCDs at the beginning of each loop
  lcd.clear();
  lcd2.clear();

  // Display GPS speed on lcd
  lcd.setCursor(0, 0);
  lcd.print("SPEED:");
  lcd.print(gps.speed.kmph());
  
  // Display temperature on lcd2
  lcd2.setCursor(0, 0);
  lcd2.print("TEMP: ");
  float temperature = thermocouple.readCelsius();
  lcd2.print(temperature);
  lcd2.print("C");

  // Display temperature status
  displayTempStatus(temperature);

  // Handle GPS data
  while (neogps.available() > 0) {
    char c = neogps.read();
    Serial.print(c); // Print raw GPS data to Serial Monitor
    if (gps.encode(c)) {
      displayInfo();
    }
  }

  // Display GPS status
  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());
  Serial.print("Location valid: ");
  Serial.println(gps.location.isValid() ? "Yes" : "No");

  // Delay between updates
  delay(1000);
}

void displayInfo() {
  if (gps.location.isValid()) {
    // Print GPS information to Serial Monitor
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("Speed (km/h): ");
    Serial.println(gps.speed.kmph());
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    Serial.print("Altitude: ");
    Serial.println(gps.altitude.meters());

    // Display speed on LCD
    displaySpeed(gps.speed.kmph());
  } else {
    Serial.println("Location not valid");
  }
}

void displayTempStatus(float temperature) {
  lcd2.setCursor(0, 2); // Set cursor to third row (index 2) of lcd2
  if (isnan(temperature)) {
    lcd2.print("STATUS: N/A     ");
  } else if (temperature < 60) {
    lcd2.print("STATUS: NORMAL  ");
  } else if (temperature >= 60 && temperature < 120) {
    lcd2.print("STATUS: GOOD    ");
  } else {
    lcd2.print("STATUS: WARNING ");
  }
}

void displaySpeed(float speed) {
  lcd.setCursor(0, 1);
  int numBars = map(speed, 0, 150, 0, 16); // Map speed to number of bars
  for (int i = 0; i < numBars; i++) {
    lcd.print((char)255); // Print solid block character for bar
  }
  for (int i = numBars; i < 16; i++) {
    lcd.print(" "); // Clear remaining part of the graph
  }
}
