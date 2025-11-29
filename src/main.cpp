

#include <Arduino.h>
#include <WiFiS3.h>

const int motorPin = 9;
const int typePin = 7;
const char* TARGET_PREFIX = "NG_";

// Настройки вибрации
const int ZONE_FAR_RSSI = -50;      // ~2 метра
const int ZONE_MEDIUM_RSSI = -48;   // ~1 метр  
const int ZONE_CLOSE_RSSI = -45;    // ~0.5 метра
const int ZONE_VERY_CLOSE_RSSI = -42; // ~0.2 метра

const int VIBRATION_FAR = 60;
const int VIBRATION_MEDIUM = 120;
const int VIBRATION_CLOSE = 200;
const int VIBRATION_VERY_CLOSE = 255;

// Переменные для быстрого отслеживания
int previousRSSI = -100;
unsigned long lastScanTime = 0;
const int SCAN_INTERVAL = 50; // 20 раз в секунду!
int rssiSamples[3] = {0}; // Меньший буфер для скорости
int sampleIndex = 0;
int lastVibration = -1;


// Forward declarations: ensure functions used in loop/setup are visible
void processNavigation();
int findStrongestRSSIID();
int calculateSmoothedRSSI();
void controlVibration(int rssi, char type);


void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  pinMode(motorPin, OUTPUT);
  pinMode(typePin, OUTPUT);

  analogWrite(motorPin, 0);
  analogWrite(typePin, 0);
  
  Serial.println("🚀 Belt Device - Ultra Fast Mode");
  
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("❌ WiFi module failed!");
    while (true);
  }
  
  Serial.println("✅ Ultra fast mode activated (20Hz)");
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastScanTime >= SCAN_INTERVAL) {
    processNavigation();
    lastScanTime = currentTime;
  }
}

void processNavigation() {
  int currentRSSIID = findStrongestRSSIID();
    if (currentRSSIID == -1) {
      analogWrite(motorPin, 0);
      analogWrite(typePin, 0);
        Serial.println("NO_SENSORS");
        return;
    }
    int currentRSSI = WiFi.RSSI(currentRSSIID);
  if (currentRSSI <= -85) {
    if (lastVibration != 0) {
      analogWrite(motorPin, 0);
      analogWrite(typePin, 0);
      lastVibration = 0;
      Serial.println("NO_SENSORS");
    }
    return;
  }
  
  // Быстрое усреднение (3 samples)
  rssiSamples[sampleIndex] = currentRSSI;
  sampleIndex = (sampleIndex + 1) % 3;
  String rssiName = WiFi.SSID(currentRSSIID);
  String msg = String("SENSOR:") + rssiName + ":RSSI:" + String(currentRSSI);
  Serial.println(msg);
  int smoothedRSSI = calculateSmoothedRSSI();
  char type = rssiName.charAt(rssiName.length() - 1); // Последний символ SSID как тип
  controlVibration(smoothedRSSI, type);
}

int calculateSmoothedRSSI() {
  int sum = 0;
  int count = 0;
  for (int i = 0; i < 3; i++) {
    if (rssiSamples[i] != 0) {
      sum += rssiSamples[i];
      count++;
    }
  }
  return count > 0 ? sum / count : -100;
}

int findStrongestRSSIID() {
  int numNetworks = WiFi.scanNetworks();
  int strongestRSSI = -100;
  int strongestRSSIID = -1;
  
  // Максимально быстрое сканирование
  for (int i = 0; i < numNetworks; i++) {
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    
    if (ssid.startsWith(TARGET_PREFIX) && rssi > strongestRSSI) {
      strongestRSSI = rssi;
      strongestRSSIID = i;
    }
  }
  
  return strongestRSSIID;
}

void controlVibration(int rssi, char type) {
  int vibrationPower = 0;
  
  // Быстрое определение зоны
  if (rssi >= ZONE_VERY_CLOSE_RSSI) {
    vibrationPower = VIBRATION_VERY_CLOSE;
  } else if (rssi >= ZONE_CLOSE_RSSI) {
    vibrationPower = map(rssi, ZONE_CLOSE_RSSI, ZONE_VERY_CLOSE_RSSI, 
                        VIBRATION_CLOSE, VIBRATION_VERY_CLOSE);
  } else if (rssi >= ZONE_MEDIUM_RSSI) {
    vibrationPower = map(rssi, ZONE_MEDIUM_RSSI, ZONE_CLOSE_RSSI,
                        VIBRATION_MEDIUM, VIBRATION_CLOSE);
  } else if (rssi >= ZONE_FAR_RSSI) {
    vibrationPower = map(rssi, ZONE_FAR_RSSI, ZONE_MEDIUM_RSSI,
                        VIBRATION_FAR, VIBRATION_MEDIUM);
  }
  
  vibrationPower = constrain(vibrationPower, 0, 255);
  
  // Обновляем вибрацию только при изменении
  if (vibrationPower != lastVibration) {
    analogWrite(motorPin, vibrationPower);
    lastVibration = vibrationPower;
    
    // Минимальный вывод для скорости
    Serial.println(vibrationPower);
  }
if (vibrationPower > 0 && type != '-') {
    static uint8_t sPulseState = 0;
    static unsigned long sPulseStart = 0;
    static char sLastType = 0;

    // reset pulse state when type changes or vibration stops
    if (type != sLastType || vibrationPower == 0) {
        sPulseState = 0;
        sPulseStart = 0;
        sLastType = type;
    }

    if (type == 'W') {
        digitalWrite(typePin, HIGH);
        // force strong vibration for Wall
        if (lastVibration != 255) {
            analogWrite(motorPin, 255);
            lastVibration = 255;
        }
        Serial.println("TYPE:Wall");
    } else if (type == 'S') {
        digitalWrite(typePin, LOW);
        // Two short non-blocking pulses for Stairs
        const unsigned long ON_MS = 60;
        const unsigned long OFF_MS = 60;
        unsigned long now = millis();

        if (sPulseState == 0) {
            // start first pulse
            analogWrite(motorPin, VIBRATION_CLOSE);
            lastVibration = VIBRATION_CLOSE;
            sPulseStart = now;
            sPulseState = 1;
            Serial.println("TYPE:Stairs");
        } else if (sPulseState == 1) {
            if (now - sPulseStart >= ON_MS) {
                analogWrite(motorPin, 0);
                lastVibration = 0;
                sPulseStart = now;
                sPulseState = 2;
            }
        } else if (sPulseState == 2) {
            if (now - sPulseStart >= OFF_MS) {
                analogWrite(motorPin, VIBRATION_CLOSE);
                lastVibration = VIBRATION_CLOSE;
                sPulseStart = now;
                sPulseState = 3;
            }
        } else if (sPulseState == 3) {
            if (now - sPulseStart >= ON_MS) {
                analogWrite(motorPin, 0);
                lastVibration = 0;
                // finished sequence - reset so next trigger can start again
                sPulseState = 0;
            }
        }
    }
}
}