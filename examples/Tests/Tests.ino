
/**
 * Test cases for the  SpektrumSatellite class
 */

#include "SpektrumSatellite.h"
#include "SpektrumCSV.h"
#include "Scaler.h"

void testScaling() {
  Serial.println("***********************");
  Serial.println("testScaling");
  Scaler<uint16_t> s;
  s.setValues(0,10,0,2048);

  for (int j=0;j<10;j++){
      int scaled = s.scale(j);
      int back = s.deScale(scaled);
      Serial.print(j);
      Serial.print("->");
      Serial.print(scaled);
      Serial.print("->");
      Serial.print(back);
      Serial.print(" -> ");
      Serial.println(j == back ? "OK" : "failed");
  }
}

void testIs2048() {
  Serial.println("***********************");
  SpektrumSatellite<uint16_t> satellite(Serial); 

  Serial.print("testIs2048 => ");
  Serial.println(satellite.is2048()?"ok" : "failed");

}

void testIs1024() {
  Serial.println("***********************");
  SpektrumSatellite<uint16_t> satellite(Serial); 
  satellite.setBindingMode(Internal_DSM2_22ms);
  Serial.print("testIs1024 => ");
  Serial.println(!satellite.is2048()?"ok" : "failed");
}


void testRange1000() {
  Serial.println("***********************");
  Serial.println("testRange1000");
  SpektrumSatellite<uint16_t> satellite(Serial); 
  satellite.setChannelValueRange(0, 1000);

  satellite.setThrottle(1000);

  Serial.print("testRange1000  getThrottle => ");
  Serial.println(satellite.getThrottle() == 1000 ?"ok" : "failed");

  Serial.print("testRange1000  getChannelValue => ");
  Serial.println(satellite.getChannelValue(Throttle) == 1000 ?"ok" : "failed");

  Serial.print("testRange1000 raw =>> ");
  Serial.println(satellite.getChannelValuesRaw()[Throttle] == 2048 ?"ok" : "failed");

}


void testFloat() {
  Serial.println("***********************");
  Serial.println("testFloat");
  SpektrumSatellite<float> satellite(Serial); 
  satellite.setChannelValueRange(-1.0, 1.0);

  satellite.setThrottle(1.0);

  Serial.print("testFloat getChannelValuesRaw => ");
  Serial.println(satellite.getChannelValuesRaw()[Throttle] == 2048?"ok" : "failed");

  Serial.print("testFloat getThrottle => ");
  Serial.println(satellite.getThrottle() == 1.0 ? "ok" : "failed");

  Serial.print("testFloat getChannelValue(Throttle) => ");
  Serial.println(satellite.getChannelValue(Throttle) == 1.0 ? "ok" : "failed");

}

void testCSV() {
  Serial.println("***********************");
  Serial.println("testCSV");
  SpektrumSatellite<uint16_t> satellite(Serial); 
  SpektrumCSV<uint16_t> csv;
  satellite.setChannelValueRange(0, 11);

  for (int j=0;j<MAX_CHANNELS;j++){
      satellite.setChannelValue((Channel)j,j);
  }

  uint8_t buffer[1000];
  csv.toString(satellite, buffer, 1000);

  char* expected = "0.00,1.00,2.00,3.00,4.00,5.00,6.00,7.00,8.00,9.00,10.00,11.00\n";
  Serial.print(expected);
  Serial.print((char*)buffer);
  Serial.print("testCSV toString => ");
  Serial.println(strcmp((char*)buffer,expected)==0 ? "ok" : "failed");

  // reset vlues
  for (int j=0;j<MAX_CHANNELS;j++){
      satellite.setChannelValue((Channel)j,0);
  }
  // read csv values
  csv.parse(buffer, satellite);

  // check the data
  for (int j=0;j<MAX_CHANNELS;j++){
      Serial.print("testCSV ");
      Serial.print(j);
      Serial.print(" => ");
      Serial.print(satellite.getChannelValue((Channel)j));
      Serial.println(satellite.getChannelValue((Channel)j) == j ? " OK" : " Error");
  }
}

void testBinary() {
  Serial.println("***********************");
  Serial.println("testBinary ");
  SpektrumSatellite<uint16_t> satellite(Serial); 
 // satellite.setChannelValueRange(0, 11);

  for (int j=0;j<MAX_CHANNELS;j++){
      satellite.setChannelValue((Channel)j,j);
  }
  
  // get and parse spektrum data
  byte* buffer = satellite.getSendBuffer(false);
  satellite.parseFrame(buffer);
  
  buffer = satellite.getSendBuffer(true);
  satellite.parseFrame(buffer);

  // check the data
  for (int j=0;j<MAX_CHANNELS;j++){
      Serial.print("testBinary ");
      Serial.print(j);
      Serial.print(" => ");
      Serial.print(satellite.getChannelValue((Channel)j));
      Serial.println(satellite.getChannelValue((Channel)j) == j ? " OK" : " Error");
  }
}

void testWaitForData() {
  Serial.println("***********************");
  Serial.println("testCSV waitForData -> ");
  SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)

  satellite.waitForData();
  Serial.println("==> ok");

}

void testHeader() {
  Serial.println("***********************");
  Serial.println("testHeader ");
  SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
  satellite.setSystem(DSMS_22MS_2048);
  Serial.print("system ->");
  Serial.println(satellite.getSystem()==DSMS_22MS_2048?"OK":"Error");
  byte* buffer = satellite.getSendBuffer(false);
  Serial.print("buffer: ");
  Serial.print(buffer[0]);
  Serial.print(" ");
  Serial.println(buffer[1]);

  satellite.parseFrame(buffer);

  Serial.print("system: ");
  Serial.println(satellite.getSystem());

  Serial.print("system ->");
  Serial.println(satellite.getSystem()==DSMS_22MS_2048?"OK":"Error");
  Serial.print("isValidSystem ->");
  Serial.println(satellite.isValidSystem(satellite.getSystem())?"OK":"Error");
  Serial.print("system ->");
  Serial.println(satellite.getFades()==0?"OK":"Error");

}


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("***********************");
  Serial.println("Tests:");
  Serial.println("***********************");
  testScaling();
  testIs2048();
  testIs1024();
  testRange1000();
  testFloat();
  testHeader();
  testCSV();
  testBinary();
  testWaitForData();
}

void loop() {
}