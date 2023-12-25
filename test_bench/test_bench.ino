#include <Arduino.h>
#include "HX711.h"
#include "TimerOne.h"

HX711 scale;

const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;
const int PWM_OUTPUT_PIN = 3;
const int AUX_INPUT_PIN = 8;  // Used to get the RPM from ESC

const int PWM_FREQUENCY = 8000;  // ESC is set to 8 kHz

static int pwm = 0; // PWM value

static String data = "";
static String var = "";
static String val = "";

bool await_data(String &var, String &val)
{
  /*
    Waits for and processes data received from the serial port.
    Splits the data into a variable and a value using the '=' character.
    Idles until it receives the terminating character '\n' or until the timeout duration is reached.
    Return true if data is received, false if the function timed out.
    Ignore outgoing data, marked by a '<'.
  */
  unsigned long start_time = millis();
  unsigned long timeout = 300;

  while (millis() - start_time < timeout) {
    data = Serial.readStringUntil('\n');
    if (!data.startsWith("<")) {  // Ignores outgoing data
      var = data.substring(0, data.indexOf('='));
      val = data.substring(data.indexOf('=') + 1);
      data = "";
      return true;
    }
  }
  return false;
}

void print_rpm_reading()
{
  //  Prints the RPM reading to the Serial channel.
  int pulse_width = pulseIn(AUX_INPUT_PIN, HIGH);
  int pole_pairs = 6;
  int rpm = 60 * 1000000 / (pulse_width * pole_pairs);

  Serial.print("<rpm=");
  Serial.println(rpm);
}

void print_lc_reading()
{
  //  Prints the load cell reading to the Serial channel.
  Serial.print("<lc=");
  Serial.println(scale.get_units(), 1);
}

void pwm_timer_ISR()
{
  //  PWM interrupt service routine
  analogWrite(PWM_OUTPUT_PIN, pwm);
}

void calibrate_esc()
{
  /*
    Waits for the user to enter the initiation sequence PWM for the ESC.
  */

  Serial.println("info=Calibrating ESC");
  while (var != "initiation_pwm") { await_data(var, val); } // Waits indefinitely for the initiation sequence PWM
  pwm = val.toInt();
  Timer1.attachInterrupt(pwm_timer_ISR);
  Serial.print("info=Initiation PWM set to ");
  Serial.println(pwm);
  delay(1000);

  Serial.println("info=Calibrating ESC complete");
}

void setup()
{
  Serial.begin(57600);
  delay(1000);
  Serial.println("info=Setup starting");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Load Cell Calibration
  Serial.print("info=Pre-calibration load cell reading: ");
  Serial.println(scale.read());

  // float calibration_factor = -282642 / 11;  // calibration_factor = pre-calibration load cell reading / known weight
  // scale.set_scale(calibration_factor);
  // scale.tare();

  Serial.print("info=Post-calibration load cell reading: ");
  Serial.println(scale.get_units(), 1);

  // ESC Setup
  pinMode(PWM_OUTPUT_PIN, OUTPUT);
  Timer1.initialize(1000000 / PWM_FREQUENCY);
  calibrate_esc();

  Serial.println("info=Setup complete");
}

void loop()
{
  if (await_data(var, val)) {
    if (var == "pwm") {
      pwm = val.toInt();
      Serial.print("info=Set pwm to ");
      Serial.println(pwm);
    }
  }
  print_lc_reading();
  print_rpm_reading();
}
