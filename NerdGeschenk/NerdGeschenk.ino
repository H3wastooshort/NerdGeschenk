//kompiliert mit ATtinyCore
#include <Ds1302.h>
#include <tinyNeoPixel.h>  //F_CPU muss >7.37Mhz sein

#define NP_PIN 0           //neopixel pin
#define BTN_PIN 2          //analog. 2.5v nominal, 5v mode, 0v set
#define DEBOUNCE_DELAY 50  //zeit in millisekunden fuer die ein weiterer knopfdruck ignoriert wird

//EN,CLK,DAT
Ds1302 rtc(1, 3, 4);
tinyNeoPixel leds = tinyNeoPixel(7, NP_PIN, NEO_GRB + NEO_KHZ800);

Ds1302::DateTime dt;  //sollte kostant pro loop sein und so oft genutzt das es kein unterschied macht

void led_wr_byte(byte led_byte, uint32_t color) {
  for (uint8_t i = 0; i < 7; i++) {
    if ((led_byte >> i) && 0x01) {
      leds.setPixelColor(i, color);
    } else {
      leds.setPixelColor(i, 0);
    }
  }
  leds.show();
}

//ganzes byte in farbe schreiben
void led_wr_color_byte(/*std::initializer_list<uint32_t>&*/ uint32_t* colors) {
  for (uint8_t i = 0; i < 7; i++) {
    leds.setPixelColor(i, colors[i]);
  }
  leds.show();
}

void led_clear() {
  for (uint8_t i = 0; i < 7; i++) leds.setPixelColor(i, 0);
}

uint32_t seasonal_color(uint8_t& month) {
  return 0xFFFFFFFF;
}

void led_run_daily_anim() {
  //boot animation
  switch (dt.month) {
    case Ds1302::MONTH_DEC:
      {
        if (dt.day <= 25) {  //weihnachten
          const char* xmas = "Merry XMAS!";
          for (uint8_t i = 0; i < strlen(xmas); i++) {
            led_wr_byte(xmas[i], leds.Color(255, 255, 255));
            delay(1000);
          }
        } else if (dt.day == 31) {  //silvester
          for (uint8_t k = 0; k <= 3; k++) {
            for (uint8_t j = 0; j < 7; j++) {  //rakete einfliegen
              for (uint8_t i = 0; i <= j; i++) {
                uint8_t brght = map(i, 0, 6, 0, 255);
                leds.setPixelColor(i, (k == 0) * brght, (k == 1) * brght, (k == 2) * brght);
                leds.show();
                delay(50);
              }
            }
            for (uint8_t b = 1; b <= 4; b++) {
              for (uint8_t j = 0; j < 7; j++) {  //rakete explosion
                for (int8_t i = j; i >= 0; i--) {
                  uint8_t brght = map(i, 0, 6, 0, b * 64);
                  leds.setPixelColor(i, brght, brght, brght);
                  leds.show();
                  delay(10);
                }
              }
            }
            led_clear();
          }
        }
      }
      break;

    case Ds1302::MONTH_JUN:
      {  //pride month
        uint32_t pride_flag[7] = { leds.Color(255, 0, 0),
                                   leds.Color(255, 128, 0),
                                   leds.Color(255, 255, 0),
                                   leds.Color(0, 255, 0),
                                   leds.Color(0, 192, 255),
                                   leds.Color(0, 0, 255),
                                   leds.Color(192, 0, 255) };
        led_wr_color_byte(pride_flag);  //keine ordentliche STL, keine einfache moeglichkeit das direkt als initializer list zu passen
        leds.show();
        delay(3000);
      }

    case Ds1302::MONTH_MAR:
      {
        if (dt.day == 14) {  //pi day
          byte pi_b[] = { 3, 255, 1, 4 };
          for (uint8_t i = 0; i < sizeof(pi_b); i++) {
            led_wr_byte(pi_b[i], leds.Color(255, 255, 255));
            delay(1000);
          }
        }
      }

    default:
      {
        for (uint8_t i = 0; i <= 4; i++) {
          leds.setPixelColor(i, seasonal_color(dt.month));
          leds.setPixelColor(6 - i, seasonal_color(dt.month));
          leds.show();
          delay(100);
        }
        break;
      }
  }
  led_wr_byte(0, 0);
}

void setup() {
  leds.begin();
  leds.show();

  //rtc setup
  rtc.init();
  if (rtc.isHalted()) {
    for (uint8_t i = 0; i < 7; i++) {
      leds.setPixelColor(i, (i + 1) * 36 /* 7->252 */, 0, 0);
    }
    leds.show();
    dt.year = 22;
    dt.month = 1;
    dt.dow = 7;
    dt.day = 1;
    dt.hour = 0;
    dt.minute = 0;
    dt.second = 0;
    rtc.setDateTime(&dt);
    delay(1000);
  }
  rtc.getDateTime(&dt);

  led_run_daily_anim();
}

