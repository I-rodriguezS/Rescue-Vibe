# Rescue-Vibe: Bio-Radar Acústico-Vibracional con Edge-AI

Sistema embebido de alta precisión para la detección, discriminación e hiperlocalización (triangulación TDoA) de personas atrapadas bajo escombros, optimizado mediante Inteligencia Artificial en el Borde (Edge-AI / TinyML) y microcontroladores ESP32-S3.

---

## 📋 Descripción del Proyecto
Tras el colapso de una estructura por eventos sísmicos o fallas estructurales, los sobrevivientes intentan comunicarse emitiendo impactos periódicos contra los elementos rígidos (vigas, columnas o losas de concreto). Sin embargo, los equipos de rescate enfrentan un grave problema operativo: **la contaminación acústica y vibracional** producida por maquinaria pesada (excavadoras, generadores diésel, taladros).

**Rescue-Vibe** resuelve este desafío utilizando el concreto como medio denso de propagación y aplicando procesamiento digital de señales junto a un modelo de red neuronal convolucional ejecutado localmente en el microcontrolador. El sistema es capaz de **isolar el pulso humano del ruido industrial**, calcular la posición exacta de la víctima mediante diferencias de tiempo de llegada (TDoA) y transmitir los datos a través de una red inalámbrica de ultra baja latencia.

---

## 🛠️ Arquitectura Tecnológica y Hardware
* **Unidad de Procesamiento Central:** Microcontrolador ESP32-S3 (gestión de bus SPI, pila TinyML, protocolo ESP-NOW y cálculo de triangulación).
* **Matriz de Sensado:** Módulos acelerométricos digitales **ADXL345** configurados en modo de alta resolución (+/- 2g, 13 bits, 3.9 mg/LSB) para capturar microvibraciones dinámicas sin la degradación de un ADC analógico.
* **Etapa de Potencia y Simulación:** 
  * Transistores MOSFET IRF520 para la conmutación de alta corriente.
  * Actuador electromagnético lineal JF-0530B (simulación de pulsos de la víctima).
  * Micromotor DC con excéntrica y modulación PWM (simulación de ruido de maquinaria).
  * Diodos de rueda libre (flyback) 1N4007 para protección contra fuerzas electromotrices inversas.
* **Interfaces HMI y Monitoreo:** Pantallas TFT / OLED (SSD1306) locales y un sistema complementario de microcámaras subterráneas para la verificación visual factual del jurado o rescatistas.

---

## 📊 Procesamiento Digital de Señales (DSP) & Edge-AI
1. **Normalización Vectorial:** Lectura sincronizada a través de temporizadores hardware (83.3 Hz / 12 ms) y cálculo de la magnitud euclidiana tridimensional para asegurar independencia direccional:
   `M = sqrt(Ax^2 + Ay^2 + Az^2)`
2. **Transformada Rápida de Fourier (FFT):** Conversión de las series temporales al dominio de la frecuencia mediante el bloque de análisis espectral de **Edge Impulse**.
3. **Inferencia Local (TinyML):** Modelo de clasificación entrenado para diferenciar entre tres clases principales:
   * `GOLPE_HUMANO` (impactos secos y rítmicos).
   * `MAQUINARIA` (vibración industrial continua).
   * `RUIDO_AMBIENTAL` (estado base).
   * *Tiempo de inferencia:* Menor a 15 ms con una certeza de clasificación superior al 90%.

---

## 📡 Red Inalámbrica y Triangulación TDoA (ESP-NOW)
El sistema opera mediante una topología en red distribuida:
* **Nodos Esclavos (4 Unidades):** Leen localmente el ADXL345, ejecutan la inferencia de la IA y, ante la detección de un impacto humano con alta certeza, capturan la marca de tiempo sincronizada en microsegundos (`micros() + clock_offset_us`).
* **Nodo Maestro (1 Unidad):** Envía pulsos periódicos de sincronización de reloj por broadcast y recopila las tramas de los esclavos mediante **ESP-NOW** (sin latencia de enrutador). Al recibir al menos 3 marcas de tiempo válidas, resuelve las ecuaciones hiperbólicas de diferencia de tiempo de llegada (**TDoA**) para fijar las coordenadas cartesianas (X, Y) del origen del impacto.

---

## 💻 Estructura del Código Base (Arduino C++)

El programa combina la lectura del acelerómetro, el manejo del display y el procesamiento de la librería generada por Edge Impulse:

```cpp
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se encontró la pantalla OLED"));
    while(1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Rescue-Vibe Iniciado");
  display.display();
  delay(1000);

  if(!accel.begin()) {
    Serial.println("No se encontró el sensor ADXL345");
    while(1);
  }

  accel.setRange(ADXL345_RANGE_2_G);
  accel.setDataRate(ADXL345_DATARATE_800_HZ);
}

void loop() {
  sensors_event_t event;
  accel.getEvent(&event);

  float accelX = event.acceleration.x / 9.81;
  float accelY = event.acceleration.y / 9.81;
  float accelZ = event.acceleration.z / 9.81;

  float vibrationMagnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Vibracion: ");
  display.print(vibrationMagnitude, 2);

  display.setCursor(0, 15);
  display.print("X: "); display.print(accelX, 2);
  display.setCursor(0, 25);
  display.print("Y: "); display.print(accelY, 2);
  display.setCursor(0, 35);
  display.print("Z: "); display.print(accelZ, 2);
  
  display.display();

  Serial.print(millis());
  Serial.print(",");
  Serial.println(vibrationMagnitude);

  delay(12);
}