#include <IRremote.h>
#include <IRremoteInt.h>

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

int startTime = 0;
int codeType = -1; 
unsigned long codeValue; 
unsigned int rawCodes[RAWBUF];
int codeLen; 
int toggle = 0;

int RECV_PIN = 2;
int BUTTON_PIN = 12;
int STATUS_PIN = 13;

void setup() {
  irrecv.enableIRIn(); 
  pinMode(BUTTON_PIN, INPUT);
  pinMode(STATUS_PIN, OUTPUT);
}

//storeCode and sendCode are from the example project in Arduino-IRremote
void storeCode(decode_results *results) {
  codeType = results->decode_type;
  int count = results->rawlen;
  if (codeType == UNKNOWN) {
    codeLen = results->rawlen - 1;
    for (int i = 1; i <= codeLen; i++) {
      if (i % 2) {
        rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK - MARK_EXCESS;
      }
      else {
        rawCodes[i - 1] = results->rawbuf[i] * USECPERTICK + MARK_EXCESS;
      }
    }
  }
  else {
    if (codeType == NEC) {
      if (results->value == REPEAT) {
        return;
      }
    }
   
    codeValue = results->value;
    codeLen = results->bits;
  }
}

void sendCode(int repeat) {
  if (codeType == NEC) {
    if (repeat) {
      irsend.sendNEC(REPEAT, codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else {
      irsend.sendNEC(codeValue, codeLen);
      Serial.print("Sent NEC ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == SONY) {
    irsend.sendSony(codeValue, codeLen);
    Serial.print("Sent Sony ");
    Serial.println(codeValue, HEX);
  } 
  else if (codeType == PANASONIC) {
    irsend.sendPanasonic(codeValue, codeLen);
    Serial.print("Sent Panasonic");
    Serial.println(codeValue, HEX);
  }
  else if (codeType == JVC) {
    irsend.sendPanasonic(codeValue, codeLen);
    Serial.print("Sent JVC");
    Serial.println(codeValue, HEX);
  }
  else if (codeType == RC5 || codeType == RC6) {
    if (!repeat) {
      toggle = 1 - toggle;
    }
    codeValue = codeValue & ~(1 << (codeLen - 1));
    codeValue = codeValue | (toggle << (codeLen - 1));
    if (codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(codeValue, HEX);
      irsend.sendRC5(codeValue, codeLen);
    } 
    else {
      irsend.sendRC6(codeValue, codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(codeValue, HEX);
    }
  } 
  else if (codeType == UNKNOWN) {
    // Assume 38 KHz
    irsend.sendRaw(rawCodes, codeLen, 38);
    Serial.println("Sent raw");
  }
}

void loop() {

  if(digitalRead(BUTTON_PIN)){
    startTime = millis();
  }
  if (millis()-startTime > 15000) {
    sendCode(true);
    delay(50);
  }
  else if (millis()-startTime < 15000 && irrecv.decode(&results)) {
    storeCode(&results);
    irrecv.resume();
  }
  if(millis() - startTime >15000){
        digitalWrite(STATUS_PIN, LOW);
  }else{
        digitalWrite(STATUS_PIN, HIGH);

  }
  if(millis() < 15000){
     digitalWrite(STATUS_PIN, HIGH);
     if (irrecv.decode(&results)) {
      storeCode(&results);
      
      irrecv.resume(); // resume receiver
    }
  }else{
    if(digitalRead(BUTTON_PIN)){
           digitalWrite(STATUS_PIN, HIGH);
      sendCode(false);
      delay(100);
    }else{
      digitalWrite(3, LOW);
      digitalWrite(STATUS_PIN, LOW);
    }
  }
  
}
