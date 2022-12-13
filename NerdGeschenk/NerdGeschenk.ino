//kompiliert mit ATtinyCore
#include <Ds1302.h>
#include <tinyNeoPixel.h>  //F_CPU muss >7.37Mhz sein

#define NP_PIN 0
#define BTN_PIN 2  //2.5v nominal, 5v mode, 0v set

//EN,CLK,DAT
Ds1302 rtc(1, 3, 4);
tinyNeoPixel leds = tinyNeoPixel(7, NP_PIN, NEO_GRB + NEO_KHZ800);

Ds1302::DateTime dt;  //sollte kostant pro loop sein und so oft genutzt das es keinunterschied macht

//color ist 0bIIRRGGBB
//I ist helligkeit aber im augenblick ignoriert
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
void led_wr_color_byte(uint32_t colors[7]) {
  for (uint8_t i = 0; i < 7; i++) {
    leds.setPixelColor(i, colors[i]);
  }
}

void setup() {
  leds.begin();
  leds.show();

  //rtc setup
  rtc.init();
  if (rtc.isHalted()) {
    for (uint8_t i = 0; i < 7; i++) {
      leds.setPixelColor(i, leds.Color((i + 1) * 36 /* 7->252 */, 0, 0));
    }
    while (true) delay(1000);
  }
  rtc.getDateTime(&dt);

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
        }
      }
      break;

    case Ds1302::MONTH_JUN:
      {  //pride month
        led_wr_color_byte({ leds.Color(255, 0, 0),
                            leds.Color(255, 128, 0),
                            leds.Color(255, 255, 0),
                            leds.Color(0, 255, 0),
                            leds.Color(0, 192, 255),
                            leds.Color(0, 0, 255),
                            leds.Color(192, 0, 255) });
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
          delay(100);
        }
        break;
      }
  }

  led_wr_byte(0, 0);
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
bool edit_mode = false;

auto last_disp_change = millis();
void display_page() {
  if (millis() > last_disp_change - 5000) {
    time_page++;
    if (time_page > PAGE_SEC) time_page = PAGE_YEAR;
  }

  bool edit_blink = edit_mode and (millis() % 1024 >= 512);
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

uint8_t button_val() {  //2 mode, 1 set, 0 keiner
  uint16_t adc_read = analogRead(BTN_PIN);
  if (adc_read < 256) return 2;
  else if (adc_read > 768) return 1;
  return 0;
}

void poll_buttons() {
}

void loop() {
  rtc.getDateTime(&dt);
  poll_buttons();
  display_page();
}
