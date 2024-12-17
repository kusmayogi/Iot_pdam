#include <WiFi.h>
#include <HTTPClient.h>

// Ganti dengan kredensial WiFi Anda
const char* ssid = "Cp0";
const char* password = "direkturpdam";

// URL endpoint
const char* serverName = "http://103.163.103.117:980/coba";
const char* deviceName = "Induk";

const float OffSet = 0.50; // Nilai ini berdasarkan hasil kalibrasi tanpa tekanan
float V, P; // Tegangan dan Tekanan

unsigned long previousMillis = 0; // Variabel untuk melacak waktu
const long resetInterval = 600000; // 10 menit

// Koefisien untuk filter low-pass
float alpha = 0.1; // Konstanta filter (0.0 - 1.0), sesuaikan dengan kebutuhan
float filteredVoltage = 0; // Tegangan yang sudah difilter

void setup() {
  Serial.begin(9600);
  Serial.println("/** Water Pressure Sensor Data **/");

  // Menghubungkan ke WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Cek apakah sudah 10 menit berlalu untuk reset
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= resetInterval) {
    Serial.println("Resetting device...");
    ESP.restart(); // Reset ESP32 setelah 10 menit
  }

  // Membaca nilai dari sensor analog (gunakan GPIO 34)
  int sensorValue = analogRead(34);
  V = sensorValue * 3.3 / 4095; // Mengonversi nilai ADC ke tegangan (3.3V, 12-bit)

  // Filter low-pass untuk menstabilkan pembacaan tegangan
  filteredVoltage = (alpha * V) + ((1 - alpha) * filteredVoltage);

  // Menghitung tekanan berdasarkan tegangan yang sudah difilter
  P = (filteredVoltage - OffSet) * 400; // Mengonversi tegangan ke tekanan (KPa)
  float P_bar = P / 100.0; // Konversi tekanan ke bar

  // Menampilkan hasil di Serial Monitor
  Serial.print("Filtered Voltage: ");
  Serial.print(filteredVoltage, 3);
  Serial.println(" V");
  Serial.print("Pressure: ");
  Serial.print(P_bar, 3);
  Serial.println(" bar");
  Serial.println();

  // Mengirim data ke server menggunakan GET
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverName) + "?device=" + String(deviceName) + "&voltage=" + String(filteredVoltage, 3) + "&pressure=" + String(P_bar, 3);
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error in HTTP request: " + String(httpResponseCode));
    }

    http.end(); // Menutup koneksi
  } else {
    Serial.println("Error in WiFi connection");
  }

  delay(2000); // Tunggu 2 detik sebelum pengiriman berikutnya
}
