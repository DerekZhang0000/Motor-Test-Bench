#include <Arduino.h>
#include "HX711.h"
#include "TimerOne.h"

HX711 scale1;
HX711 scale2;

const int LOADCELL1_DOUT_PIN = 12;
const int LOADCELL1_SCK_PIN = 13;
const int LOADCELL2_DOUT_PIN = 10;
const int LOADCELL2_SCK_PIN = 11;
const int PWM_OUTPUT_PIN = 3;
const int AUX_INPUT_PIN = 8;  // Used to get the RPM from ESC
const int PWM_FREQUENCY = 8000;  // ESC is set to 8 kHz

static int pwm = 0; // PWM value
static int pole_pairs = 0;  // Number of pole pairs in the motor
static unsigned long program_start_time = millis(); // Milliseconds at program start

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
  unsigned long function_start_time = millis();
  unsigned long timeout = 100;

  while (millis() - function_start_time < timeout) {
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
  int rpm = 60000000 / (pulse_width * pole_pairs);

  Serial.print("<rpm=");
  Serial.print(rpm);
  Serial.print(",");
  Serial.println(millis() - program_start_time);
}

void print_lc_readings()
{
  //  Prints both load cell readings to the Serial channel.
  Serial.print("<lc1=");
  Serial.print(scale1.get_units(), 4);
  Serial.print(",");
  Serial.println(millis() - program_start_time);
  Serial.print("<lc2=");
  Serial.print(scale2.get_units(), 4);
  Serial.print(",");
  Serial.println(millis() - program_start_time);
}

void pwm_timer_ISR()
{
  //  PWM interrupt service routine
  analogWrite(PWM_OUTPUT_PIN, pwm);
}

void calibrate_esc()
{
  //  Waits for the user to enter the initiation sequence PWM for the ESC.

  Serial.println("<info=Calibrating ESC (press Reset or load PWM program to arm)");
  while (pwm == 0 || pole_pairs == 0) {
    await_data(var, val);
    if (var == "pwm") {
      pwm = val.toInt();
      Serial.print("<info=Initiation PWM set to ");
      Serial.println(pwm);
    } else if (var == "pole_pairs") {
      pole_pairs = val.toInt();
      Serial.print("<info=Set # of pole pairs to ");
      Serial.println(pole_pairs);
    }
  } // Waits indefinitely for the initiation sequence PWM and pole pairs

  Timer1.attachInterrupt(pwm_timer_ISR);
  delay(1000);

  Serial.println("<info=Calibrating ESC complete");
}

void setup()
{
  Serial.begin(57600);
  delay(1000);
  Serial.println("<info=Setup starting");
  scale1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);
  scale2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);

  // Load Cell Calibration
  Serial.print("<info=Pre-calibration load cell readings: ");
  Serial.println(scale1.read(), 4);
  Serial.println(scale2.read(), 4);

  // float calibration_factor = -282642 / 11;  // calibration_factor = pre-calibration load cell reading / known weight
  // scale1.set_scale(calibration_factor); // ADD MORE POINTS
  // scale1.tare();
  // scale2.set_scale(calibration_factor);
  // scale2.tare();

  Serial.print("<info=Post-calibration load cell reading: ");
  Serial.println(scale1.get_units(), 4);
  Serial.println(scale2.get_units(), 4);

  // ESC Setup
  pinMode(PWM_OUTPUT_PIN, OUTPUT);
  Timer1.initialize(1000000 / PWM_FREQUENCY);
  calibrate_esc();

  program_start_time = millis();
  Serial.println("<info=Setup complete");
  Serial.print("<log_start=0,");
  Serial.println(millis() - program_start_time);
}

void loop()
{
  if (await_data(var, val)) {
    if (var == "log_start") {
      Serial.print("<log_start=0,");
      Serial.println(millis() - program_start_time);
    } else if (var == "log_stop") {
      Serial.print("<log_stop=0,");
      Serial.println(millis() - program_start_time);
    } else if (var == "pwm") {
      pwm = val.toInt();
      Serial.print("<info=Set pwm to ");
      Serial.println(pwm);
    } else if (var == "pole_pairs") {
      pole_pairs = val.toInt();
      Serial.print("<info=Set # of pole pairs to ");
      Serial.println(pole_pairs);
    }
  }
  print_lc_readings();
  // print_rpm_reading();  // ESC already records RPM
}
