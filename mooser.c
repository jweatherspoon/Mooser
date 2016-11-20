/**
 * @file mooser.c
 *
 * @author Jonathan Weatherspoon, Nico Bernt
 *
 * @version 0.86
 *      Playing around with color changing algorithms. Added lastNote / note
 *      to change led color in colors array based on the current note
 *      being played. Color will also only change if the current note is at
 *      least 7 unites different than the previous. 
 *
 * @version 0.85
 *      Still working on LED visualizer algorithm. Added colors.h file to
 *      choose color from. 
 *
 * @version 0.84
 *      Changed initial volume back to 0.5, as it isn't the actual volume
 *      But rather the signal volume. Either Get or Set color functions
 *      aren't working as intended.
 *
 * @version 0.83
 *      Added constant for number of songs
 *      
 * @version 0.82
 *      GetColor function should now map note to a color
 *      Added SetLeds function to set all LEDs to a given
 *      color. GetColor now returns CRGB object. Added code to setup
 *      which will turn on the relay to give led control to teensy
 *
 * @version 0.81
 *      Added AddSongs function and global list to store the 
 *      song names. Setup now adds all possible songs in a random
 *      order to the cyclical list.
 *
 * @version 0.80
 *      Implemented simple list class as a cyclical playlist style
 *      data structure. This stops the program from playing the 
 *      same song more than once in a loop.
 *
 * @version 0.75
 *      Testing has shown that the same file can be played over and 
 *      over again, needs to be fixed.
 *
 * @version 0.70 
 *      Changed play file function again to no longer use blocking code 
 *      Main loop now checks to see if file is playing and updates volume 
 *      if so.
 *
 * @version 0.60
 *      Changed play file function to use blocking code while file
 *      is playing. 
 *
 * @version 0.51
 *      Added volume control for music. Also changed setup volume to 
 *      the value read from the teensy audio potentiometer.
 *
 * @version 0.50
 *      Added play file function to take a filename and play that from
 *      the SD card.
 *
 * @version 0.22
 *      Added functions to generate random file from the sd card
 *      Time to play some music :)
 *
 * @version 0.21
 *      Added attempt to retry SD card 
 *
 * @version 0.20
 *      Fixed all compiler errors. Time to work on implementation
 *
 * @version 0.11
 *      Added comments to each line to dissect code.
 *
 * @version 0.10
 *      Initial commit. Code sent to me by Nico
 */

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
#include "colors.h"

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

//Contains the songs in a cyclical playlist data structure
List songs;

float lastNote = 0, note;

//Define pins needed to read from the SDcard
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

//The number of LEDs on the visualizer
#define NUM_LEDS 6

//PWM signal for LED visualizer
#define DATA_PIN 21
#define RELAY_PIN 2

//Number of songs on the SD card
#define NUM_SONGS 118

//Difference between notes before LED change
#define NOTE_DIFFERENCE 7

void AddSongs();

char *CreateFilename(int);
void PlayFile(const char *);

CRGB GetColor(float);
void SetLeds();

CRGB leds[NUM_LEDS];

void setup() {
  
  //Seed the RNG
  randomSeed(analogRead(0));

  //Add the songs to the list
  AddSongs();
  
  //Serial.begin(9600);

  //Allocate the memory for the audio data
  AudioMemory(30);

  //Enable the sgtl5000 which will send audio data through to the teensy audio shield
  sgtl5000_1.enable();
  //  sgt15000_1.audioPostProcessorEnable();

  //Set the volume for the audio shield
  //digitalWrite(15, HIGH);
  //float vol = analogRead(15) / 1024.0;
  float vol = 0.5;
  sgtl5000_1.volume(vol);

  //Initialize the SD Card for reading
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.printf("Unable to access the SD card");
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

  //Give LED Control to teensy
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
}

void loop() {
  /*
  static int col = 0;
  for(int i = 0; i < NUM_LEDS; i++)
    leds[i] = colors[col];
  delay(30);
  FastLED.show();
  col = (col + 1) % NUM_COLORS;
  delay(100);
  
  */
  //delay(5000);
  Serial.printf("pls");
  if(!playSdWav1.isPlaying()) {
    PlayFile(songs.getCurrent());
    songs.moveCurrent();
  } else {
    float vol = analogRead(15) / 1024.0;
    sgtl5000_1.volume(vol);
    //Change the color of the LEDS
    note = notefreq.read();
    float prob = notefreq.probability();
    Serial.printf("Note: %3.2f | Probability: %.2f\n", note, prob);
    if(prob > .75 && ((note > lastNote + NOTE_DIFFERENCE) || note < lastNote - NOTE_DIFFERENCE)) 
      SetLeds(GetColor(note));
    delay(30);
  }
  lastNote = note;
  
}

/**
 * @brief Add all songs to the playlist
 * @details Add all possible song names in a random order to the global
 *      list "songs"
 */
void AddSongs() {
  bool songAdded[NUM_SONGS] = { 0 };

  int randomIndex;
  for(int i = 0; i < NUM_SONGS; i++) {
    //Create a unique number
    while(true) {
      randomIndex = random(1, NUM_SONGS + 1);
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

/**
 * @brief Create a filename for a song on the SD card
 * @details Create a c style string between 1.WAV and 118.WAV
 * @return The random filename
 * @Note  The filename will have to be deleted with delete[] */
char *CreateFilename(int num) {
  int fileNum = random(NUM_SONGS + 1);
  char *filename = new char[8];
  sprintf(filename, "%i.wav", fileNum);
  return filename;
}

/**
 * @pre filename should be a valid file. Behavior undefined if not
 *    valid.
 * @brief Play the given file
 * @param filename  The name of the file to play from the SD card
 */
void PlayFile(const char *filename) {
  Serial.printf("Playing file: %s\n", filename);
  //Serial.println(filename);
  
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

/**
 * @brief Return a color based on the current note
 * @details Read the current playing note, convert to a color
 *      and return the color to the program
 * @return A CRGB object containing the converted color.
 */
CRGB GetColor(float note) {
  
  //Constrain the note, then map it to a color
  /*
  note = constrain(note, 15, 8000);
  unsigned long code = map(note, 15, 8000, 0x0000, 0x0FFF);

  CRGB color(code);
  return color;
  */
  note = constrain(note, 30, 1000);
  int index = map(note, 30, 1000, 0, NUM_COLORS - 1);
  Serial.printf("index: %i\n", index);
  //int index = random(0, NUM_COLORS);
  return colors[index];
}

/**
 * @brief Set all LEDs to the given color
 * @param color  The CRGB object containing the desired color
 */
void SetLeds(CRGB color) {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}



