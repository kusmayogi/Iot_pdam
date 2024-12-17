#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Ganti dengan kredensial WiFi Anda
const char* ssid = "Cp0";
const char* password = "direkturpdam";

// URL endpoint
const char* serverName = "http://103.163.103.117:980/coba";

// Nama perangkat
const char* deviceName = "Induk"; // Nama perangkat yang akan dikirim

// Inisialisasi Serial pada GPIO 16 (RX) dan GPIO 17 (TX)
HardwareSerial mySerial(1);

// Variabel untuk mengelola waktu pembacaan sensor
unsigned long previousMillis = 0;
const long interval = 10000;       // Interval pengukuran dalam milidetik (5 detik)
const long resetInterval = 20000; // Interval reset dalam milidetik (20 detik)
const float maxDistance = 250.0;  // Jarak maksimum 330 cm

void setup() {
  Serial.begin(115200);           // Serial untuk debugging
  mySerial.begin(9600, SERIAL_8N1, 16, 17); // Serial untuk sensor

  // Menghubungkan ke WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to WiFi");
}

void loop() {
  unsigned long currentMillis = millis();

  // Mengambil nilai millis saat ini
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Reset waktu sebelumnya
    
    // Memastikan data tersedia pada serial sensor
    if (mySerial.available() >= 4) { // Pastikan 4 byte tersedia sebelum dibaca
      // Membaca data dari sensor
      uint8_t data[4];
      mySerial.readBytes(data, 4);
      
      // Debugging: Tampilkan data yang diterima dari sensor
      Serial.print("Data received: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(data[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      
      // Memeriksa apakah byte pertama valid (misalnya, 0xFF sebagai start byte)
      if (data[0] == 0xFF) {
        // Menggabungkan byte 1 dan byte 2 untuk mendapatkan jarak dalam milimeter
        uint16_t distance_mm = (data[1] << 8) | data[2];
        
        // Mengonversi jarak dari mm ke cm
        float distance_cm = distance_mm / 10.0;

        // Batasi jarak maksimal hingga 330 cm
        if (distance_cm > maxDistance) {
          distance_cm = maxDistance;
        }

        // Membalikkan jarak: semakin dekat objek, semakin besar nilainya
        float invertedDistance = maxDistance - distance_cm;
        
        Serial.print("JARAK: ");
        Serial.println(invertedDistance);
        Serial.println(" cm");

        // Jika terhubung ke WiFi, kirim data jarak ke server
        if (WiFi.status() == WL_CONNECTED) {
          HTTPClient http;
          
          // Membuat URL dengan parameter jarak terbalik dan nama perangkat
          String url = String(serverName) + "?device=" + String(deviceName) + "&distance=" + String(invertedDistance, 3);
          http.begin(url);
          
          // Melakukan HTTP GET request
          int httpResponseCode = http.GET();
          
          // Mengecek apakah request berhasil
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
      } else {
        Serial.println("Invalid start byte received.");
      }
    } else {
      Serial.println("Not enough data available to read.");
    }
  }

  // Periksa apakah waktu reset telah berlalu
  if (currentMillis % resetInterval == 0) {
    Serial.println("Restarting bolo");
    delay(10000); // Tunggu 10 detik
    ESP.restart(); // Mereset ESP32
  }
}
