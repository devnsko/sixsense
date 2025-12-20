#include <Arduino.h>
#include <WiFiS3.h>

// --- КОНФИГУРАЦИЯ ---
const int motorPin = 9;
const int typePin = 6;
const char* TARGET_PREFIX = "NG_";

// Настройки RSSI (расстояние)
const int ZONE_FAR_RSSI = -50;
const int ZONE_MEDIUM_RSSI = -48;
const int ZONE_CLOSE_RSSI = -45;
const int ZONE_VERY_CLOSE_RSSI = -42;

// Настройки ШИМ (мощность вибрации)
const int VIBRATION_FAR = 60;
const int VIBRATION_MEDIUM = 120;
const int VIBRATION_CLOSE = 200;
const int VIBRATION_VERY_CLOSE = 255;

// --- ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ---

// Состояние сканирования
bool isScanning = false;
int lastFoundRSSI = -100;
char lastFoundType = '-';

// Scan timer to throttle synchronous scans
unsigned long lastScanTime = 0;
const unsigned long SCAN_INTERVAL = 300; // ms between full scans

// Сглаживание
const int SMOOTH_WINDOW = 3;
int rssiHistory[SMOOTH_WINDOW];
int rssiIndex = 0;

// Управление вибрацией (State Machine)
unsigned long vibrationTimer = 0;
int vibrationState = 0; // 0: Idle, 1: Pulse High, 2: Pulse Low
int currentTargetPower = 0; // Целевая мощность (0-255)

// --- ПРОТОТИПЫ ---
void checkScanResults();
void updateVibrationSystem();
int getSmoothedRSSI(int newRSSI);

void setup() {
  Serial.begin(115200);
  // while (!Serial); // Можно убрать для автономной работы

  pinMode(motorPin, OUTPUT);
  pinMode(typePin, OUTPUT);
  analogWrite(motorPin, 0);
  digitalWrite(typePin, LOW);

  Serial.println("🚀 Belt Device - Async Mode");

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("❌ WiFi module failed!");
    while (true);
  }

  // Заполняем буфер начальными значениями
  for(int i=0; i<SMOOTH_WINDOW; i++) rssiHistory[i] = -100;

  // Выполняем первое синхронное сканирование (WiFiS3 использует синхронный API)
  WiFi.scanNetworks();
  lastScanTime = millis();
}

void loop() {
  // 1. Управление мотором (Выполняется ВСЕГДА, максимально быстро)
  updateVibrationSystem();

  // 2. Проверка результатов сканирования (Не блокирует мотор)
  checkScanResults();
}

// --- ЛОГИКА СКАНИРОВАНИЯ ---
void checkScanResults() {
  // Throttle synchronous scans
  if (millis() - lastScanTime < SCAN_INTERVAL) return;
  lastScanTime = millis();

  // Perform a synchronous scan (WiFiS3 doesn't provide the async helpers used previously)
  int scanResult = WiFi.scanNetworks();

  if (scanResult <= 0) {
    // No networks found
    if (lastFoundRSSI > -90) {
      lastFoundRSSI = -100;
      lastFoundType = '-';
      Serial.println("LOST_TARGET");
    }
    return;
  }

  int strongestID = -1;
  int maxRSSI = -100;

  for (int i = 0; i < scanResult; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.startsWith(TARGET_PREFIX)) {
      int rssi = WiFi.RSSI(i);
      if (rssi > maxRSSI) {
        maxRSSI = rssi;
        strongestID = i;
      }
    }
  }

  if (strongestID != -1) {
    int smoothed = getSmoothedRSSI(maxRSSI);
    String ssid = WiFi.SSID(strongestID);
    char type = ssid.charAt(ssid.length() - 1);

    lastFoundRSSI = smoothed;
    lastFoundType = type;

    Serial.print("TARGET: "); Serial.print(ssid);
    Serial.print(" | RAW: "); Serial.print(maxRSSI);
    Serial.print(" | SMTH: "); Serial.println(smoothed);
  } else {
    if (lastFoundRSSI > -90) {
      lastFoundRSSI = -100;
      lastFoundType = '-';
      Serial.println("LOST_TARGET");
    }
  }
}

