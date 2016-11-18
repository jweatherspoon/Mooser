#include <SerialFlash.h>

//Pin Info:
//pin2 - Relay
//pin21/A7 - LED D_out
//Audio Board



#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <FastLED.h>

#include "list.h"

// GUItool: begin automatically generated code

//Allows us to play a .wav file from SD card
AudioPlaySdWav           playSdWav1;     //xy=149.08570861816406,245.0857162475586

//Combines audio channels together. Each channel has adjustable gain
AudioMixer4              mixer1;         //xy=311.23809814453125,291.2380905151367

//Allows us to analyze the frequency of the playing note
AudioAnalyzeNoteFrequency notefreq;      //xy=496.08570861816406,291.0857162475586

//Sends audio output to the teensy audio board
AudioOutputI2S           i2s1;           //xy=516.2380981445312,205.23809051513672

//Initialize audio signals left (0) / right (1)  
AudioConnection          patchCord1(playSdWav1, 0, i2s1, 0);
AudioConnection          patchCord2(playSdWav1, 0, mixer1, 0);
AudioConnection          patchCord3(playSdWav1, 1, i2s1, 1);
AudioConnection          patchCord4(playSdWav1, 1, mixer1, 1);

//Connect output from mixer1 to the notefreq object to allow us to analyze playing note
AudioConnection          patchCord5(mixer1, notefreq);

//This object controls sending output to the teensy audio shield using I2S output object above
AudioControlSGTL5000     sgtl5000_1;     //xy=302.08570861816406,389.08573150634766
// GUItool: end automatically generated code

List songs;

//Define pins needed to read from the SDcard
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

//The number of LEDs on the visualizer
#define NUM_LEDS 6

//TODO
#define DATA_PIN 21

void AddSongs();

char *CreateFilename(int);
void PlayFile(const char *);

void GetColor();

CRGB leds[NUM_LEDS];

void setup() {
  
  //Seed the RNG
  randomSeed(analogRead(0));

  //Add the songs to the list
  AddSongs();
  
  Serial.begin(9600);

  //Allocate the memory for the audio data
  AudioMemory(30);

  //Enable the sgtl5000 which will send audio data through to the teensy audio shield
  sgtl5000_1.enable();
  //  sgt15000_1.audioPostProcessorEnable();

  //Set the volume for the audio shield
  float vol = analogRead(15) / 1024.0;
  sgtl5000_1.volume(vol);

  //Initialize the SD Card for reading
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);

      //Try to connect to the SD card
      if(SD.begin(SDCARD_CS_PIN))
        break;
    }
  }

  //Add the LEDs for the visualizer controller
    //WS2812B - type of LED
    //DATA_PIN data out
    //RGB - Order of colors (GRB could be another)
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);

  //Begin detecting frequencies from mixer1 w/ a threshold of allowed uncertainty
  notefreq.begin(.15);
}

void loop() {

  if(!playSdWav1.isPlaying()) {
    PlayFile(songs.getCurrent());
    songs.moveCurrent();
    /*
    char *filename = CreateFilename(118);
    PlayFile(filename);
    delete[] filename;
    */
  } else {
    float vol = analogRead(15) / 1024.0;
    sgtl5000_1.volume(vol);
  }

}

void InitializeSongPicks() {
  for(int i = 0; i < 118; i++) {
    
  }
}

void AddSongs() {
  bool songAdded[118] = { 0 };

  int randomIndex;
  for(int i = 0; i < 118; i++) {
    //Create a unique number
    while(true) {
      randomIndex = random(1, 119);
      if(!songAdded[randomIndex - 1]) {
        songAdded[randomIndex - 1] = true;
        break;
      }
    }

    char *filename = new char[8];
    sprintf(filename, "%i.WAV", randomIndex);

    //Add to the list
    songs.insertAtRear(filename);
    delete[] filename;
  }
}

char *CreateFilename(int num) {
  int fileNum = random(119);
  char *filename = new char[8];
  sprintf(filename, "%i.wav", fileNum);
  return filename;
}

void PlayFile(const char *filename) {
  Serial.print("Playing file: ");
  Serial.println(filename);
  
  playSdWav1.play(filename);
  
  delay(5);

  /*
  //Wait for the file to stop playing
  while(playSdWav1.isPlaying()) {
    //Volume control
    float vol = analogRead(15) / 1024.0;
    sgtl5000_1.volume(vol);
    
    //TODO:
    //  Change LED visualizer
    if(notefreq.available()) {
      GetColor(); 
    }
  }
  */
}

void GetColor() {
  float note = notefreq.read();
  float prob = notefreq.probability();
  Serial.printf("Note: %3.2f | Probability: %.2f\n", note, prob);
  (note > 50) ? digitalWrite(13, HIGH) : digitalWrite(13, LOW);
}



