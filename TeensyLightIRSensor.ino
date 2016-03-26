#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include <FlexCAN.h>

#define PIN 6
#define IR_ANALOG 0
const int numberPixels = 60;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numberPixels, 6, NEO_GRB + NEO_KHZ800);

const int solidMode = 0;
const int sweepMode = 1;

int mode = 0;
int progress = 0;
int R = 0;
int G = 0;
int B = 0;

int runningInfraredTotal = 0;
int infraredTicker = 0;
int latestInfraredValue = 0;
unsigned int lastInfraredSampleEpoch = 0;

FlexCAN canBus = FlexCAN(1000000);
const int pixelCanID = 0x608;
const int irCanID = 0x611;

void setup() {
  pixels.begin();
  Serial.begin(9600);
  Serial.println("Begin Teensy Connection");
  mode = 0;
  progress = 0;
  R = 0;
  G = 0;
  B = 0;
  canBus.begin();
}

void loop() {
  takeInfraredValues();
  if(canBus.available() > 0){
    CAN_message_t rxmsg;

    if (canBus.read(rxmsg)) {
      uint8_t * data = (uint8_t *) malloc(8);
      if (rxmsg.id == pixelCanID){
        memcpy(data, rxmsg.buf, 8);
        mode = data[6] + (data[7] << 8);
        progress = data[4] + (data[5] << 8);
        R = data[2];
        G = data[1];
        B = data[0];
        Serial.print(mode);
        Serial.print("\t");
        Serial.print(progress);
        Serial.print("\t");
        Serial.print(R);
        Serial.print("\t");
        Serial.print(G);
        Serial.print("\t");
        Serial.print(B);
        Serial.println();
      }
      else if (rxmsg.id == irCanID) {
        CAN_message_t txmsg;

        txmsg.id = irCanID;
        txmsg.len = 8;

        data[0] = latestInfraredValue & 0xff;
        data[1] = (latestInfraredValue >> 8) & 0xff;
        data[2] = (latestInfraredValue >> 16) & 0xff;
        data[3] = (latestInfraredValue >> 24) & 0xff;

        data[4] = 0; // mode

        for(int k = 5; k < 8; k++){
          data[k] = 0;
        }

        memcpy(txmsg.buf, data, 8);
        canBus.write(txmsg);
      }
      
      delete data;
    }
  }

  lights();
}

void lights(){
  switch(mode){
    default:
      solid();
  }

  pixels.show();
}

void solid(){
  for(int i = 0; i < numberPixels; i++){
    pixels.setPixelColor(i, pixels.Color(R, G, B));
  }
}


void takeInfraredValues(){
  unsigned int now = millis();
  if (now - lastInfraredSampleEpoch > 10){ // every 10 ms
    lastInfraredSampleEpoch = now;
    runningInfraredTotal += analogRead(0);
    ++infraredTicker;
    if (infraredTicker%20 == 0){ // every 20 samples
      latestInfraredValue = runningInfraredTotal / infraredTicker;
      infraredTicker = 0;
      runningInfraredTotal = 0;
      Serial.println(latestInfraredValue);
    }
  }
}
