#define LGFX_USE_V1

#include <Arduino.h>
#include <LovyanGFX.hpp>

#define screenWidth 480
#define screenHeight 320

unsigned long lastUpdate = 0;
const long updateInterval = 66;

// Configuration for ESP32-S3 WT32-SC01 Plus 3.5" 480x320 / 16MB Flash / 2MB PSRAM
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7796 _panel_instance;
  lgfx::Bus_Parallel8 _bus_instance;  // 8-bit parallel bus instance (ESP32 only)
  lgfx::Light_PWM _light_instance;

public:
  LGFX(void) {
    // Bus settings
    {
      auto cfg = _bus_instance.config();
      // 8-bit parallel bus settings
      // cfg.i2s_port = I2S_NUM_0;    // Select the I2S port to use (I2S_NUM_0 or I2S_NUM_1) (use ESP32's I2S LCD mode)
      cfg.freq_write = 20000000;               // Transmit clock (up to 20MHz, rounded to the value obtained by dividing 80MHz by an integer)
      cfg.pin_wr = 47;                         // WR
      cfg.pin_rd = -1;                         // RD
      cfg.pin_rs = 0;                          // RS(D/C)
      cfg.pin_d0 = 9;                          // D0
      cfg.pin_d1 = 46;                         // D1
      cfg.pin_d2 = 3;                          // D2
      cfg.pin_d3 = 8;                          // D3
      cfg.pin_d4 = 18;                         // D4
      cfg.pin_d5 = 17;                         // D5
      cfg.pin_d6 = 16;                         // D6
      cfg.pin_d7 = 15;                         // D7
      _bus_instance.config(cfg);               // Applies the set value to the bus
      _panel_instance.setBus(&_bus_instance);  // Set the bus on the panel
    }

    // Display panel settings
    {
      auto cfg = _panel_instance.config();  // Get structure for display panel settings.

      cfg.pin_cs = -1;    // CS
      cfg.pin_rst = 4;    // RST
      cfg.pin_busy = -1;  // BUSY

      // The following setting values â€‹â€‹are general initial values â€‹â€‹for each panel
      cfg.panel_width = screenHeight;  // Actual displayable width - Rotated and swapped to keep USB left
      cfg.panel_height = screenWidth;  // Actual visible height - Rotated and swapped to keep USB left
      cfg.offset_x = 0;                // Panel offset amount in X direction
      cfg.offset_y = 0;                // Panel offset amount in Y direction
      cfg.offset_rotation = 90;        // Rotation direction value offset 0~7 (4~7 is upside down)
      cfg.dummy_read_pixel = 8;        // Number of bits for dummy read before pixel readout
      cfg.dummy_read_bits = 1;         // Number of bits for dummy read before non-pixel data read
      cfg.readable = true;             // Set to true if data can be read
      cfg.invert = true;               // Set to true if the light/darkness of the panel is reversed
      cfg.rgb_order = false;           // Set to true if the panel's red and blue are swapped
      cfg.dlen_16bit = false;          // Set to true for panels that transmit data length in 16-bit units
      cfg.bus_shared = true;           // If the bus is shared with the SD card, set to True

      // Set the following only when the display is shifted with a driver with a variable number
      // of pixels, such as the ST7735 or ILI9163.
      //    cfg.memory_width     =   240;  // Maximum width supported by the driver IC
      //    cfg.memory_height    =   320;  // Maximum height supported by the driver IC

      _panel_instance.config(cfg);
    }

    // Backlight settings
    {
      auto cfg = _light_instance.config();  // Get structure for backlight settings

      cfg.pin_bl = 45;      // Pin number to which the backlight is connected
      cfg.invert = false;   // True to invert the brightness of the backlight
      cfg.freq = 44100;     // PWM frequency of backlight
      cfg.pwm_channel = 7;  // PWM channel number to use

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  // Set the backlight on display
    }

    setPanel(&_panel_instance);  // Set the panel to use
  }
};

class FPSCounter {
protected:
  unsigned int m_fps;
  unsigned int m_fpscount;
  unsigned long m_lastTime;

public:
  // Constructor
  FPSCounter()
    : m_fps(0), m_fpscount(0), m_lastTime(0) {
  }

  // Update
  void update() {
    // increase the counter by one
    m_fpscount++;

    // Check if one second has elapsed
    unsigned long currentTime = millis();
    if (currentTime - m_lastTime > 1000) {
      // save the current counter value to m_fps
      m_fps = m_fpscount;

      // reset the counter
      m_fpscount = 0;

      // update the last time
      m_lastTime = currentTime;
    }
  }

