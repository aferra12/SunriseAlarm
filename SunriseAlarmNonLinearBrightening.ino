#include <RTClib.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#define LEDPIN 3
#define ONBUTTON 2

#define EEPROM_ADDRESS 0  // Starting address in EEPROM to store our reference date
#define EEPROM_VERSION_ADDRESS (EEPROM_ADDRESS + sizeof(ReferenceDateTime))  // Address to store the version

#define TWILIGHT_DURATION 1800000 // 30 minutes in milliseconds
#define SUNRISE_DURATION 1800000 // 30 minutes in milliseconds
#define POST_SUNRISE_DURATION 1800000 // 30 minutes in milliseconds
#define TOTAL_DURATION (TWILIGHT_DURATION + SUNRISE_DURATION + POST_SUNRISE_DURATION)
#define MAX_RUNTIME 10800000 // 3 hours in milliseconds

#define INITIAL_SUNRISE_HOUR 4    // Set your desired initial sunrise hour here (24-hour format)
#define INITIAL_SUNRISE_MINUTE 51 // Set your desired initial sunrise minute here

RTC_DS1307 rtc;

volatile bool should_reset = false;  // Flag to indicate if we should reset
unsigned long last_toggle_time = 0;  // To debounce the button

bool isSunriseGettingLater = true; // Set this to false when sunrise starts getting earlier

// Structure to hold our reference date and time
struct ReferenceDateTime {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
};

ReferenceDateTime referenceDateTime;
uint16_t currentVersion;

void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(ONBUTTON, INPUT_PULLUP);
  Serial.begin(57600);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // Uncomment the following line to set the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(__DATE__, __TIME__));
  }

  attachInterrupt(digitalPinToInterrupt(ONBUTTON), buttonPressed, FALLING);

  currentVersion = (INITIAL_SUNRISE_HOUR << 8) | INITIAL_SUNRISE_MINUTE;
  checkAndResetReferenceDate();

  loadReferenceDateTime();

  Serial.println("Enter 'c' to change sunrise direction.");
}

void loop() {
  if (should_reset) {
    resetAndRestart();
  }

  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == 'c') {
      toggleSunriseDirection();
    }
  }

  DateTime now = rtc.now();
  int alarmHour, alarmMinute;
  getAdjustedAlarmTime(now, alarmHour, alarmMinute);
  
  Serial.print("Current time: ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(" Alarm time: ");
  Serial.print(alarmHour);
  Serial.print(":");
  Serial.println(alarmMinute);

  if ((now.minute() == alarmMinute) && (now.hour() == alarmHour)) {
    Serial.println("Sunrise started");
    simulateSunrise();
  } else {
    delay(1000);
  }
}

void simulateSunrise() {
  unsigned long startTime = millis();
  while (millis() - startTime < MAX_RUNTIME && !should_reset) {
    unsigned long elapsedTime = millis() - startTime;
    
    int brightness = calculateBrightness(min(elapsedTime, TOTAL_DURATION));
    analogWrite(LEDPIN, brightness);
    
    delay(100);  // Update every 100ms
  }
  
  // Ensure LED is off after the simulation
  analogWrite(LEDPIN, 0);
}

int calculateBrightness(unsigned long elapsedTime) {
  float progress = (float)elapsedTime / TOTAL_DURATION;
  
  if (progress < 0.3333) { // Civil twilight
    return map(elapsedTime, 0, TWILIGHT_DURATION, 0, 64);
  } else if (progress < 0.6666) { // Rapid sunrise
    float x = (float)(elapsedTime - TWILIGHT_DURATION) / SUNRISE_DURATION;
    float k = 12; // Steepness of the curve
    float x0 = 0.5; // Midpoint of the curve
    return 64 + (int)(191 / (1 + exp(-k * (x - x0))));
  } else { // Post-sunrise
    return map(elapsedTime, TWILIGHT_DURATION + SUNRISE_DURATION, TOTAL_DURATION, 255, 255);
  }
}

void buttonPressed() {
  // Debounce
  if (millis() - last_toggle_time > 50) {
    should_reset = true;
    last_toggle_time = millis();
  }
}

