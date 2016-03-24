#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

#define PIN 6
const int numberPixels = 60;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numberPixels, 6, NEO_GRB + NEO_KHZ800);

const int solidMode = 0;
const int sweepMode = 1;

int mode = 0;
int progress = 0;
int R = 0;
int G = 0;
int B = 0;

FlexCAN pixelBus = FlexCAN(1000000);
const int pixelCanID = 0x608;
FlexCAN irBus = FlexCAN(1000000);
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
  pixelBus.begin();
  irBus.begin();
}

void loop() {
  if(pixelBus.available() > 0){
    CAN_message_t rxmsg;

    if (pixelBus.read(rxmsg)) {
      uint8_t * data = (uint8_t *) malloc(8);
      if (rxmsg.id == canID){
        memcpy(data, rxmsg.buf, 8);
      }
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
      delete data;
    }
  }
  
  if(irBus.available() > 0){
    CAN_message_t rxmsg;

    if (irBus.read(rxmsg)) {
      CAN_message_t txmsg;

      txmsg.id = irCanID;
      txmsg.len = 8;

      memcpy(txmsg.buf, data, 8);
      irCanID.write(txmsg);
      
      delete data;
    }
  }

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
