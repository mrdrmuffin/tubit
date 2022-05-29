#include <Adafruit_VS1053.h>

#define VS1053_RESET 9
// Solder closed jumper on bottom!

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define OCARINA 80
#define TRUMPET 57
#define TUBA 59
#define FLUTE 74
#define GUITAR_S 26
#define SITAR 105
#define BIRD 124

#define TAIKO_DRUM 117
#define SYNTH_DRUM 119


#define INSTRUMENT OCARINA

#define MIDI_CHAN_EXPRESSION  0x0B
#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0
#define MIDI_CHAN_NO_SOUND 0x78


// Note buttons
#define BTN_N0 A1
#define BTN_N1 A2
#define BTN_N2 A3
#define BTN_N3 A4

// Play buttons
#define BTN_OL 10
#define BTN_OM 11
#define BTN_OH 12
#define BTN_OE 13

// Other buttons
#define BTN_INSTRUMENT_SWITCH 6

// Note starting points
#define NOTE_C3 48
#define NOTE_C4 60
#define NOTE_C5 72
#define NOTE_C6 84

// Old notes?
#define NOTE_0 54
#define NOTE_1 66
#define NOTE_2 78

#define VOL_TOP_CUTOFF 540
#define VOL_TOP_RANGE (1024 - VOL_TOP_CUTOFF)

#define VS1053_MIDI Serial1

#if defined(__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined(TEENSYDUINO) || defined(ARDUINO_STM32_FEATHER)
  #define VS1053_MIDI Serial1
#elif defined(ESP32)
  HardwareSerial Serial1(2);
  #define VS1053_MIDI Serial1
#elif defined(ESP8266)
  #define c Serial
#endif

bool btn_ol_curr = false;
bool btn_ol_last = false;
bool btn_om_curr = false;
bool btn_om_last = false;
bool btn_oh_curr = false;
bool btn_oh_last = false;
bool btn_oe_curr = false;
bool btn_oe_last = false;

// 
int b0_curr = 0;
int b1_curr = 0;
int b2_curr = 0;
int b3_curr = 0;
int vol_curr = 0;
int vol_last = 0;
int note_curr = 0;
int note_last = 0;
int note_temp = 0xFF;
bool play_last = false;
bool play_curr = false;

bool btn_instrument_last = false;
bool btn_instrument_curr = false;
int curr_instrument = 0;
int instruments[] = {OCARINA, TRUMPET, TAIKO_DRUM, SYNTH_DRUM};
int num_instruments = 4; //sizeof(instruments) / sizeof(int);

void setup() {
  delay(1000);
  
  Serial.begin(115200);

  Serial.println("VS1053 MIDI test");

  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate'
  
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  delay(100);
  
  midiSetChannelVolume(0, 127);
  delay(100);
  midiSetChannelExpression(0, 90);
  delay(100);

  midiSetInstrument(0, instruments[curr_instrument]);
  delay(100);

  midiTurnOffSound(0);
  delay(100);

  pinMode(BTN_N0, INPUT_PULLUP);
  pinMode(BTN_N1, INPUT_PULLUP);
  pinMode(BTN_N2, INPUT_PULLUP);
  pinMode(BTN_N3, INPUT_PULLUP);

  pinMode(BTN_INSTRUMENT_SWITCH, INPUT_PULLUP);
  
  pinMode(BTN_OL, INPUT_PULLUP);
  pinMode(BTN_OM, INPUT_PULLUP);
  pinMode(BTN_OH, INPUT_PULLUP);
  pinMode(BTN_OE, INPUT_PULLUP);
}

void loop() {
    btn_instrument_curr = digitalRead(BTN_INSTRUMENT_SWITCH) == LOW;

    if(btn_instrument_curr != btn_instrument_last){
        delay(100);
        if(btn_instrument_last == true){
            curr_instrument++;
            if(curr_instrument >= num_instruments){
                curr_instrument = 0;
            }
            midiSetInstrument(0, instruments[curr_instrument]);
            delay(100);
        }
        btn_instrument_last = btn_instrument_curr;
    }
    else if(btn_instrument_curr == false){
        play_loop();
    }
}


void play_loop() {
  btn_ol_curr = digitalRead(BTN_OL) == LOW;
  btn_om_curr = digitalRead(BTN_OM) == LOW;
  btn_oh_curr = digitalRead(BTN_OH) == LOW;
  btn_oe_curr = digitalRead(BTN_OE) == LOW;

  b0_curr = digitalRead(BTN_N0) == LOW ? 1 : 0;
  b1_curr = digitalRead(BTN_N1) == LOW ? 2 : 0;
  b2_curr = digitalRead(BTN_N2) == LOW ? 4 : 0;
  b3_curr = digitalRead(BTN_N3) == LOW ? 8 : 0;

  if(btn_ol_curr){
    note_curr = NOTE_C3;
  }
  else if(btn_om_curr){
    note_curr = NOTE_C4;
  }
  else if(btn_oh_curr){
    note_curr = NOTE_C5;
  }
  else if(btn_oe_curr){
    note_curr = NOTE_C6;
  }
  else {
    note_curr = 0;
  }

  if(note_curr > 0){
    note_temp = b0_curr | b1_curr | b2_curr | b3_curr;
    note_curr += note_temp;
  }

  if(note_curr != note_last){
    midiNoteOff(0, note_last, 127);
    if(note_curr > 0){
      delay(50);
      midiNoteOn(0, note_curr, 127);
    }
  }
  
  note_last = note_curr;

  delay(50);
}

void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);  
  delay(10);
  VS1053_MIDI.write(inst);
  delay(10);
}

void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelExpression(uint8_t chan, uint8_t expression) {
  if (chan > 15) return;
  if (expression > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_EXPRESSION);
  VS1053_MIDI.write(expression);
}

void midiTurnOffSound(uint8_t chan) {
  if (chan > 15) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_NO_SOUND);
}

void midiSetMasterVolume(uint8_t vol) {
  if (vol > 127) return;
  
  VS1053_MIDI.write(0x01);
//  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}
