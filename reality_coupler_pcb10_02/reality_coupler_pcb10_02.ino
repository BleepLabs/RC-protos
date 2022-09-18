#define TOUCH_PIN 7
#define SENSOR 6
#define POT_TOP 1
#define POT_MIDDLE 3
#define ADJ_BUTTON 9
#define CALIBRATING_button 2
#define JACK_SWITCH_PIN 8
#define LED_PIN 10
#define CV 0
#define T1 4
#define T2 5
#define T2 5
#define flip_jack 1
byte lock_out_period = 6;
byte pot_tick;
int f1, f2, intpot;
int touch1;
float adj_color;

#include "Adafruit_FreeTouch.h"
Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(TOUCH_PIN, OVERSAMPLE_4, RESISTOR_100K, FREQ_MODE_NONE);
#include <TimerTC3.h>

#include <Adafruit_NeoPixel.h>
byte num_leds = 43;
Adafruit_NeoPixel leds = Adafruit_NeoPixel( num_leds, LED_PIN, NEO_RGB + NEO_KHZ800 );

byte tick;

#include <FlashAsEEPROM.h>

#include <Bounce2.h>
Bounce bottom_buttom = Bounce();
Bounce top_buttom = Bounce();

#define BOUNCE_LOCK_OUT
uint32_t cm, pm, pm2;
int in[8];
int pbutt[2], butt[2];
int set_low = 0;
int set_high = 1023;
int fout;
int thresh;
int test_out, test_dir;
int raw_in;
int sense_exp;
int sense_ampcal;
int sense_smooth;
int pot_raw[3];
const int ppc_len = 50;
int prev_pot_raw[3][ppc_len];
int prev_pot_counter[3];

int trails[8];
const int trail_len = 12;
int trail_count;
int ledout, prev_ledout;
int smooth_amount_s1;
int smooth_s1;
int smooth_s1_c;
int32_t du, cu;
int prev_pos[trail_len];
float fade_level[trail_len];
int smooth_pot;
int smooth_amount;
int smooth_bank[1025];
int smooth_bank_s1[200];
int smooth_count;
int sm_out;
int real_fout;
int set_mode_out;
//int sm_mult[16] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 256, 256, 256, 256, 256, 256, 256};
byte tout;
byte high_is_set = 0;
byte low_is_set = 0;
byte test_mode = 0;


int clear_count;
byte clear_hi_low;
int high_read = 0;
int low_read = 2000;
int adj_led;
int cal_led;
int blinko;
int blinko_count;
int jack_switch;
int16_t cal_count;
int32_t cal_time;
float cal_color = .2;
byte tick_out;
int led_lerp;
int gap_size;
int sm_mode;
int sr, tr;

int sm_set;
int final_sm_out;
float prev_follow_out, follow_out;
float follow_amount;
float g1;
float amp_pot;
int exp_in;
byte setting_out_mode;
int32_t setting_out_mode_timer;
byte out_mode = 3;
byte amp_pot_change;
byte thresh_pot_change;
byte setting_thresh_size;
int thresh_size = 0;
byte smooth_pot_change;
byte setting_bright;
float max_brightness = .05;
int led_fout;
int  sm_low = 9;
byte set_mode;
float grad_amount = 200.0; //gradient distance
float sm_pot_center = 512;
int sm_gap = 75;
int prev_sm_mode;
int prev_smooth_amount;
int jack_a;

#define IDLE 10
#define CALIBRATING 20

int sermode = 1;
int compc = 0x2000;
int intc = 0x3F;
byte smode;
byte blink0;
byte lock_out;
byte lock_count;
byte prev_tout;
byte ftout;

void printer() {


  if (1) {
    Serial.print(raw_in);
    Serial.print(" ");
    Serial.print(pot_raw[0]);
    Serial.print(" ");
    Serial.println(pot_raw[1]);
  }

}


