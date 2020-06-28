//
void getModuleDateTime() {
  String veri = "";
  simdi = saat.now();

  veri += String(simdi.day());
  veri += ".";
  veri += String(simdi.month());
  veri += ".";
  veri += String(simdi.year());
  veri += " ";
  veri += String(simdi.hour());
  veri += ".";
  veri += String(simdi.minute());
  veri += ".";
  veri += String(simdi.second());
  
  server.send(200, "text/plain", veri);
  log("Saat verisi sunucuya gönderildi...");
}

void isAutoRunActive() {
  if(zamanAktifmi) {
    server.send(200, "text/plain", "evet");
  } else {
    server.send(200, "text/plain", "hayir");
  }
  log("Otomatik çalışma durumu sunucuya gönderildi...");
}

void setAutoRun() {
  if(server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed!");
  } else {
    if(server.hasArg("auto")) {
      bool veri = server.arg("auto") == "1" ? true : false;
      zamanAktifmi = veri;
      // eeprom a kaydet...
      otomatikCalismaAyarla(veri);
      
      server.send(200, "text/plain", "OK");
    } else {
      server.send(405, "text/plain", "Method Not Allowed!");
    }
  }
  log("Otomatik çalışma durumu ile ilgili işlem yapıldı...");
}

void moveDoor() {
  if(server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed!");
  } else {
    if(server.hasArg("yon")) {
      bool veri = server.arg("yon") == "1" ? true : false;

      if(veri) {
        log("Kapı kaldırılıyor...");
        kapiKaldir();
      } else {
        log("Kapı indiriliyor...");
        kapiIndir();
      }
      
      server.send(200, "text/plain", "OK");
    } else {
      server.send(405, "text/plain", "Method Not Allowed!");
    }
  }
}

void saveDateTimeToModule() {
  if(server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed!");
  } else {

    uint16_t yil;
    uint8_t ay;
    uint8_t gun;
    uint8_t saatimiz;
    uint8_t dakika;
    uint8_t saniye;

    yil     = server.hasArg("yil") ? server.arg("yil").toInt() : 2020;
    ay      = server.hasArg("ay") ? server.arg("ay").toInt() : 1;
    gun     = server.hasArg("gun") ? server.arg("gun").toInt() : 1;
    saatimiz = server.hasArg("saat") ? server.arg("saat").toInt() : 0;
    dakika  = server.hasArg("dakika") ? server.arg("dakika").toInt() : 0;
    saniye  = server.hasArg("saniye") ? server.arg("saniye").toInt() : 0;
    
    Serial.print("Gelen Tarih: ");
    Serial.print(yil);
    Serial.print(" ");
    Serial.print(ay);
    Serial.print(" ");
    Serial.println(gun);
    Serial.print("Gelen Zaman: ");
    Serial.print(saatimiz);
    Serial.print(":");
    Serial.print(dakika);
    Serial.print(":");
    Serial.println(saniye);

    saat.adjust(DateTime(yil, ay, gun, saatimiz, dakika, saniye));
    server.send(200, "text/plain", "OK");
    log("Saat verisi cihaz üzerine kaydedildi...");
  }
}
