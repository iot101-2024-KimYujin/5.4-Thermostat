#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_NeoPixel.h>
#include <DHTesp.h>


DHTesp          dht;
int interval = 2000; // 2초 간격
unsigned long lastDHTReadMillis = 0; // 마지막 DHT 읽기 시간
float temperature = 0; // 온도 저장 변수
char buf[50]; // 출력 문자열 저장
#define DHTPIN 17 // DHT 센서 연결 핀 정의
int RELAY_PIN  = 21; // 릴레이 핀 정의

TFT_eSPI tft = TFT_eSPI();

const int pulseA = 43;
const int pulseB = 44;
const int pushSW = 2;


volatile long encoderValue = 0;
volatile int lastEncoded = 0;

IRAM_ATTR void handleRotary() {
    int MSB = digitalRead(pulseA);
    int LSB = digitalRead(pulseB);
    int encoded = (MSB << 1) | LSB;
    int sum = (lastEncoded << 2) | encoded;

    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;
    if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;

    lastEncoded = encoded;

    // 값 제한 (0 ~ 60)
    if (encoderValue > 60) {
        encoderValue = 60;
    } else if (encoderValue < 0) {
        encoderValue = 0;
    }
}

IRAM_ATTR void buttonClicked() {
    Serial.println("Button pushed");
}

void readDHT22() {
    unsigned long currentMillis = millis();

    if(currentMillis > lastDHTReadMillis + interval) {
        lastDHTReadMillis = currentMillis;

        temperature = dht.getTemperature();
    }
}

void setup() {
    Serial.begin(115200);
 
    // 핀 설정
    pinMode(pushSW, INPUT_PULLUP);
    pinMode(pulseA, INPUT_PULLUP);
    pinMode(pulseB, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT); // 릴레이 핀 설정
    
    // 인터럽트 설정
    attachInterrupt(pushSW, buttonClicked, FALLING);
    attachInterrupt(pulseA, handleRotary, CHANGE);
    attachInterrupt(pulseB, handleRotary, CHANGE);
    
    // TFT 초기화
    dht.setup(DHTPIN, DHTesp::DHT22); 
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Atmosphere Meter", 60, 20, 4);
    delay(1000);
    Serial.println();
    Serial.println("Temperature (C)");
}

void loop() {
    // 온도 읽기
    readDHT22();

    // 화면에 출력
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Temp: ");
    tft.print(temperature);
    tft.println(" C");

    tft.setCursor(10, 40);
    tft.print("Encoder: ");
    tft.print(encoderValue);

     // 시리얼 모니터에 현재 값 출력
    Serial.printf("Temperature: %.1f, Encoder: %ld\n", temperature, encoderValue);

    // 온도와 Rotary Encoder 값 비교
    if (temperature < encoderValue) {
        digitalWrite(RELAY_PIN, HIGH); // 릴레이 구동
    } else {
        digitalWrite(RELAY_PIN, LOW);  // 릴레이 정지
    }

    delay(1000); // 1초마다 업데이트
}