void setup() {
  //SerialUSB.begin(115200);
  //while(!SerialUSB);
  leds.begin();           //initialize Adafruit_NeoPixel
  leds.show();
  pinMode(13, OUTPUT);
  pinMode(T1, OUTPUT);
  pinMode(T2, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(JACK_SWITCH_PIN, INPUT); //has pull up resistor

  pinMode(CALIBRATING_button, INPUT_PULLUP);
  bottom_buttom.attach( CALIBRATING_button , INPUT_PULLUP  );       //setup the bounce instance for the current bottom_buttom
  bottom_buttom.interval(5);             // interval in ms


  pinMode(ADJ_BUTTON, INPUT_PULLUP);
  top_buttom.attach( ADJ_BUTTON , INPUT_PULLUP  );       //setup the bounce instance for the current bottom_buttom
  top_buttom.interval(5);             // interval in ms

  qt_1.setCompCap(0x2000);  //0-16383, default  0x2000, 8192
  qt_1.setIntCap(63); // 0 - 63, default 0x3F, 63
  delay(100);
  qt_1.begin();


  //(f >> 24) / 255.0;
  if (EEPROM.isValid()) {  //eeep is erased on upload

    max_brightness = EEPROM.read(0) / 255.0;
    out_mode = EEPROM.read(1);
    thresh_size = (EEPROM.read(2) << 8) + EEPROM.read(3);
    set_low = (EEPROM.read(4) << 8) + EEPROM.read(5);
    set_high = (EEPROM.read(6) << 8) + EEPROM.read(7);
  }

  else {
    //if (1) {
    max_brightness = .05;
    out_mode = 3;
    thresh_size = 0;
    set_low = 0;
    set_high = 1023;
    if (1) {
      EEPROM.write(0, max_brightness * 255);
      EEPROM.write(1, out_mode);
      EEPROM.write(2, thresh_size >> 8);
      EEPROM.write(3, byte(thresh_size));
      EEPROM.commit();
    }
  }

  bottom_buttom.update();
  top_buttom.update();

  if (bottom_buttom.read() == 0 &&
      top_buttom.read() == 0) {

    test_mode = 1;
    max_brightness = .05;
    out_mode = 3;
    thresh_size = 0;
    set_low = 0;
    set_high = 1023;

    if (0) {
      for (int j = 1; j < num_leds; j++) {
        set_LED(j, .0, 0, 1);
        leds.show();
        delay(25);
      }
    }

  }

  TimerTc3.initialize(400);
  TimerTc3.attachInterrupt(timerIsr);



}


void loop() {
  cm = millis();

  bottom_buttom.update();
  top_buttom.update();

  if (set_low < 0) {
    set_low = 0;
  }
  if (set_high > 1023) {
    set_high = 1023;
  }

  if (bottom_buttom.fell()) {
    if (set_mode == CALIBRATING) {
      set_mode = IDLE;
      sm_mode = prev_sm_mode;
      smooth_amount = prev_smooth_amount ;
    }

    else {
      set_mode = CALIBRATING;
      set_high = 0;
      set_low = 1023;
      prev_sm_mode = sm_mode;
      prev_smooth_amount = smooth_amount;
    }

  }

  if (top_buttom.fell()) {
    test_mode = 0;

    if (setting_out_mode == 1) {
      out_mode++;
      if (out_mode > 3) {
        out_mode = 0;

      }
    }
    setting_out_mode = 1;

  }


  if (top_buttom.read() == 0) {
    adj_led = 1;
    setting_out_mode_timer = -1;

    setting_thresh_size = thresh_pot_change;
    setting_bright = smooth_pot_change;
  }

  if (top_buttom.rose()) {
    setting_out_mode_timer = cm;

    setting_thresh_size = 0;
    setting_bright = 0;
    //EEPROM.write(0, mb);
    if (1) {
      EEPROM.write(0, max_brightness * 255);
      EEPROM.write(1, out_mode);
      EEPROM.write(2, thresh_size >> 8);
      EEPROM.write(3, byte(thresh_size));
      EEPROM.commit();
    }

  }

  if (bottom_buttom.rose()) {
    //cal_time = -1;
    //setting_out_mode = 0;
    //amp_pot_change = 0;

    if (1) {
      EEPROM.write(4, set_low >> 8);
      EEPROM.write(5, byte(set_low));
      EEPROM.write(6, set_high >> 8);
      EEPROM.write(7, byte(set_high));
      EEPROM.commit();
    }

  }


  if (set_mode == CALIBRATING) {
    cal_color = .2;
    // setting_out_mode = amp_pot_change;
    if (smooth_s1 > set_high) {
      set_high = constrain(smooth_s1, 0, 1023);
    }
    if (smooth_s1 < set_low) {
      set_low = constrain(smooth_s1, 0, 1023);
    }
  }



  if (setting_bright == 1 && millis() > 2000) {
    adj_led = 1;
    setting_out_mode = 0;
    setting_thresh_size = 0;
    max_brightness = (pot_raw[1] / 1023.0);
  }

  if (setting_thresh_size == 1 && millis() > 2000) {
    adj_led = 1;
    setting_out_mode = 0;
    setting_bright = 0;
    thresh_size = pot_raw[0];
    if (pot_raw[0] < 20) {
      thresh_size = 0;
    }
  }

  if (top_buttom.read() == 1 &&
      setting_thresh_size == 0 &&
      setting_bright == 0 &&
      setting_out_mode == 0) {

    adj_led = 0;
  }




  if (setting_out_mode == 1) {
    adj_led = 1;
    if (setting_out_mode_timer > 0
        && cm - setting_out_mode_timer > 1000) {
      setting_out_mode = 0;
      setting_thresh_size = 0;
      setting_bright = 0;
    }

    for (int j = 0; j < num_leds; j++) {
      leds.setPixelColor(j, leds.Color(0, 0, 0));
      //leds.setPixelColor(j, leds.Color(random(127), random(127), random(127)));
    }
    byte areas = (36 / 4);
    set_LED(0, 0, 0, 1);
    set_LED(37, 0, 0, 1);
    set_LED(areas * 1, 0, 0, 1);
    set_LED(areas * 2, 0, 0, 1);
    set_LED(areas * 3, 0, 0, 1);


    if (out_mode == 0) {
      for (int j = areas * 0; j < areas * 1; j++) {
        set_LED(j, .4, 1, 1);
      }
    }
    if (out_mode == 1) {
      for (int j = areas * 1; j < areas * 2; j++) {
        set_LED(j, .5, 1, 1);
      }
    }
    if (out_mode == 2) {

      for (int j = areas * 2; j < areas * 3; j++) {
        set_LED(j, .6, 1, 1);
      }
    }
    if (out_mode == 3) {
      for (int j = areas * 3; j < areas * 4; j++) {
        set_LED(j, .7, 1, 1);
      }
    }

  }

  if (cm - pm > 5) {
    pm = cm;

    if (setting_out_mode == 0) {
      for (int j = 0; j < num_leds; j++) {
        leds.setPixelColor(j, leds.Color(0, 0, 0));
      }

      for (int j = trail_len - 1; j > 0; j--) {
        prev_pos[j] = prev_pos[j - 1];
        fade_level[j] = fade_level[j - 1];
      }

      ledout = map(real_fout, 0, 1023, 0, 37);
      if (led_lerp < ledout) {
        led_lerp++;
      }
      if (led_lerp > ledout) {
        led_lerp--;
      }
      prev_pos[0] = ledout;
      fade_level[1] = 1.0;

      if (thresh_size == 0) {
        int thresh_led = map(thresh, 0, 1023, 0, 37);
        set_LED(thresh_led, 0, 0, 1);
      }
      else {
        int thresh_start = map(thresh, 0, 1023, 0, 38);
        int thresh_end = map(thresh + thresh_size, 0, 1023, 0, 38);
        if (thresh_end <= thresh_start + 2) {
          thresh_end = thresh_start + 2;
        }
        for ( int th = thresh_start; th < thresh_end; th++) {
          set_LED(th, 0, 0, .5);
        }
      }

      if (sm_mode == 0) {
        g1 = 0 + (ledout / grad_amount);
      }
      if (sm_mode == 1) {
        g1 = .58 - (ledout / grad_amount);
      }
      if (sm_mode == 2) {
        g1 = .72 + (ledout / grad_amount);
      }

      for ( int trail = 0; trail < trail_len; trail++) {

        set_LED(prev_pos[trail], g1, 1, fade_level[trail]);

        if (trail > 0) {
          gap_size = prev_pos[trail - 1] - prev_pos[trail];
          if (gap_size >= 1) {
            for (byte f = 1; f < gap_size; f++) {
              int loc = prev_pos[trail] + f;
              set_LED(loc, g1, 1, fade_level[trail]);
            }
          }
          if (gap_size <= -1) {
            for (int g = abs(gap_size - 1); g > 1; g--) {
              int loc = prev_pos[trail] - g;
              set_LED(loc, g1, 1, fade_level[trail]);
            }
          }
        }


      }
      set_LED(prev_pos[0], g1 , 1, 1);

      for ( int i = 0; i < trail_len; i++) {
        fade_level[i] *= .96;
        if (fade_level[i] <= .01) { //an arbitray cuttoff since it will never quite get to 0 through multiplication
          fade_level[i] = 0;
        }
      }



    }

    set_LED(38, 0, 0, tout * 1);
    set_LED(39, 0, 0, !(tout * 1));

    if (out_mode == 3 && setting_out_mode == 0) {
      set_LED(41, 0, 0, 0);
    }
    if (setting_out_mode == 1) {
      set_LED(41, cal_color, 1, blinko);
    }
    if (out_mode !=3 && setting_out_mode == 0) {
      set_LED(41, 0, 0, 1);
    }
    set_LED(40, cal_color, 0, cal_led);
    set_LED(42, 0, 0, jack_switch);

    if (set_mode == CALIBRATING) {
      adj_led = 0;
      set_LED(40, cal_color, 1, blinko);
    }
    if (leds.canShow()) {
      leds.show();
    }
    blinko_count++;
    if (blinko_count > 6) {
      blinko_count = 0;
      blinko = !blinko;
    }
  }

  if (cm - pm2 > 20 && test_mode) {
    pm2 = cm;
    blink0 = !blink0;
    digitalWrite(13, blink0);

    printer();
  }

}


/////////////////////////////////////////////////////////////


void timerIsr()
{
  //digitalWrite(T1, 1);
  cu = micros();

  pot_tick++;
  if (pot_tick  > 2) {
    pot_tick = 0;
  }

  if (pot_tick == 2) {
    jack_a = analogRead(JACK_SWITCH_PIN);

    if (jack_a > 100) {
      if (flip_jack) {
        jack_switch = 0;
      }
      else {
        jack_switch = 1;
      }
    }
    else {
      if (flip_jack) {
        jack_switch = 1;
      }
      else {
        jack_switch = 0;
      }
    }

  }

  if (pot_tick == 0) {
    prev_pot_counter[0]++;
    if (prev_pot_counter[0] > ppc_len - 1) {
      prev_pot_counter[0] = 0;
    }

    prev_pot_raw[0][prev_pot_counter[0]] = pot_raw[0];
    byte gap = 6;
    pot_raw[0] = smooth(pot_tick, 27, analogRead(POT_TOP));
    int ppc2 = prev_pot_counter[0] + 1;
    if (ppc2 > ppc_len - 1) {
      ppc2 -= ppc_len - 1;
    }

    if (pot_raw[0] > prev_pot_raw[0][ppc2] + gap ||
        pot_raw[0] < prev_pot_raw[0][ppc2] - gap) {
      thresh_pot_change = 1;
      if (setting_thresh_size != 1) {
        thresh = pot_raw[0];
      }

    }
    else {
      if (top_buttom.read() == 1) {
        thresh_pot_change = 0;
      }
    }


  }

  if (pot_tick == 1) {

    prev_pot_counter[pot_tick]++;
    if (prev_pot_counter[pot_tick] > ppc_len - 1) {
      prev_pot_counter[pot_tick] = 0;
    }
    prev_pot_raw[pot_tick][prev_pot_counter[pot_tick]] = pot_raw[pot_tick];
    byte gap = 4;

    pot_raw[1] = smooth(pot_tick, 27, analogRead(POT_MIDDLE));
    int ppc2 = prev_pot_counter[pot_tick] + 1;
    if (ppc2 > ppc_len - 1) {
      ppc2 -= ppc_len - 1;
    }

    if (pot_raw[pot_tick] > prev_pot_raw[pot_tick][ppc2] + gap ||
        pot_raw[pot_tick] < prev_pot_raw[pot_tick][ppc2] - gap) {
      smooth_pot_change = 1;
      if (setting_bright != 1) {

        if (pot_raw[1] < sm_pot_center - sm_gap) {
          sm_mode = 1;
          smooth_amount = 20;
          follow_amount = (sm_pot_center - pot_raw[1] - sm_gap) / (sm_pot_center - sm_gap);
          float fa = .004;
          follow_amount = (follow_amount * fa) + (1.0 - fa);
        }

        else if (pot_raw[1] > sm_pot_center + sm_gap) {
          sm_mode = 2;
          smooth_amount = (pot_raw[1] - (sm_gap / 2) - sm_pot_center);
        }
        else {
          sm_mode = 0;
          smooth_amount = sm_low;
        }

        smooth_amount = constrain(smooth_amount, sm_low, 1023);
      }
    }
    else {
      if (top_buttom.read() == 1) {
        smooth_pot_change = 0;
      }
    }


    if (set_mode == CALIBRATING) {
      smooth_amount = sm_low;
      sm_mode = 0;
    }

  }



  ////////////////////////////////////

  if (jack_switch == 1) {
    int tr1 = qt_1.measure();
    int tr2 = qt_1.measure();
    tr = (tr1 + tr2) >> 1;
    raw_in = tr;
  }

  if (jack_switch == 0) {
    int ra = analogRead(SENSOR);
    int rb = analogRead(SENSOR);
    sr = (ra + rb) >> 1;
    raw_in = sr;
  }

  raw_in = constrain(raw_in, 0, 1023);



  if (sm_mode == 1) {
    smooth_amount_s1 = smooth_amount / 4;
    if (smooth_amount_s1 < 4) {
      smooth_amount_s1 = 4;
    }
  }
  if (sm_mode == 2) {
    smooth_amount_s1 = 16;
  }
  smooth_s1_c++;
  if (smooth_s1_c > smooth_amount_s1) {
    smooth_s1_c = 0;
  }
  smooth_bank_s1[smooth_s1_c] = raw_in;

  smooth_s1 = 0;
  for (uint16_t j = 0; j < smooth_amount_s1; j++) {
    smooth_s1 += smooth_bank_s1[j];
  }
  //fill the rest so it wont have weird old data
  for (uint16_t j = smooth_amount_s1; j < 200; j++) {
    smooth_bank_s1[j] = raw_in;
  }

  smooth_s1 = smooth_s1 / smooth_amount_s1;

  if  (sm_mode == 0) {
    smooth_s1 = raw_in;
  }

  if (set_mode != CALIBRATING) {
    cal_led = 0;
    set_mode_out = map(smooth_s1, set_low, set_high, 0, 1023);
    //fout = constrain(fout, 0, 1023);
  }

  if (set_mode == CALIBRATING) {
    adj_led = 0;
    cal_led = 1;
    set_mode_out = map(smooth_s1, set_low, set_high, 0, 1023);
    //fout = constrain(fout, 0, 1023);
  }


  smooth_count++;
  if (smooth_count > smooth_amount - 1) {
    smooth_count = 0;
  }
  smooth_bank[smooth_count] = set_mode_out;

  sm_out = 0;
  for (uint16_t j = 0; j < smooth_amount; j++) {
    sm_out += smooth_bank[j];
  }
  //fill the rest so it wont have weird old data
  for (uint16_t j = smooth_amount; j < 1000; j++) {
    smooth_bank[j] = set_mode_out;
  }

  sm_out = constrain(sm_out / smooth_amount, 0, 1023);


  if (follow_out < 1.0) {
    follow_out = 1.0;
  }
  if (follow_out > 1022) {
    follow_out = 1022;
  }

  prev_follow_out = follow_out;
  if (set_mode_out >= prev_follow_out) {
    follow_out = set_mode_out;
  }
  if (set_mode_out < prev_follow_out) {
    follow_out *= follow_amount;
  }

  if (sm_mode == 0) {
    final_sm_out = set_mode_out;
  }

  if (sm_mode == 1) {
    final_sm_out = follow_out;
  }
  if (sm_mode == 2) {
    final_sm_out = sm_out;
  }



  fout = constrain(final_sm_out, 0, 1023);

  if (out_mode == 0) {
    real_fout = 1023 - fout;
    led_fout = real_fout;
  }
  if (out_mode == 1) {
    real_fout = (1023 - fout) / 2;
    led_fout = 1023 - fout;
  }
  if (out_mode == 2) {
    real_fout = fout / 2;
    led_fout = fout;
  }
  if (out_mode == 3) {
    real_fout = fout;
    led_fout = fout;
  }

  if (test_mode == 1) {
    if (test_dir == 1) {
      test_out += 5;
    }
    if (test_dir == 0) {
      test_out -= 5;
    }
    if (test_out > 1023) {
      test_out = 1023;
      test_dir = 0;
    }
    if (test_out < 1 || test_out > 2000) {
      test_out = 0;
      test_dir = 1;
    }
    analogWrite(CV, test_out);
  }
  else {
    analogWrite(CV, real_fout);
  }
  prev_tout = tout;
  if (lock_out == 0) {
    if (thresh_size > 0) {
      if (fout < thresh) {
        tout = 0;
      }
      else if (fout > thresh + thresh_size) {
        tout = 0;
      }
      else
      {
        tout = 1;
      }
    }

    if (thresh_size == 0) {
      if (fout < thresh) {
        tout = 0;
      }
      else {
        tout = 1;
      }
    }

    if (prev_tout != tout) {
      lock_out = 1;
      lock_count = lock_out_period;
      ftout = tout;
    }
  }
  if (lock_out == 1) {
    lock_count--;
    if (lock_count <= 1) {
      lock_out = 0;
    }
  }

  if (0) {
    Serial.print(lock_out * 2);
    Serial.print(" ");
    Serial.println(ftout);
  }
  digitalWrite(T1, !ftout);
  digitalWrite(T2, ftout);
  //du = micros() - cu;
}




void set_LED(int pixel, float fh, float fs, float fv) {
  byte RedLight;
  byte GreenLight;
  byte BlueLight;

  byte h = fh * 255;
  byte s = fs * 255;
  byte v = fv * max_brightness * 255;

  h = (h * 192) / 256;  // 0..191
  unsigned int i = h / 32;   // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;   // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;  // 0 -> 0xff, 0xff -> 0
  unsigned int fInv = 255 - f;  // 0 -> 0xff, 0xff -> 0
  byte pv = v * sInv / 256;  // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      RedLight = v;
      GreenLight = tv;
      BlueLight = pv;
      break;
    case 1:
      RedLight = qv;
      GreenLight = v;
      BlueLight = pv;
      break;
    case 2:
      RedLight = pv;
      GreenLight = v;
      BlueLight = tv;
      break;
    case 3:
      RedLight = pv;
      GreenLight = qv;
      BlueLight = v;
      break;
    case 4:
      RedLight = tv;
      GreenLight = pv;
      BlueLight = v;
      break;
    case 5:
      RedLight = v;
      GreenLight = pv;
      BlueLight = qv;
      break;
  }
  leds.setPixelColor(pixel, RedLight, GreenLight, BlueLight);
}


