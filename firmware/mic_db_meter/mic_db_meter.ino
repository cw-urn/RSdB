#include <PDM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define MIC_CLK_PIN   D2
#define MIC_DATA_PIN  D3

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_I2C_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const int CHANNELS = 1;
static const int SAMPLE_RATE = 16000;
short sampleBuffer[512];

volatile double sumSquares = 0;
volatile uint32_t sampleCount = 0;

static const float SPL_OFFSET_DB = 120.0;

uint32_t lastUpdate = 0;

void onPDMdata() {
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable);
  int samplesRead = bytesAvailable / 2;
  for (int i = 0; i < samplesRead; i++) {
    sumSquares += (double)sampleBuffer[i] * (double)sampleBuffer[i];
  }
  sampleCount += samplesRead;
}

void setup() {
  Serial.begin(115200);

  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("SSD1306 not found");
    while (1) { delay(10); }
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Starting mic...");
  display.display();

  PDM.setCLK(MIC_CLK_PIN);
  PDM.setDIN(MIC_DATA_PIN);
  PDM.onReceive(onPDMdata);

  if (!PDM.begin(CHANNELS, SAMPLE_RATE)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("PDM init failed!");
    display.display();
    while (1) { delay(10); }
  }

  lastUpdate = millis();
}

void loop() {
  if (millis() - lastUpdate >= 1000) {
    lastUpdate += 1000;

    noInterrupts();
    double ss = sumSquares;
    uint32_t n = sampleCount;
    sumSquares = 0;
    sampleCount = 0;
    interrupts();

    float dbSPL = 0;
    if (n > 0) {
      double rms = sqrt(ss / (double)n);
      if (rms < 1.0) {
        rms = 1.0;
      }
      float dbFS = 20.0 * log10(rms / 32768.0);
      dbSPL = dbFS + SPL_OFFSET_DB;
    }

    Serial.print("dB SPL (approx): ");
    Serial.println(dbSPL, 1);

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.print(dbSPL, 1);
    display.println(" dB");
    display.display();
  }
}
