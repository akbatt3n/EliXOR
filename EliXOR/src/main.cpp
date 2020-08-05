#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>

// #define _DEBUG_

// GUItool: begin automatically generated code
AudioInputI2S            in;             //xy=187,138
AudioEffectWaveshaper    waveshape;      //xy=367,162
AudioEffectBitcrusher    bitcrusher;
AudioEffectFreeverb      freeverb;
AudioEffectFlange        flange;
AudioMixer4              mixer;
AudioOutputI2S           out;            //xy=689,159
AudioConnection          patchCord1(in, 1, waveshape, 0);
AudioConnection          patchCord2(in, 1, bitcrusher, 0);
AudioConnection          patchCord3(in, 1, freeverb, 0);
// save patchCord4 for flange later
AudioConnection          patchCord4(in, 1, flange, 0);
AudioConnection          patchCord5(waveshape, 0, mixer, 0);
AudioConnection          patchCord6(bitcrusher, 0, mixer, 1);
AudioConnection          patchCord7(freeverb, 0, mixer, 2);
AudioConnection          patchCord8(flange, 0, mixer, 3);
AudioConnection          patchCord10(mixer, 0, out, 0);
AudioConnection          patchCord11(mixer, 0, out, 1);
AudioControlSGTL5000     sgtl5000;       //xy=479,74
// GUItool: end automatically generated code

///////////////////////////////////////////////////////////////
// if in debug mode, declare a few extra objects for reading //
// the state of the pedal                                    //
///////////////////////////////////////////////////////////////
#ifdef _DEBUG_
    AudioAnalyzePeak peak1;
    AudioAnalyzePeak peak2;
    AudioConnection patchCord50(in, 1, peak1, 0);
    AudioConnection patchCord51(mixer, 0, peak2, 0);
#endif

///////////////////////////////////////////////////////////////
// define the length of the array used in the waveshape      //
// along with pins for all inputs and outputs                //
///////////////////////////////////////////////////////////////
#define LENGTH 17
#define LINE_IN_LEVEL 7 // higher value = lower line voltage. 5 is default

#define EFF_PIN 22
#define VOL_PIN 15
#define BTN1 1
#define BTN2 2
#define LED_T 17
#define LED_B 16
#define NUM_EFFECTS 4

#define SAMP_RATE_LOW 750
#define SAMP_RATE_HIGH 10000
#define CHANGE_THRESHOLD 30

///////////////////////////////////////////////////////////////
// these values taken from the flange example in the library //
///////////////////////////////////////////////////////////////
#define FLANGE_DELAY_LENGTH (6*AUDIO_BLOCK_SAMPLES)
short delayLine[FLANGE_DELAY_LENGTH];
int flangeOffset = FLANGE_DELAY_LENGTH / 4;
int flangeDepth = FLANGE_DELAY_LENGTH / 4;      // equal to the amplitude of the flange's modulator. sets how far the 2nd voice will swing
float flangeFreq = 0.0;                         // frequency at which the flange's modulator modulates. Main control for flange effect
#define FLANGE_FREQ_MAX 2.0
#define FLANGE_FREQ_MIN 0.1

///////////////////////////////////////////////////////////////
// include a custom library for printing debug info and      //
// effect controls                                           //
///////////////////////////////////////////////////////////////
#include "teensyPedal.h"

// These arrays represent the maximum and minimum amount of fuzz possible
// maxFuzz is somewhat random. The goal is asymmetric, somewhat soft clipping
float refPassthrough[] = {-1.0, -0.875, -0.75, -0.625, -0.5, -0.375, 
            -0.25, -0.125, 0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875, 1.0};

float refMaxFuzz[] = {-1.0, -0.966, -0.933, -0.9, -0.87, -0.82, 
            -0.75, -0.2, 0.0, 0.2, 0.5, 0.55, 0.60, 0.65, 0.7, 0.7, 0.7};

float fuzzArray[LENGTH] = {0.0};

float effCtrlVal = 0;
float prevVal = 0;
float prevVolume = 0;
int currentEffect = 0;

Bounce effBtn = Bounce(BTN1, 20);
Bounce volBtn = Bounce(BTN2, 20);

elapsedMillis updateTimer;

void setup() {

    //////////////////////////////////////////////////
    // set pin modes                                //
    //////////////////////////////////////////////////
    pinMode(EFF_PIN, INPUT);
    pinMode(BTN1, INPUT_PULLUP);
    pinMode(BTN2, INPUT_PULLUP);
    pinMode(LED_T, OUTPUT);
    pinMode(LED_B, OUTPUT);

    //////////////////////////////////////////////////
    // set up audio adaptor                         //
    //////////////////////////////////////////////////
    AudioMemory(20);
    sgtl5000.enable();
    sgtl5000.volume(0.5); // only affects headphone out
    sgtl5000.inputSelect(AUDIO_INPUT_LINEIN);
    sgtl5000.lineInLevel(LINE_IN_LEVEL);

    //////////////////////////////////////////////////
    // set up effects                               //
    //////////////////////////////////////////////////
    effCtrlVal = analogRead(EFF_PIN);
    prevVal = 20000;
    adjustFuzz(effCtrlVal, fuzzArray, refPassthrough, refMaxFuzz, prevVal);
    bitcrusher.bits(16);
    bitcrusher.sampleRate(44100);
    freeverb.roomsize(0.0);
    freeverb.damping(0.0);
    flange.begin(delayLine, FLANGE_DELAY_LENGTH, flangeOffset, flangeDepth, flangeFreq);
    updateMixer(currentEffect);
    updateVolume(currentEffect, prevVolume);

    #ifdef _DEBUG_
        Serial.begin(9600);
    #endif

}

void loop() {

    effBtn.update();
    volBtn.update();

    if (volBtn.fallingEdge()) {
        currentEffect = (currentEffect + 1) % NUM_EFFECTS;
        displayStatus(currentEffect);
        updateMixer(currentEffect);
        prevVal = 20000; // set this so that effect update functions will always set their parameters on the first udpate after a change
    }

    // only update effect parameters every 250ms
    if (updateTimer > 250) {
        updateVolume(currentEffect, prevVolume);
        switch (currentEffect) {
            case 0:
                adjustFuzz(effCtrlVal, fuzzArray, refPassthrough, refMaxFuzz, prevVal);
                break;
            case 1:
                adjustBitcrusher(effCtrlVal, prevVal);
                break;
            case 2:
                adjustReverb(effCtrlVal, prevVal);
                break;
            case 3:
                adjustFlange(flangeFreq, effCtrlVal, prevVal);
                break;
            default:
                break;
        }

        #ifdef _DEBUG_
            printDebugInfo();
        #endif
    }
    else updateTimer = 0;
}