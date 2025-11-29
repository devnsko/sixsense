// // *** The Sixth Sense City ***
// // prototype V2P (Vehicle-to-Pedestrian) или I2P (Infrastructure-to-Pedestrian)
// #include <Arduino.h>

// constexpr uint8_t VIBRO_LEFT_PIN  = 5;
// constexpr uint8_t VIBRO_RIGHT_PIN = 6;
// constexpr uint8_t VIBRO_PWM_MAX   = 255;

// // Call once at startup
// void VibroInit() {
//     pinMode(VIBRO_LEFT_PIN, OUTPUT);
//     pinMode(VIBRO_RIGHT_PIN, OUTPUT);
//     analogWrite(VIBRO_LEFT_PIN, 0);
//     analogWrite(VIBRO_RIGHT_PIN, 0);
// }

// static inline void setVibro(uint8_t pin, uint8_t intensity) {
//     analogWrite(pin, intensity);
// }

// // Left motor: intensity 0-255, duration in ms
// void VibroLeft(uint8_t intensity = 200, unsigned long durationMs = 200) {
//     setVibro(VIBRO_LEFT_PIN, intensity);
//     delay(durationMs);
//     setVibro(VIBRO_LEFT_PIN, 0);
// }

// // Right motor
// void VibroRight(uint8_t intensity = 200, unsigned long durationMs = 200) {
//     setVibro(VIBRO_RIGHT_PIN, intensity);
//     delay(durationMs);
//     setVibro(VIBRO_RIGHT_PIN, 0);
// }

// // Both motors
// void VibroBoth(uint8_t intensity = 200, unsigned long durationMs = 200) {
//     setVibro(VIBRO_LEFT_PIN, intensity);
//     setVibro(VIBRO_RIGHT_PIN, intensity);
//     delay(durationMs);
//     setVibro(VIBRO_LEFT_PIN, 0);
//     setVibro(VIBRO_RIGHT_PIN, 0);
// }

// // Short both: quick pulse
// void VibroShortBoth() {
//     VibroBoth(180, 100);
// }

// // Long strong both: full power long pulse
// void VibroLongStrongBoth() {
//     VibroBoth(VIBRO_PWM_MAX, 1000);
// }

// void setup() {
//     VibroInit();
//     // Example usage
//     VibroShortBoth();
//     delay(500);
//     VibroLongStrongBoth();
// }

// void loop() {
//     static unsigned long lastShort = 0;
//     static unsigned long lastLong  = 0;
//     const unsigned long SHORT_INTERVAL = 2000;  // ms between short pulses
//     const unsigned long LONG_INTERVAL  = 10000; // ms between long strong pulses

//     unsigned long now = millis();

//     if (now - lastShort >= SHORT_INTERVAL) {
//         lastShort = now;
//         VibroShortBoth();
//     }

//     if (now - lastLong >= LONG_INTERVAL) {
//         lastLong = now;
//         VibroLongStrongBoth();
//     }

//     delay(10); // small idle to avoid a tight busy-loop
// }


// #include <Arduino.h>
// #include <WiFiS3.h>

// const int motorPin = 9;
// const char* TARGET_PREFIX = "NG_";

// // Настройки вибрации
// const int ZONE_FAR_RSSI = -65;      // ~1 метр
// const int ZONE_MEDIUM_RSSI = -55;   // ~0.4 метра  
// const int ZONE_CLOSE_RSSI = -45;    // <0.4 метра

// const int VIBRATION_FAR = 80;
// const int VIBRATION_MEDIUM = 150;
// const int VIBRATION_CLOSE = 255;

// // Переменные состояния
// int previousRSSI = -100;
// unsigned long lastScanTime = 0;
// const int SCAN_INTERVAL = 300;


// int findStrongestRSSI() {
//   int numNetworks = WiFi.scanNetworks();
//   int strongestRSSI = -100;
  
//   for (int i = 0; i < numNetworks; i++) {
//     String ssid = WiFi.SSID(i);
//     int rssi = WiFi.RSSI(i);
    
//     if (ssid.startsWith(TARGET_PREFIX) && rssi > strongestRSSI) {
//       strongestRSSI = rssi;
//     }
//   }
  
