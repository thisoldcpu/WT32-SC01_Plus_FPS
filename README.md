![WT32-SC01_Plus_FPS_thumbnail](https://github.com/thisoldcpu/WT32-SC01_Plus_FPS/assets/47921016/e9a3219f-8cad-404b-a53b-107eb6359979)

```
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
```

```
FPSCounter fps;
```

```
void loop() {
  tftBuffer.drawString(String(fps.get()), 8, 8);
  fps.update();
}
```
https://youtu.be/


[![IMAGE ALT TEXT](http://img.youtube.com/vi/5LCwOt_R0Yw/0.jpg)](http://www.youtube.com/watch?v=5LCwOt_R0Yw "FPS counter for ESP32 based microcontrollers")
