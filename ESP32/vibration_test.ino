#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320  // Cambiar a 32 si tu pantalla OLED es de
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuración del ADXL345
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void setup() {
  Serial.begin(115200);

  // Inicializar pantalla
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se encontró la pantalla OLED"));
    while (1)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Iniciado correctamente");
  display.display();
  delay(1000);

  // Inicializar sensor
  if (!accel.begin()) {
    Serial.println("No se encontró el sensor ADXL345");
    while (1)
      ;
  }

  accel.setRange(ADXL345_RANGE_2_G);
  accel.setDataRate(ADXL345_DATARATE_800_HZ);
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  // Convertir los valores de aceleración a G (gravedad)
  float accelX = event.acceleration.x / 9.81;
  float accelY = event.acceleration.y / 9.81;
  float accelZ = event.acceleration.z / 9.81;

  float vibrationMagnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  // presentar los datos en pantalla
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Vibracion: ");
  display.print(vibrationMagnitude, 2);

  display.setCursor(0, 15);
  display.print("X: ");
  display.print(accelX, 2);

  display.setCursor(0, 25);
  display.print("Y: ");
  display.print(accelY, 2);

  display.setCursor(0, 35);
  display.print("Z: ");
  display.print(accelZ, 2);

  display.display();

  Serial.print(millis());
  Serial.print(",");
  Serial.println(vibrationMagnitude);

  delay(12);
}
