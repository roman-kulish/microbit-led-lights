#include <Arduino.h>
#include <Adafruit_Microbit.h>
#include <Wire.h>
#include "SparkFun_VEML6030_Ambient_Light_Sensor.h"
#include "VEML6030.h"
#include "Button.h"
#include "State.h"
#include <WS2812FX.h>

Adafruit_Microbit uBit{};

SparkFun_Ambient_Light lightSensor{0x10};
Sensor::VEML6030 light{};

WS2812FX ws2812fx{120, PIN_A0, NEO_GRB};

Button buttonA{PIN_BUTTON_A};
Button buttonB{PIN_BUTTON_B};

State state{};

static constexpr uint8_t IMAGE_MODE_ON[5] = {
    B00000,
    B01010,
    B00000,
    B10001,
    B01110};

static constexpr uint8_t IMAGE_MODE_OFF[5] = {
    B00000,
    B01010,
    B00000,
    B01110,
    B10001};

void onStateChange(bool isOn)
{
  if (isOn)
  {
    uBit.matrix.show(IMAGE_MODE_ON);
    ws2812fx.start();

    return;
  }

  uBit.matrix.show(IMAGE_MODE_OFF);
  ws2812fx.stop();
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  Wire.begin();

  if (!light.begin(&lightSensor))
  {
    Serial.println("Ambient light sensor not found. Halting ...");

    while (true)
      ; // Hang here
  }

  lightSensor.setGain(2);
  lightSensor.setIntegTime(400);
  lightSensor.disablePowSave();

  uBit.begin();

  ws2812fx.init();
  ws2812fx.setMode(state.getPattern());

  uint32_t colors[] = {BLUE, YELLOW, GREEN};
  ws2812fx.setColors(0, colors);

  state.OnStateChange(onStateChange);

  state.OnPatternChange([](uint8_t pattern)
                        {
                           ws2812fx.setMode(pattern); 

                            Serial.print("Pattern changed to: ");
                            Serial.println(ws2812fx.getModeName(pattern)); });

  buttonA.onPress([]()
                  { state.toggle(); });

  buttonB.onPress([]()
                  { state.nextPattern(); });

  state.setState(false);
}

void loop()
{
  buttonA.processButton();
  buttonB.processButton();

  ws2812fx.setBrightness(light.getBrightness());
  ws2812fx.service();
}
