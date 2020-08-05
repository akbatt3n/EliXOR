#include <Arduino.h>
#include <Audio.h>

#ifdef _DEBUG_
void printDebugInfo() {

    Serial.print("Volume Knob: ");
    Serial.print(analogRead(VOL_PIN));

    Serial.print(" || Effect Knob: ");
    Serial.print(analogRead(EFF_PIN));

    Serial.print(" || Current Effect: ");
    Serial.print(currentEffect);

    Serial.println();
}
#endif

void adjustFuzz(float effCtrlVal, float fuzzArray[], float refPassthrough[], float refMaxFuzz[], float &prevVal) {
    effCtrlVal = analogRead(EFF_PIN);
    if (effCtrlVal > (prevVal + CHANGE_THRESHOLD) || effCtrlVal < (prevVal - CHANGE_THRESHOLD)) {
        prevVal = effCtrlVal;
        for (int i=0; i < LENGTH; i++) {
            fuzzArray[i] = map(effCtrlVal, 0, 1023, refMaxFuzz[i], refPassthrough[i]);
        }
        waveshape.shape(fuzzArray, LENGTH);
    }
}

void adjustBitcrusher(float effCtrlVal, float &prevVal) {
    effCtrlVal = analogRead(EFF_PIN);
    if (effCtrlVal > (prevVal + CHANGE_THRESHOLD) || effCtrlVal < (prevVal - CHANGE_THRESHOLD)) {
        prevVal = effCtrlVal;
        int sampRate = map(effCtrlVal, 1023, 0, SAMP_RATE_HIGH, SAMP_RATE_LOW);
        bitcrusher.sampleRate(sampRate);
    }
}

void adjustReverb(float effCtrlVal, float &prevVal) {
    effCtrlVal = analogRead(EFF_PIN);
    if (effCtrlVal > (prevVal + CHANGE_THRESHOLD) || effCtrlVal < (prevVal - CHANGE_THRESHOLD)) {
        prevVal = effCtrlVal;
        float val = map(effCtrlVal, 1023, 0, 0.05, 1.0);
        freeverb.roomsize(val);
        freeverb.damping(val);
    }
}

void adjustFlange(float flangeFreq, float effCtrlVal, float &prevVal) {
    effCtrlVal = analogRead(EFF_PIN);
    if (effCtrlVal > (prevVal + CHANGE_THRESHOLD) || effCtrlVal < (prevVal - CHANGE_THRESHOLD)) {
        prevVal = effCtrlVal;
        flangeFreq = map(effCtrlVal, 1023, 0, FLANGE_FREQ_MIN, FLANGE_FREQ_MAX);
        flange.voices(flangeOffset, flangeDepth, flangeFreq);
    }
}

void updateMixer(int currentEffect) {
    float vol = analogRead(VOL_PIN);
    vol = map(vol, 1023, 0, 0.0, 0.95);
    
    mixer.gain(0, 0.0);
    mixer.gain(1, 0.0);
    mixer.gain(2, 0.0);
    mixer.gain(3, 0.0);
    mixer.gain(currentEffect, vol);
}

void updateVolume(int currentEffect, float &prevVolume) {
    float vol = analogRead(VOL_PIN);
    if (vol > (prevVolume + CHANGE_THRESHOLD) || vol < (prevVolume - CHANGE_THRESHOLD)) {
        prevVolume = vol;
        vol = map(vol, 1023, 0, 0.0, 0.95);
        mixer.gain(currentEffect, vol);
    }
}

void displayStatus(int currentEffect) {
    digitalWrite(LED_T, bitRead(currentEffect, 1));
    digitalWrite(LED_B, bitRead(currentEffect, 0));
}