void resetAndRestart() {
  // Save the current time as the new reference
  DateTime now = rtc.now();
  saveReferenceDateTime(now);
  
  // Increment the sunrise time by 1 minute
  int newMinute = (referenceDateTime.minute + 1) % 60;
  int newHour = referenceDateTime.hour;
  if (newMinute == 0) {
    newHour = (newHour + 1) % 24;
  }
  
  referenceDateTime.hour = newHour;
  referenceDateTime.minute = newMinute;
  EEPROM.put(EEPROM_ADDRESS, referenceDateTime);
  
  Serial.println("Resetting and restarting...");
  delay(1000);  // Give some time for the serial message to be sent
  
  // Reset the Arduino
  wdt_enable(WDTO_15MS);
  while(1) {}
}

void saveReferenceDateTime(const DateTime& dt) {
  referenceDateTime.year = dt.year();
  referenceDateTime.month = dt.month();
  referenceDateTime.day = dt.day();
  referenceDateTime.hour = dt.hour();
  referenceDateTime.minute = dt.minute();
  EEPROM.put(EEPROM_ADDRESS, referenceDateTime);
}

void loadReferenceDateTime() {
  EEPROM.get(EEPROM_ADDRESS, referenceDateTime);
  // If the stored year is invalid (e.g., first run), set it to the current date with initial sunrise time
  if (referenceDateTime.year < 2000 || referenceDateTime.year > 2099) {
    DateTime now = rtc.now();
    referenceDateTime.year = now.year();
    referenceDateTime.month = now.month();
    referenceDateTime.day = now.day();
    referenceDateTime.hour = INITIAL_SUNRISE_HOUR;
    referenceDateTime.minute = INITIAL_SUNRISE_MINUTE;
    EEPROM.put(EEPROM_ADDRESS, referenceDateTime);
    EEPROM.put(EEPROM_VERSION_ADDRESS, currentVersion);
    Serial.println("Initial sunrise time set.");
  }
  Serial.print("Reference sunrise time: ");
  Serial.print(referenceDateTime.hour);
  Serial.print(":");
  Serial.println(referenceDateTime.minute);
}

long daysSinceReference(const DateTime& current) {
  DateTime reference(referenceDateTime.year, referenceDateTime.month, referenceDateTime.day, 
                     referenceDateTime.hour, referenceDateTime.minute, 0);
  TimeSpan diff = current - reference;
  return diff.days();
}

void getAdjustedAlarmTime(const DateTime& current, int& hour, int& minute) {
  long days = daysSinceReference(current);
  int totalMinutes = referenceDateTime.hour * 60 + referenceDateTime.minute;
  
  if (isSunriseGettingLater) {
    totalMinutes += days; // Sunrise gets later each day
  } else {
    totalMinutes -= days; // Sunrise gets earlier each day
  }
  
  // Ensure totalMinutes stays within 0-1439 range (0-23:59)
  while (totalMinutes < 0) totalMinutes += 1440;
  totalMinutes %= 1440;
  
  hour = totalMinutes / 60;
  minute = totalMinutes % 60;
}

void toggleSunriseDirection() {
  isSunriseGettingLater = !isSunriseGettingLater;
  // Reset the reference date when we change direction
  DateTime now = rtc.now();
  saveReferenceDateTime(now);
  Serial.print("Sunrise is now getting ");
  Serial.println(isSunriseGettingLater ? "later" : "earlier");
  Serial.println("Reference date reset to current date");
}

void checkAndResetReferenceDate() {
  uint16_t storedVersion;
  EEPROM.get(EEPROM_VERSION_ADDRESS, storedVersion);
  
  if (storedVersion != currentVersion) {
    DateTime now = rtc.now();
    referenceDateTime.year = now.year();
    referenceDateTime.month = now.month();
    referenceDateTime.day = now.day();
    referenceDateTime.hour = INITIAL_SUNRISE_HOUR;
    referenceDateTime.minute = INITIAL_SUNRISE_MINUTE;
    EEPROM.put(EEPROM_ADDRESS, referenceDateTime);
    EEPROM.put(EEPROM_VERSION_ADDRESS, currentVersion);
    Serial.println("Initial sunrise time updated and reference date reset.");
  }
}