// --- ЛОГИКА ВИБРАЦИИ (НЕБЛОКИРУЮЩАЯ) ---
void updateVibrationSystem() {
  int targetPWM = 0;
  
  // 1. Расчет базовой силы вибрации на основе RSSI
  if (lastFoundRSSI >= ZONE_VERY_CLOSE_RSSI) {
    targetPWM = VIBRATION_VERY_CLOSE;
  } else if (lastFoundRSSI >= ZONE_CLOSE_RSSI) {
    targetPWM = map(lastFoundRSSI, ZONE_CLOSE_RSSI, ZONE_VERY_CLOSE_RSSI, VIBRATION_CLOSE, VIBRATION_VERY_CLOSE);
  } else if (lastFoundRSSI >= ZONE_MEDIUM_RSSI) {
    targetPWM = map(lastFoundRSSI, ZONE_MEDIUM_RSSI, ZONE_CLOSE_RSSI, VIBRATION_MEDIUM, VIBRATION_CLOSE);
  } else if (lastFoundRSSI >= ZONE_FAR_RSSI) {
    targetPWM = map(lastFoundRSSI, ZONE_FAR_RSSI, ZONE_MEDIUM_RSSI, VIBRATION_FAR, VIBRATION_MEDIUM);
  } else {
    targetPWM = 0;
  }
  
  targetPWM = constrain(targetPWM, 0, 255);

  // 2. Обработка типов препятствий
  if (targetPWM > 0) {
    if (lastFoundType == 'W') { // Wall - Стена
      digitalWrite(typePin, HIGH); 
      analogWrite(motorPin, 255); // Максимальная постоянная вибрация
      return; // Выходим, пульсация не нужна
      
    } else if (lastFoundType == 'S') { // Stairs - Лестница (Двойной пульс)
      digitalWrite(typePin, LOW);
      
      unsigned long currentMillis = millis();
      const int PULSE_ON = 80;
      const int PULSE_OFF = 80;
      const int PAUSE_LONG = 300; 

      // Логика двойного пульса без delay()
      // Цикл: Вкл -> Выкл -> Вкл -> Длинная пауза -> повтор
      
      switch (vibrationState) {
        case 0: // Старт первого пульса
          analogWrite(motorPin, VIBRATION_CLOSE); 
          vibrationTimer = currentMillis;
          vibrationState = 1;
          break;
        case 1: // Ждем конца первого пульса
          if (currentMillis - vibrationTimer >= PULSE_ON) {
            analogWrite(motorPin, 0);
            vibrationTimer = currentMillis;
            vibrationState = 2;
          }
          break;
        case 2: // Ждем конца короткой паузы
          if (currentMillis - vibrationTimer >= PULSE_OFF) {
            analogWrite(motorPin, VIBRATION_CLOSE);
            vibrationTimer = currentMillis;
            vibrationState = 3;
          }
          break;
        case 3: // Ждем конца второго пульса
          if (currentMillis - vibrationTimer >= PULSE_ON) {
            analogWrite(motorPin, 0);
            vibrationTimer = currentMillis;
            vibrationState = 4;
          }
          break;
        case 4: // Длинная пауза перед повтором
          if (currentMillis - vibrationTimer >= PAUSE_LONG) {
            vibrationState = 0; // Перезапуск цикла
          }
          break;
      }
      return; // Выходим, ШИМ контролируется внутри switch
    }
  }

  // Обычный режим (дистанция) или если нет сигнала
  digitalWrite(typePin, LOW);
  analogWrite(motorPin, targetPWM);
  if (lastFoundType != 'S') vibrationState = 0; 
}

int getSmoothedRSSI(int newRSSI) {
  rssiHistory[rssiIndex] = newRSSI;
  rssiIndex = (rssiIndex + 1) % SMOOTH_WINDOW;

  long sum = 0;
  int count = 0;
  for (int i = 0; i < SMOOTH_WINDOW; i++) {
    if (rssiHistory[i] != -100) {
      sum += rssiHistory[i];
      count++;
    }
  }
  return (count > 0) ? (int)(sum / count) : -100;
}