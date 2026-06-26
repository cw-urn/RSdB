#include <PDM.h> // microphone library
#include <Wire.h> // I2C library
#include <Adafruit_GFX.h> // graphics
#include <Adafruit_SSD1306.h> // library for the oled
#include <math.h>

#define MIC_CLK_PIN   D2 // pins
#define MIC_DATA_PIN  D3

#define SCREEN_WIDTH  128 // basic setup stuff
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_I2C_ADDR 0x3C // not too sure about this i might have to change it
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const int CHANNELS = 1; // mono
static const int SAMPLE_RATE = 16000; // mic's sample rate is 16kHz
short sampleBuffer[512]; // buffer to hold PDM samples // short is basically an integer
                         // 512 samples at 16kHz is 32ms of audio

volatile double sumSquares = 0; // holds the running sum of every sample squared
volatile uint32_t sampleCount = 0; // tracks how many samples have gone into that sum
                                   // volatile because theyr written from an interrupt service routine and read from the main loop

static const float SPL_OFFSET_DB = 120.0; // this is a guess. the datasheet says it typically ranges from -26 dBFS at 94 dB SPL so 94--26 = 120.

uint32_t lastUpdate = 0; // uint32_t is millis()'s return type

void onPDMdata() { // function to handle incoming PDM data
  int bytesAvailable = PDM.available();
  PDM.read(sampleBuffer, bytesAvailable); // reads
  int samplesRead = bytesAvailable / 2; // 2 bytes per sample
  for (int i = 0; i < samplesRead; i++) { 
    sumSquares += (double)sampleBuffer[i] * (double)sampleBuffer[i];
  }
  sampleCount += samplesRead; // for rms calculation
}

void setup() {
  Serial.begin(115200); // usb port for debugging

  Wire.begin(); // start the I2C bus on the default pins
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("SSD1306 not found");
    while (1) { delay(10); }
  }
  display.clearDisplay(); // make it black
  display.setTextColor(SSD1306_WHITE); // make the text white
  display.setTextSize(1); // set the text size to 1
  display.setCursor(0, 0); // set the cursor to the top left
  display.println("Starting mic..."); // print to the display
  display.display(); // display the display lmao

  PDM.setCLK(MIC_CLK_PIN); // clock
  PDM.setDIN(MIC_DATA_PIN); // data
  PDM.onReceive(onPDMdata); // set the function to be called when data is available

  if (!PDM.begin(CHANNELS, SAMPLE_RATE)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("PDM init failed!");
    display.display();
    while (1) { delay(10); }
  }

  lastUpdate = millis(); // one second timer for the loop to update the display every second
}

void loop() {
  if (millis() - lastUpdate >= 1000) {
    lastUpdate += 1000;

    noInterrupts(); // disable interrupts for a couple milliseconds
    double ss = sumSquares;
    uint32_t n = sampleCount;
    sumSquares = 0;
    sampleCount = 0;
    interrupts();

    float dbSPL = 0;
    if (n > 0) {
      double rms = sqrt(ss / (double)n); // rms formula: sqrt(sum of squares / number of samples)
      if (rms < 1.0) {
        rms = 1.0; // log10(0) is undefined, so rms to 1.0 to avoid that
      }
      float dbFS = 20.0 * log10(rms / 32768.0); 
      dbSPL = dbFS + SPL_OFFSET_DB; // final dB calculatiuon!!!! convert from dBFS to dB SPL
    }

    Serial.print("dB SPL (approx): ");
    Serial.println(dbSPL, 1); // print it

    display.clearDisplay(); // clear it
    display.setTextSize(2); // make it bigger
    display.setCursor(0, 16); // set the cursor to the middle of the screen
    display.print(dbSPL, 1); // print the dB SPL to the display
    display.println(" dB"); // print the unit
    display.display(); // display the display again
  }
}
