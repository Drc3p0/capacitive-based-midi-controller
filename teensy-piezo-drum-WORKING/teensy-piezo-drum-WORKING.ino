
int state[] = {0,0,0,0,0,0}; // 0=idle, 1=looking for peak, 2=ignore aftershocks
int peak[] = {0,0,0,0,0,0};    // remember the highest reading
elapsedMillis msec[] = {0,0,0,0,0,0}; // timer to end states 1 and 2
const int numDrumPins = 6;
const int drumNotes[] = {38,39,40,41,42,43};
const int drumPins[] = {A0, A1, A2, A7, A8, A9};
const int channel = 1;
const int thresholdMin = 12;  // minimum reading, avoid "noise"
const int aftershockMillis = 60; // time of aftershocks & vibration

void setup() {
  Serial.begin(115200);
  //pinMode(A0, INPUT_DISABLE);
  while (!Serial && millis() < 2500) /* wait for serial monitor */ ;
  Serial.println("Piezo Peak Capture");
}

void loop() {

//while (usbMIDI.read()) { } // ignore incoming messages??

  for (uint8_t x=0; x<numDrumPins; x++) { //changed i<0 to i<numElectrodes
    int value = analogRead(drumPins[x]);

    if (state[x] == 0) {
      // IDLE state: if any reading is above a threshold, begin peak
      if (value > thresholdMin) {
        //Serial.println("begin state 1");
        state[x] = 1;
        peak[x] = value;
        msec[x] = 0;
      }
    } else if (state[x] == 1) {
      // Peak Tracking state: for 10 ms, capture largest reading
      if (value > peak[x]) {
        peak[x] = value;
      }
      if (msec[x] >= 10) {
        Serial.print("peak = ");
        Serial.println(peak[x]);
        //Serial.println("begin state 2");
        int velocity = map(peak[x], thresholdMin, 1023, 1, 127);
        usbMIDI.sendNoteOn(drumNotes[x], velocity, channel);
        state[x] = 2;
        msec[x] = 0;
      }
    } else {
      // Ignore Aftershock state: wait for things to be quiet again
      if (value > thresholdMin) {
        msec[x] = 0; // keep resetting timer if above threshold
      } else if (msec[x] > 30) {
        //Serial.println("begin state 0");
        usbMIDI.sendNoteOff(drumNotes[x], 0, channel);
        state[x] = 0; // go back to idle after 30 ms below threshold
      }
    }
  } 
}