#define maxarrays 4 //max number of different variables to smooth
#define maxsamples 37 //max number of points to sample and 
//reduce these numbers to save RAM

unsigned int smoothArray[maxarrays][maxsamples];

// sel should be a unique number for each occurrence
// samples should be an odd number greater that 7. It's the length of the array. The larger the more smooth but less responsive
// raw_in is the input. positive numbers in and out only.

unsigned int smooth(byte sel, unsigned int samples, unsigned int raw) {
  int j, k, temp, top, bottom;
  long total;
  static int i[maxarrays];
  static int sorted[maxarrays][maxsamples];
  boolean done;

  i[sel] = (i[sel] + 1) % samples;    // increment counter and roll over if necessary. -  % (modulo operator) rolls over variable
  smoothArray[sel][i[sel]] = raw;                 // input new data into the oldest slot

  for (j = 0; j < samples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[sel][j] = smoothArray[sel][j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (samples - 1); j++) {
      if (sorted[sel][j] > sorted[sel][j + 1]) {    // numbers are out of order - swap
        temp = sorted[sel][j + 1];
        sorted[sel] [j + 1] =  sorted[sel][j] ;
        sorted[sel] [j] = temp;
        done = 0;
      }
    }
  }

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((samples * 15)  / 100), 1);
  top = min((((samples * 85) / 100) + 1  ), (samples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j < top; j++) {
    total += sorted[sel][j];  // total remaining indices
    k++;
  }
  return total / k;    // divide by number of samples
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
