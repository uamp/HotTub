#include <HotTub.h>

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
String state;

//temp, hall sensor, valve
HotTub ht1(A0,2,3);

void setup()
{
  // initialize serial:
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {  
  if (stringComplete) {
    if (inputString=="Start"){
        ht1.start();
        Serial.println("Starting");
    } else if (inputString=="Stop") {
         ht1.stop();
         Serial.println("Pausing");
    } else if (inputString=="Reset") {
         ht1.reset();
         Serial.println("Reseting");
    } else if (inputString=="Display"){
         Serial.print("Litres = ");
         Serial.println(ht1.getVolume());
         Serial.print("L/hour = ");
         Serial.println(ht1.getFlowRate());
         Serial.print("State = ");
         Serial.println(ht1.getState(&state));
         Serial.println(state);
         Serial.print("Time on state (mins) = ");
         Serial.println(ht1.getTimeOnState());
       }
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  ht1.update();
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    } 
  }
}