  // Get fps
  unsigned int get() const {
    return m_fps;
  }
};

LGFX tft;
LGFX_Sprite tftBuffer(&tft);
FPSCounter fps;

String formatBytes(uint64_t bytes) {
  String formattedSize;
  if (bytes < 1024) {
    formattedSize = String(bytes) + " B";
  } else if (bytes < 1024 * 1024) {
    formattedSize = String(bytes / 1024.0, 2) + " KB";
  } else if (bytes < 1024 * 1024 * 1024) {
    formattedSize = String(bytes / 1024.0 / 1024.0, 2) + " MB";
  } else {
    formattedSize = String(bytes / 1024.0 / 1024.0 / 1024.0, 2) + " GB";
  }
  return formattedSize;
}

String formatWithCommas(uint32_t value) {
  String result;

  for (int i = 0; value != 0; i++, value /= 10) {
    result = char(value % 10 + '0') + result;
    if (i % 3 == 2 && value / 10 != 0) result = ',' + result;
  }

  return result;
}

void animateSphere(LGFX_Sprite &tftBuffer, int radius, unsigned long currentMillis) {
  static int currentX = radius + 15;  // Current X position of the sphere
  static bool movingRight = true;     // Direction of movement

  // Calculate the position based on elapsed time
  int speed = 3;                         // Adjust the speed as needed
  int maxX = screenWidth - radius - 15;  // Maximum X position

  // Determine the new position
  if (movingRight) {
    currentX += speed;
    if (currentX >= maxX) {
      movingRight = false;
    }
  } else {
    currentX -= speed;
    if (currentX <= radius) {
      movingRight = true;
    }
  }

  // Draw the sphere at the new position
  tftBuffer.fillCircle(currentX, screenHeight - radius - 15, radius, TFT_WHITE);
}

void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.setBrightness(255);

  tftBuffer.createSprite(screenWidth, screenHeight);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate >= updateInterval) {
    lastUpdate = currentMillis;

    tftBuffer.clearClipRect();
    tftBuffer.fillScreen(TFT_BLACK);

    // Header
    tftBuffer.drawRect(3, 3, 476, 30, TFT_WHITE);
    tftBuffer.drawRect(4, 3, 477, 30, TFT_WHITE);
    tftBuffer.drawString(String(fps.get()), 8, 8);
    tftBuffer.drawString("FPS Test - WT32-SC01 Plus Edition", 140, 14);

    // Body
    tftBuffer.drawRect(3, 34, 477, 130, TFT_WHITE);

    // Left column
    tftBuffer.drawRect(4, 32, 238, 130, TFT_WHITE);
    tftBuffer.drawString("Chip Model:       " + String(ESP.getChipModel()) + " Rev " + String(ESP.getChipRevision()), 16, 46);
    tftBuffer.drawString("CPU Frequency:    " + String(ESP.getCpuFreqMHz()) + " MHz", 16, 62);
    tftBuffer.drawString("Cores:            " + String(ESP.getChipCores()), 16, 78);
    //tftBuffer.drawString("CPU Cycle Count:  " + String(ESP.getCycleCount()), 16, 94);
    uint32_t cycleCount = ESP.getCycleCount();
    String formattedCycleCount = formatWithCommas(cycleCount);
    tftBuffer.drawString("CPU Cycle Count:  " + formattedCycleCount, 16, 94);

    tftBuffer.drawString("SDK Version:      " + String(ESP.getSdkVersion()), 16, 110);
    tftBuffer.drawString("Flash Chip Size:  " + formatBytes(ESP.getFlashChipSize()), 16, 126);

    // Right column
    tftBuffer.drawRect(242, 32, 236, 130, TFT_WHITE);
    tftBuffer.drawString("Heap Size:        " + formatBytes(ESP.getHeapSize()), 254, 46);
    tftBuffer.drawString("Free Heap:        " + formatBytes(ESP.getFreeHeap()), 254, 62);
    tftBuffer.drawString("Min Free Heap:    " + formatBytes(ESP.getMinFreeHeap()), 254, 78);
    tftBuffer.drawString("PSRAM Size:       " + formatBytes(ESP.getPsramSize()), 254, 94);
    tftBuffer.drawString("Free PSRAM:       " + formatBytes(ESP.getFreePsram()), 254, 110);
    tftBuffer.drawString("Min Free PSRAM:   " + formatBytes(ESP.getMinFreePsram()), 254, 126);

    animateSphere(tftBuffer, 50, millis());
  }

  tftBuffer.pushSprite(0, 0);

  fps.update();
}