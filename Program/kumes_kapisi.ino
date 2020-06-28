/**
 * Kümes Kapısı Projesi
 * Kart Wemos D1 Mini
 * Motor Sürücü L298N
 * Motor 12V 12RPM
 * DS3231 I2C RTC Modülü ve EEPROM
 * End Switch x 2
 * LDR -- Sonraki sürümlerde..
 * LCD Ekran I2C -- Sonraki sürümlerde..
 * Optik Sensör x 2 - Sayım İçin -- Sonraki sürümlerde..
 * 
 */

// Kütüphaneler
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <RTClib.h>
#include <Wire.h>


// Pin Dağılımları
#define Motor_A D7
#define Motor_B D8
#define LDR A0
#define DS3231_ADDRESS 0x57
#define ENDSW_ALT D6
#define ENDSW_UST D5

#define DEBUG true


RTC_DS3231 saat;
DateTime simdi;
ESP8266WebServer server(80);

// tarihlemece
uint8_t acilisSaati[13][2] = {
  {0, 0}, // burası kullanılmayacak!
  {8, 45}, // Ocak
  {8, 30}, // Şubat
  {7, 55}, // Mart
  {7, 10}, // Nisan
  {6, 30}, // Mayıs
  {6, 30}, // Haziran
  {6, 30}, // Temmuz
  {6, 30}, // Ağustos
  {7, 0}, // Eylül
  {7, 20}, // Ekim
  {8, 0}, // Kasım
  {8, 25} // Aralık
};

uint8_t kapanisSaati[13][2] = {
  {0, 0}, // burası kullanılmayacak!
  {18, 35}, // ocak
  {19, 5}, // şubat
  {19, 40}, // mart
  {20, 10}, // nisan
  {20, 40}, // mayıs
  {20, 50}, // haziran
  {20, 50}, // temmuz
  {20, 35}, // ağustos
  {19, 45}, // eylül
  {19, 0}, // ekim
  {18, 15}, // kasım
  {18, 0} // aralık
};

// Diğer global değişkenler...
byte kapiDurum = 0; // 0 - bilinmiyor, 1- kapali, 2 - açık, 3 - kapıda sıkıntı var.
byte durumAltSW;
byte durumUstSW;

// otomatik çalışma aktif mi?
bool zamanAktifmi = true;



// loop içindeki sorgulama...
uint32_t sonSorguZamani = 0;
uint16_t ikiSorguArasi = 60000; // 1 dakika

// Wifi Şifreleri
const char* ssid = "KumesKapisi";
const char* password = "123456789";
// 80 Portunda bir sunucu açalım...



/*
 * Interrupt Metodları
 */
ICACHE_RAM_ATTR void swDegisimAlt() {
  durumAltSW = digitalRead(ENDSW_ALT);
}

ICACHE_RAM_ATTR void swDegisimUst() {
  durumUstSW = digitalRead(ENDSW_UST);
}

void setup() {
  // Serial monitörü açalım
  if(DEBUG) {
    Serial.begin(9600);
    log(""); // boş bir satır atalım. Anlamsız karakterlerden kurtulalım...
  }

  if(!SPIFFS.begin()){
    log("SPIFFS başlatılırken hata oluştu...");
    return;
  }
  
  // Pinleri Tanımlayalım
  pinMode(Motor_A, OUTPUT);
  pinMode(Motor_B, OUTPUT);
  pinMode(ENDSW_ALT, INPUT_PULLUP);
  pinMode(ENDSW_UST, INPUT_PULLUP);

  // Switchlere kesme ayarlayalım...
  attachInterrupt(digitalPinToInterrupt(ENDSW_ALT), swDegisimAlt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENDSW_UST), swDegisimUst, CHANGE);

  // switchlerin durumlarını oku. daha sonrasında interruptlar bu işi yapacak.
  durumAltSW = digitalRead(ENDSW_ALT);
  durumUstSW = digitalRead(ENDSW_UST);

  // RTC yi başlatalım...
  if(!saat.begin()) {
    log(F("RTC Başlatılamadı..."));
    abort();
  }

  // Otomatik çalışma ile ilgili veriyi eepromdan alalım
  otomatikCalismaGetir();
  delay(500);

  
  /*
  // saat daha önce ayarlanmamışsa...
  if(saat.lostPower()) {
    log(F("RTC güç kaybetmiş, hadi saati tekrardan ayarlayalım..."));
    saat.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.print("Tarih: ");
  Serial.println(__DATE__);
  Serial.print("Saat: ");
  Serial.println(__TIME__);
  */

  // program yüklenirken kapı biraz kalkıyor. Yerine indirelim.
  log("İlk çalışma. Kapı aşağıya indiriliyor...");
  kapiIndir();
  delay(1000);
  
  
  // Kapı ile ilgili ayarlar...
  // kapı durumu nedir?
  kapiDurumuNedir();
  if(kapiDurum == 0) {
    // kapının durumu belli değil ise kapıyı kapat... Tilki falan girmesin...
    log("Kapı ortada kalmış. Kapı kapatılacak...");
    kapiIndir();
  }

  // Kapı durumunu sorgula ve ona göre aksiyon al...
  kapiSorgula();
  

  
  log(F("Wifi başlatılıyor..."));
  // Wifiyi Başlatalım. ama başlamış mı diye bekleyemeyelim...
  WiFi.softAP(ssid, password);

  // Soft AP IP Address
  IPAddress ip = WiFi.softAPIP();
  Serial.print("Modulün IP Adresi: ");
  Serial.println(ip);

  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/jquery.js", SPIFFS, "/jquery.js");
  server.serveStatic("/era.js", SPIFFS, "/era.js");
  server.on("/getModuleDateTime", getModuleDateTime);
  server.on("/saveDateTimeToModule", saveDateTimeToModule);
  server.on("/isAutoRunActive", isAutoRunActive);
  server.on("/setAutoRun", setAutoRun);
  server.on("/moveDoor", moveDoor);
  
  server.begin();
  log(F("HTTP Sunucu Başlatıldı..."));
  
}


void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  /**/
  // millis taşması için. Millis 32bitlik bir sayı olabilir. dolayısıyla 49 gün 17 saat sonra dolacak ve tekrar
  // 0 a dönecektir. Bu durumda ortaya çıkacak karmaşıklığı aşmak için bunu ekledik...
  if(millis() - sonSorguZamani < 0) {
    sonSorguZamani = millis();
    // burada bir daha kapı sorgulayalım... ya da 2 dakika sorgulama olmasın...
  }

  if(millis() > sonSorguZamani + ikiSorguArasi) {
    // sorgudan itibaren 1 dakika geçmiş...
    //log("Kapı sorgulanıyor...");
    kapiSorgula();
  }
  
}

void log(String veri) {
  if(DEBUG) {
    Serial.println(veri);
  }
}
