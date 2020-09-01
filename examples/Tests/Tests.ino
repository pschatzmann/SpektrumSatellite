

/**
 * Test cases for the  SpektrumSatellite class
 */

#include "SpektrumSatellite.h"


void testIs2048() {
  SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
  satellite.setLog(Serial);

  Serial.print("testIs2048 -> ");
  Serial.println(satellite.is2048()?"ok" : "failed");

}

void testRange1000() {
  SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
  satellite.setLog(Serial);
  satellite.setChannelValueRange(0, 1000);

  satellite.setThrottle(1000);

  Serial.print("testRange1000 raw-> ");
  Serial.println(satellite.getChannelValuesRaw()[Throttle] == 2048 ?"ok" : "failed");

  Serial.print("testRange1000  getThrottle -> ");
  Serial.println(satellite.getThrottle() == 1000 ?"ok" : "failed");

  Serial.print("testRange1000  getChannelValue -> ");
  Serial.println(satellite.getChannelValue(Throttle) == 1000 ?"ok" : "failed");
}


void testFloat() {
  SpektrumSatellite<float> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
  satellite.setLog(Serial);
  satellite.setChannelValueRange(-1.0, 1.0);

  satellite.setThrottle(1.0);

  Serial.print("testFloat getChannelValuesRaw -> ");
  Serial.println(satellite.getChannelValuesRaw()[Throttle] == 2048?"ok" : "failed");

  Serial.print("testFloat getThrottle-> ");
  Serial.println(satellite.getThrottle() == 1.0 ? "ok" : "failed");

  Serial.print("testFloat getChannelValue(Throttle)-> ");
  Serial.println(satellite.getChannelValue(Throttle) == 1.0 ? "ok" : "failed");

}

void testCSV() {
  SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
  SpektrumCSV<uint16_t> csv(',');
  satellite.setLog(Serial);
  satellite.setChannelValueRange(0, 1000);
  satellite.setLog(Serial);

  for (int j=0;j<MAX_CHANNELS;j++){
      satellite.setChannelValue((Channel)j,j);
  }

  uint8_t buffer[1000];
  csv.toString(satellite, buffer, 1000);

  char* expected = "0,1,2,3,4,5,6,7,8,9,10,11\n";
  Serial.println(expected);
  Serial.println((char*)buffer);
  Serial.print("testCSV toString -> ");
  Serial.println(strcmp((char*)buffer,expected)?"ok" : "failed");


  for (int j=0;j<MAX_CHANNELS;j++){
      satellite.setChannelValue((Channel)j,0);
  }
  csv.parse(buffer, satellite);

  Serial.print("testCSV parse Throttle -> ");
  Serial.println(satellite.getChannelValue(Throttle)==0 ? "ok" : "failed");
  Serial.print("testCSV parse Aux7 -> ");
  Serial.println(satellite.getChannelValue(Aux7)==11 ? "ok" : "failed");

}

void testBinary() {
  SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
  satellite.setLog(Serial);
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
      Serial.print(" -> ");
      Serial.println(satellite.getChannelValue((Channel)j) == j ? "OK" : "Error");
  }

}



void testWaitForData() {
  SpektrumSatellite<uint16_t> satellite(Serial); // Assing satellite to Serial (use Serial1 or Serial2 if available!)
  satellite.setLog(Serial);

  Serial.print("testCSV waitForData -> ");
  satellite.waitForData();
  Serial.println("ok");

}


void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("tests:");

  testIs2048();
  testRange1000();
  testFloat();
  testCSV();
  testBinary();
  testWaitForData();
}

void loop() {
}


