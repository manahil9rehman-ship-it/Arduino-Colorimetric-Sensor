#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <math.h>

// define TFT pins
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// define colors
#define BLACK   0x0000
#define BLUE    0x001F
#define GREEN   0x07E0
#define RED     0xF800
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// set up graph + sensor
int graphX0 = 40, graphY0 = 160, graphW = 240, graphH = 120;
int maxPoints = 80;
float values[80];
int sampleCount = 0;

float I0 = 1.0;
int sensorPin = A5;

// set measuring time
unsigned long lastMeasureTime = 0;
unsigned long measureInterval = 2000; // 2 seconds
bool measuringSample = false;

// setup
void setup() {
  Serial.begin(9600);
  tft.reset();
  tft.begin(0x9341);
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  drawAxes();

  // blank automatically
  blankSample();

  Serial.println("Blank set. Insert sample, then type 's' in Serial Monitor to start measurements.");
}

// loop
void loop() {
  // Check for Serial trigger
  if (Serial.available() > 0) {
    char c = Serial.read();
    if ((c == 's' || c == 'S') && !measuringSample) {
      measuringSample = true;
      Serial.println("Starting sample measurements...");
    }
  }

  // measure once triggered 
  if (measuringSample) {
    unsigned long now = millis();
    if (now - lastMeasureTime >= measureInterval) {
      measureSample();
      lastMeasureTime = now;
    }
  }
}

// functions
void drawAxes() {
  tft.setTextColor(WHITE);
  tft.setTextSize(1);

  for (int j = 0; j <= 10; j++) {
    int yTick = graphY0 - (j * graphH) / 10;
    tft.drawLine(graphX0 - 5, yTick, graphX0, yTick, WHITE);
    tft.setCursor(5, yTick - 4);
    tft.print(j * 0.1);
  }

  for (int i = 0; i <= 8; i++) {
    int xTick = graphX0 + (i * graphW) / 8;
    tft.drawLine(xTick, graphY0, xTick, graphY0 + 5, WHITE);
  }

  tft.drawRect(graphX0, graphY0 - graphH, graphW, graphH, WHITE);
}

void blankSample() {
  int raw = analogRead(sensorPin);
  if (raw < 1) raw = 1; // avoid zero
  I0 = raw;
  Serial.print("Blank set: I0 = "); Serial.println(I0);
}

void measureSample() {
  if (sampleCount >= maxPoints) return;

  int raw = analogRead(sensorPin);
  if (raw < 1) raw = 1; // avoid zero
  if (I0 < 1) I0 = 1;

  float I = raw;
  float A = -log10(I / I0);
  values[sampleCount] = A;

  // print raw + absorbance values
  Serial.print("Raw sensor: "); Serial.print(raw);
  Serial.print(" | I0: "); Serial.print(I0);
  Serial.print(" | A: "); Serial.println(A);

  if (sampleCount > 0) {
    int x1 = graphX0 + (sampleCount - 1) * graphW / (maxPoints - 1);
    int x2 = graphX0 + sampleCount * graphW / (maxPoints - 1);
    int y1 = graphY0 - (values[sampleCount - 1] / 1.0) * graphH;
    int y2 = graphY0 - (A / 1.0) * graphH;
    tft.drawLine(x1, y1, x2, y2, YELLOW);
  }

  sampleCount++;
}
