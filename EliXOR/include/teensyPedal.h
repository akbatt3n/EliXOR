#include <Arduino.h>
#include <Audio.h>

#ifdef _DEBUG_
void printDebugInfo(int effCtrlVal, float fuzzArray[]) {

    Serial.print("Effect Knob: ");
    Serial.print(effCtrlVal);

    Serial.print(" || Input Peak: ");
    if (peak1.available()) Serial.print(peak1.read());
    else Serial.print("(not available)");

    Serial.print(" || Waveshape Peak: ");
    if (peak2.available()) Serial.print(peak2.read());
    else Serial.print("(not available)");

    // Serial.println();
    // for (int i=0; i < LENGTH; i++) {
    //     Serial.print(fuzzArray[i]);
    //     Serial.print(", ");
    // }
    // Serial.println();

    Serial.println();
}
#endif

void adjustFuzz(float effCtrlVal, float fuzzArray[], float refPassthrough[], float refMaxFuzz[]) {
    effCtrlVal = analogRead(EFF_PIN);
    for (int i=0; i < LENGTH; i++) {
        fuzzArray[i] = map(effCtrlVal, 0, 1023, refMaxFuzz[i], refPassthrough[i]);
    }
    waveshape.shape(fuzzArray, LENGTH);
}

void adjustBitcrusher(float effCtrlVal, float prevVal) {
    effCtrlVal = analogRead(EFF_PIN);
    if (effCtrlVal > prevVal+20 || effCtrlVal < prevVal-20) {
        prevVal = effCtrlVal;
        int sampRate = map(effCtrlVal, 1023, 0, SAMP_RATE_HIGH, SAMP_RATE_LOW);
        bitcrusher.sampleRate(sampRate);
    }
}

void adjustReverb(float effCtrlVal) {
    effCtrlVal = analogRead(EFF_PIN);
    float val = map(effCtrlVal, 1023, 0, 0.1, 1.0);
    freeverb.roomsize(val);
    freeverb.damping(val);
}

void adjustFlange() {
    
}

void updateMixer(int currentEffect) {
    float vol = analogRead(VOL_PIN);
    vol = map(vol, 1023, 0, 0.0, 1.0);
    
    mixer.gain(0, 0.0);
    mixer.gain(1, 0.0);
    mixer.gain(2, 0.0);
    mixer.gain(3, 0.0);
    mixer.gain(currentEffect, vol);
}

void displayStatus(int currentEffect) {
    digitalWrite(LED_T, bitRead(currentEffect, 1));
    digitalWrite(LED_B, bitRead(currentEffect, 0));
}