//   return strongestRSSI;
// }

// void controlVibration(int rssi) {
//   int vibrationPower = 0;
//   String zone = "";
  
//   if (rssi >= ZONE_CLOSE_RSSI) {
//     vibrationPower = VIBRATION_CLOSE;
//     zone = "CLOSE";
//   } else if (rssi >= ZONE_MEDIUM_RSSI) {
//     vibrationPower = VIBRATION_MEDIUM;
//     zone = "MEDIUM";
//   } else if (rssi >= ZONE_FAR_RSSI) {
//     vibrationPower = VIBRATION_FAR;
//     zone = "FAR";
//   }
  
//   analogWrite(motorPin, vibrationPower);
  
//   Serial.print("RSSI:");
//   Serial.print(rssi);
//   Serial.print(" ZONE:");
//   Serial.print(zone);
//   Serial.print(" VIB:");
//   Serial.println(vibrationPower);
// }

// void analyzeMovement(int currentRSSI) {
//   int rssiChange = currentRSSI - previousRSSI;
  
//   if (rssiChange > 3) {
//     Serial.println("MOVEMENT:APPROACHING");
//   } else if (rssiChange < -3) {
//     Serial.println("MOVEMENT:RETREATING");
//   }
// }

// void processNavigation() {
//   int currentRSSI = findStrongestRSSI();
  
//   if (currentRSSI <= -80) {
//     analogWrite(motorPin, 0);
//     Serial.println("NO_SENSORS");
//     previousRSSI = -100;
//     return;
//   }
  
//   controlVibration(currentRSSI);
  
//   if (previousRSSI != -100) {
//     analyzeMovement(currentRSSI);
//   }
  
//   previousRSSI = currentRSSI;
// }


// void setup() {
//   Serial.begin(115200);
//   pinMode(motorPin, OUTPUT);
  
//   // Проверка WiFi модуля
//   if (WiFi.status() == WL_NO_MODULE) {
//     Serial.println("❌ WiFi module failed!");
//     while (true);
//   }
  
//   Serial.println("🚀 BELT DEVICE - Navigation System Started");
//   Serial.println("📡 Scanning for floor sensors...");
// }

// void loop() {
//   unsigned long currentTime = millis();
  
//   if (currentTime - lastScanTime >= SCAN_INTERVAL) {
//     processNavigation();
//     lastScanTime = currentTime;
//   }
// }

#include <Arduino.h>
#include <WiFiS3.h>

const int motorPin = 9;
const char* TARGET_PREFIX = "NG_";

// Более точные настройки вибрации
const int ZONE_FAR_RSSI = -50;      // ~2 метра
const int ZONE_MEDIUM_RSSI = -48;   // ~1 метр  
const int ZONE_CLOSE_RSSI = -45;    // ~0.5 метра
const int ZONE_VERY_CLOSE_RSSI = -42; // ~0.2 метра

const int VIBRATION_FAR = 60;
const int VIBRATION_MEDIUM = 120;
const int VIBRATION_CLOSE = 200;
const int VIBRATION_VERY_CLOSE = 255;

// Переменные для улучшенного отслеживания
int previousRSSI = -100;
unsigned long lastScanTime = 0;
const int SCAN_INTERVAL = 100; // Увеличили частоту до 10 раз в секунду
int rssiSamples[5] = {0}; // Буфер для усреднения
int sampleIndex = 0;

// Forward declarations: ensure functions used in loop/setup are visible
void processNavigation();
int findStrongestRSSI();
int calculateSmoothedRSSI();
void controlVibration(int rssi);
void analyzeMovement(int currentRSSI);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  pinMode(motorPin, OUTPUT);
  analogWrite(motorPin, 0);
  
  Serial.println("🚀 Belt Device - High Precision Mode");
  
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("❌ WiFi module failed!");
    while (true);
  }
  
  // Настраиваем WiFi для более быстрого сканирования
  // WiFi.setScanTimeout(100); // removed: CWifi (WiFiS3) has no setScanTimeout() member
  // Use the default scan settings or call WiFi.scanNetworks() with desired parameters if needed
  Serial.println("✅ High precision mode activated");
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastScanTime >= SCAN_INTERVAL) {
    processNavigation();
    lastScanTime = currentTime;
  }
}

