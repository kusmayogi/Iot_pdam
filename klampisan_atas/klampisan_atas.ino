#include <WiFi.h>
#include <HTTPClient.h>

const int trigPin = 5; // Pin Trig
const int echoPin = 18; // Pin Echo

// Ganti dengan kredensial WiFi Anda
const char* ssid = "PDAM";
const char* password = "12345678";

// URL endpoint
const char* serverName = "http://103.163.103.117:980/coba";

// Nama perangkat
const char* deviceName = "klampisan_atas"; // Nama perangkat yang akan dikirim

// Waktu untuk restart (1 jam dalam milidetik)
const unsigned long restartInterval = 3600000; // 1 jam = 3600000 ms
unsigned long previousMillisRestart = 0; // Waktu sebelumnya untuk restart

const long maxDistance = 125; // Jarak maksimal dalam cm

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);

  // Mengatur pin Trig sebagai output
  pinMode(trigPin, OUTPUT);
  // Mengatur pin Echo sebagai input
  pinMode(echoPin, INPUT);

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
  // Mengambil waktu saat ini
  unsigned long currentMillis = millis();

  // Mengirimkan pulse 10us ke pin Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Membaca durasi pulse pada pin Echo
  long duration = pulseIn(echoPin, HIGH);

  // Menampilkan durasi pulse untuk debugging
  Serial.print("Duration: ");
  Serial.print(duration);
  Serial.println(" microseconds");

  // Menghitung jarak dalam cm
  if (duration > 0) {
    long distance = (duration / 2) / 29.1;

    // Batasi jarak maksimal hingga 250 cm
    if (distance > maxDistance) {
      distance = maxDistance;
    }

    // Membalikkan jarak: semakin dekat objek, semakin besar nilainya
    long mappedDistance = map(distance, 0, maxDistance, maxDistance, 0);
    
    Serial.print("Mapped Distance: ");
    Serial.print(mappedDistance);
    Serial.println(" cm");

    // Jika terhubung ke WiFi, kirim data jarak ke server
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      // Membuat URL dengan parameter jarak yang sudah dipetakan dan nama perangkat
      String url = String(serverName) + "?device=" + String(deviceName) + "&distance=" + String(mappedDistance);
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
    Serial.println("No pulse detected.");
  }

  // Cek apakah waktu 1 jam sudah berlalu untuk restart
  if (currentMillis - previousMillisRestart >= restartInterval) {
    Serial.println("Restarting system after 1 hour...");
    delay(1000); // Tunggu 1 detik sebelum restart
    ESP.restart(); // Restart ESP32
  }

  // Menunggu 1 detik sebelum pembacaan berikutnya
  delay(1000);
}
