void kapiIndir() {
  uint32_t baslama = millis();
  // kapı normalde 17,8 sn de açılıyor. 18,5 sn de hala açılmamışsa kapıda sorun var demektir. o zaman motor dursun.
  // millis zamanını doldurduğunda kendini sıfırlayacaktır.
  // bu durumda kapı otomatik olarak açılmayacaktır.
  // her 10 dk da bir kapı olması gereken konumda mı diye bakalım. Bunu da zaman modülünden gelen dakikayla ölçelim..
  // minute() % 10 == 0 ise kontrol yap. Her 10 saat dakikasında bir.
  while(durumAltSW == 0 && millis() < baslama + 18500) {
    digitalWrite(Motor_A, LOW);
    digitalWrite(Motor_B, HIGH);

    delay(50);
  }

  digitalWrite(Motor_A, LOW);
  digitalWrite(Motor_B, LOW);
  delay(50);
}

void kapiKaldir() {
  uint32_t baslama = millis();

  while(durumUstSW == 0 && millis() < baslama + 18500) {
    digitalWrite(Motor_A, HIGH);
    digitalWrite(Motor_B, LOW);

    delay(50);
  }

  digitalWrite(Motor_A, LOW);
  digitalWrite(Motor_B, LOW);
  
  delay(50);
}

void kapiDurumuNedir() {
  //kapiDurum = 0;
  if(durumAltSW == 1 && durumUstSW == 0) {
    kapiDurum = 1;
  } else if(durumAltSW == 0 && durumUstSW == 1) {
    kapiDurum = 2;
  } else if(durumAltSW == 1 && durumUstSW == 1) {
    // kapıda sıkıntı var... acil durum...
    kapiDurum = 3;
  } else {
    // kapı durumu belirlenemedi. Muhtemelen kapı yarım açık kalmış. onu önce bir açalım.
    kapiDurum = 0;
  }
}

byte kapiNeredeOlmali() {
  
  simdi = saat.now();
  
  uint8_t ay = simdi.month();
  uint8_t acilma_sa = acilisSaati[ay][0];
  uint8_t acilma_dk = acilisSaati[ay][1];

  uint8_t kapanma_sa = kapanisSaati[ay][0];
  uint8_t kapanma_dk = kapanisSaati[ay][1];

  uint8_t sa = simdi.hour();
  uint8_t dk = simdi.minute();

  Serial.print("Açılma Saati: ");
  Serial.print(acilma_sa);
  Serial.print(":");
  Serial.println(acilma_dk);
  Serial.print("Kapanma Saati: ");
  Serial.print(kapanma_sa);
  Serial.print(":");
  Serial.println(kapanma_dk);
  Serial.print("Şimdiki Saat: ");
  Serial.print(sa);
  Serial.print(":");
  Serial.println(dk);

  if(sa > acilma_sa) {
    // kapanma saati kontrol edilsin. kapanma saatinden küçük ise sorgulama yapalım daha
    if(sa < kapanma_sa) {
      // bu durumda kapı her türlü açık
      log("Kapı kontrolü: Kapı açık...");
      return 2;
    } else if(sa == kapanma_sa) {
      // dakika kontrolü yapalım...
      if(dk < kapanma_dk) {
        log("Kapı kontrolü: Kapı açık...");
        return 2;
      } else {
        // dakika eşit ya da büyük olduğunda kapanma saati geçmiş demek. kapı kapalı
        log("Kapı kontrolü: Kapı kapalı...");
        return 1;
      }
    } else {
      // kapanma saati daha küçük bu durumda kapanma saati geçmiş demek. kapı kapalı olsun
      log("Kapı kontrolü: Kapı kapalı...");
      return 1;
    }
  } else if(sa == acilma_sa) {
    // bu durumda saatler eşit dakika kontrolü yapılsın... 
    if(dk < acilma_dk) {
      // daha saati gelmemiş o zaman kapı kapalı.
      log("Kapı kontrolü: Kapı kapalı...");
      return 1; 
    } else {
      // açılma saati gelmiş ya da geçmiş. o zaman kapı açık...
      log("Kapı kontrolü: Kapı açık...");
      return 2;
    }
  } else {
    // bu durumda kapı kapalı olması gerekiyor. Gece saati...
    log("Kapı kontrolü: Kapı kapalı...");
    return 1;
  }
}

void kapiSorgula() {
  if(zamanAktifmi) {
    log("Kapı durumu sorgulanıyor...");
    // kapı şu anda hangi durumda olmalı?
    kapiDurumuNedir();
    byte kapiNeOlmali = kapiNeredeOlmali();
  
    sonSorguZamani = millis();
    
    // kapının şimdiki durumu olması gerekenden farklı ise,
    if(kapiDurum != kapiNeOlmali) {
      if(kapiNeOlmali== 1) {
        // kapalı olması gerekiyor...
        kapiIndir();
      } else {
        // kapı açık olması gerekiyor...
        kapiKaldir();
      }
    }
  }
}

void otomatikCalismaGetir() {
  int veri = (int) readEEPROM(5);
  if(veri == 0) {
    zamanAktifmi = false;
  } else if(veri == 1) {
    zamanAktifmi = true;
  } else {
    log("EEPROM dan gelen veride hata var...");
  }
}

void otomatikCalismaAyarla(bool aktif) {
  if(aktif) {
    writeEEPROM(5, 1);
  } else {
    writeEEPROM(5, 0);
  }
  log("EEPROM a yazılmış olması lazım...");
}


void writeEEPROM(uint8_t eepromAddress, byte data) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((int)(eepromAddress >> 8)); // MSB
  Wire.write((int)(eepromAddress & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
  delay(10);
}

byte readEEPROM(uint8_t eepromAddress) {
  byte rdata = 0xFF;
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((int)(eepromAddress >> 8)); // MSB
  Wire.write((int)(eepromAddress & 0xFF)); // LSB
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 1);
  if(Wire.available()) {
    rdata = Wire.read();
  }
  return rdata;
}