void processNavigation() {
  int currentRSSI = findStrongestRSSI();
  
  if (currentRSSI <= -85) {
    analogWrite(motorPin, 0);
    if (previousRSSI > -85) {
      Serial.println("NO_SENSORS");
    }
    previousRSSI = -100;
    return;
  }
  
  // Усредняем значения для большей стабильности
  rssiSamples[sampleIndex] = currentRSSI;
  sampleIndex = (sampleIndex + 1) % 5;
  
  int smoothedRSSI = calculateSmoothedRSSI();
  
  controlVibration(smoothedRSSI);
  
  if (previousRSSI != -100) {
    analyzeMovement(smoothedRSSI);
  }
  
  previousRSSI = smoothedRSSI;
}

int calculateSmoothedRSSI() {
  int sum = 0;
  int count = 0;
  for (int i = 0; i < 5; i++) {
    if (rssiSamples[i] != 0) {
      sum += rssiSamples[i];
      count++;
    }
  }
  return count > 0 ? sum / count : -100;
}

int findStrongestRSSI() {
  int numNetworks = WiFi.scanNetworks();
  int strongestRSSI = -100;
  String closestSensor = "";
  
  // Быстрое сканирование с приоритетом на наши сети
  for (int i = 0; i < numNetworks; i++) {
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    
    if (ssid.startsWith(TARGET_PREFIX)) {
      if (rssi > strongestRSSI) {
        strongestRSSI = rssi;
        closestSensor = ssid;
      }
    }
  }
  
  // Быстрый вывод для отладки (только при изменении)
  static String lastClosest = "";
  if (closestSensor != "" && closestSensor != lastClosest) {
    Serial.print("🎯 ");
    Serial.print(closestSensor);
    Serial.print(" (");
    Serial.print(strongestRSSI);
    Serial.println(" dBm)");
    lastClosest = closestSensor;
  }
  
  return strongestRSSI;
}

void controlVibration(int rssi) {
  int vibrationPower = 0;
  String zone = "";
  
  // Более точные зоны с плавными переходами
  if (rssi >= ZONE_VERY_CLOSE_RSSI) {
    vibrationPower = VIBRATION_VERY_CLOSE;
    zone = "VERY_CLOSE";
  } else if (rssi >= ZONE_CLOSE_RSSI) {
    // Плавный переход между зонами
    vibrationPower = map(rssi, ZONE_CLOSE_RSSI, ZONE_VERY_CLOSE_RSSI, 
                        VIBRATION_CLOSE, VIBRATION_VERY_CLOSE);
    zone = "CLOSE";
  } else if (rssi >= ZONE_MEDIUM_RSSI) {
    vibrationPower = map(rssi, ZONE_MEDIUM_RSSI, ZONE_CLOSE_RSSI,
                        VIBRATION_MEDIUM, VIBRATION_CLOSE);
    zone = "MEDIUM";
  } else if (rssi >= ZONE_FAR_RSSI) {
    vibrationPower = map(rssi, ZONE_FAR_RSSI, ZONE_MEDIUM_RSSI,
                        VIBRATION_FAR, VIBRATION_MEDIUM);
    zone = "FAR";
  } else {
    vibrationPower = 0;
    zone = "OUT_OF_RANGE";
  }
  
  vibrationPower = constrain(vibrationPower, 0, 255);
  analogWrite(motorPin, vibrationPower);
  
  // Компактный вывод для частого обновления
  static int lastPrintedPower = -1;
  if (vibrationPower != lastPrintedPower) {
    Serial.print(rssi);
    Serial.print("dBm ");
    Serial.print(zone);
    Serial.print(" ");
    Serial.println(vibrationPower);
    lastPrintedPower = vibrationPower;
  }
}

void analyzeMovement(int currentRSSI) {
  int rssiChange = currentRSSI - previousRSSI;
  
  if (rssiChange > 2) {
    Serial.println("↑ APPROACHING");
  } else if (rssiChange < -2) {
    Serial.println("↓ RETREATING");
  }
}