enum time_page_e {
  PAGE_YEAR = 0,  //rot
  PAGE_MON,       //gruen
  PAGE_DAY,       //blau
  PAGE_HOUR,      //magenta
  PAGE_MIN,       //cyan
  PAGE_SEC        //gelb
};
uint8_t time_page = PAGE_YEAR;
uint8_t edit_mode = 0;  //0 keine bearbeitung, 1 hinzufuegen, 2 abziehen

auto last_disp_change = millis();
void display_page() {
  if (millis() > last_disp_change - 5000) {
    if (edit_mode == 0) time_page++;
    if (time_page > PAGE_SEC) time_page = PAGE_YEAR;
  }

  bool edit_blink = (edit_mode > 0) and (millis() % 1024 >= 512);
  switch (time_page) {
    case PAGE_YEAR:  //rot
      led_wr_byte(dt.year, edit_blink ? 0xFFFFFFFF : leds.Color(255, 0, 0));
      break;
    case PAGE_MON:  //gruen
      led_wr_byte(dt.month, edit_blink ? 0xFFFFFFFF : leds.Color(0, 255, 0));
      break;
    case PAGE_DAY:  //blau
      led_wr_byte(dt.day, edit_blink ? 0xFFFFFFFF : leds.Color(0, 0, 255));
      break;
    case PAGE_HOUR:  //magenta
      led_wr_byte(dt.hour, edit_blink ? 0xFFFFFFFF : leds.Color(255, 0, 255));
      break;
    case PAGE_MIN:  //cyan
      led_wr_byte(dt.minute, edit_blink ? 0xFFFFFFFF : leds.Color(0, 255, 255));
      break;
    case PAGE_SEC:  //gelb
      led_wr_byte(dt.second, edit_blink ? 0xFFFFFFFF : leds.Color(255, 255, 0));
      break;
  }
  leds.show();
}

uint8_t read_button_val() {  //2 mode, 1 set, 0 keiner
  uint16_t adc_read = analogRead(BTN_PIN);
  if (adc_read < 256) return 2;       //spannung nahe 0V
  else if (adc_read > 768) return 1;  //spannung nahe 5V
  return 0;
}

uint8_t last_button_val = 0;
auto last_button_millis = millis();
void poll_buttons() {
  uint8_t button_val = read_button_val();

  if ((button_val != last_button_val) and (millis() - last_button_millis > DEBOUNCE_DELAY)) {
    if (button_val != 0) last_button_millis = millis();

    //TODO: button reagiert auf loslassen, lange halten setzt feld zurueck

    switch (last_button_val) {
      case 1:
        if (edit_mode > 0) {
          switch (time_page) {  //kein <0 test noetig, unter 0 wird 255 was bei allen zu hoch ist
            case PAGE_YEAR:
              dt.year += (edit_mode == 1) ? 1 : -1;
              break;
            case PAGE_MON:
              dt.month += (edit_mode == 1) ? 1 : -1;
              if (dt.month > 12) dt.month = 0;
              break;
            case PAGE_DAY:
              dt.day += (edit_mode == 1) ? 1 : -1;
              dt.dow += (edit_mode == 1) ? 1 : -1;
              dt.dow %= 7;
              if (dt.day > 31) dt.day = 0;
              break;
            case PAGE_HOUR:
              dt.hour += (edit_mode == 1) ? 1 : -1;
              if (dt.hour > 24) dt.hour = 0;
              break;
            case PAGE_MIN:
              dt.minute += (edit_mode == 1) ? 1 : -1;
              if (dt.minute > 60) dt.minute = 0;
              break;
            case PAGE_SEC:
              dt.second += (edit_mode == 1) ? 10 : -10;
              if (dt.second > 60) dt.second = 0;
              break;
          }
          rtc.setDateTime(&dt);

        } else {
          uint8_t led_bright = leds.getBrightness();
          led_bright += 32;
          leds.setBrightness(led_bright);
        }
        break;

      case 2:
        switch (edit_mode) {
          default:
          case 0:  //edit start
            edit_mode = 1;
            time_page = 0;
            break;

          case 1:  //edit wechsel auf abzug
            edit_mode = 2;

            //led abzug modus
            led_clear();
            leds.setPixelColor(3, 0, 255, 0);
            delay(250);
            break;

          case 2:  //edit wechsel auf hinzu
            time_page++;
            edit_mode = 1;

            if (time_page > PAGE_SEC) {  //edit verlassen wenn fertig
              edit_mode = 0;
              time_page = PAGE_YEAR;

              //led anzeige edit verlassen
              led_clear();
              leds.setPixelColor(3, 0, 0, 255);
              delay(250);
              break;
            }

            //led hinzuf. modus
            led_clear();
            leds.setPixelColor(3, 255, 0, 0);
            delay(250);
            break;
        }

        break;
    }
    last_button_val = button_val;
  }
}

void loop() {
  rtc.getDateTime(&dt);
  poll_buttons();
  display_page();
}
