#include <Arduino.h>
#include "HX711.h"

//  any digital pins can be used
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;
const int PWM_OUTPUT_PIN = 3;
const int AUX_INPUT_PIN = 8;

HX711 scale;

int log_delay = 1000; // delay between logging in ms
bool lc_logging = true; // load cell logging on/off
float pwm = 10; // pwm represents the percentage of the duty cycle
bool pwm_update = true; // signal to update the pwm
bool m_on = true; // motor on/off
bool m_logging = true;  // motor logging on/off

void m_log()
{
  if (m_logging == true) {
    Serial.print("pwm=");
    Serial.println(pwm);
    char rpm_bytes[4];
    Serial.readBytesUntil('\n', rpm_bytes, 4);
    int rpm = atoi(rpm_bytes);
    Serial.print("rpm=");
    Serial.println(rpm);

    delay(log_delay);
  }
}

void m_signal()
{
  if (pwm_update == true) {
    if (m_on == true) {
      analogWrite(PWM_OUTPUT_PIN, pwm * 255 / 100);
      pwm_update = false;
    } else {
      analogWrite(PWM_OUTPUT_PIN, 0);
      pwm_update = false;
    }
  }
}

void lc_log()
{
  if (lc_logging == true) {
    Serial.print("lc_read=");
    Serial.println(scale.get_units(), 1);

    delay(log_delay);
  }
}

void setup()
{
  Serial.begin(57600);
  Serial.println("setup begin");

  pinMode(PWM_OUTPUT_PIN, OUTPUT);
  pinMode(AUX_INPUT_PIN, INPUT);

  analogWrite(PWM_OUTPUT_PIN, pwm * 255 / 100);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.print("precal_read=");
  Serial.println(scale.read());

//  calibration_factor = (reading)/(known weight)
//  float calibration_factor = -282642 / 11;
//  scale.set_scale(calibration_factor);
//  scale.tare();

  Serial.print("postcal_read=");
  Serial.println(scale.get_units(), 1);
  Serial.println("setup done");
}

void loop()
{
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    String var = data.substring(0, data.indexOf('='));
    String val = data.substring(data.indexOf('=') + 1);

    if (var.equals("log_delay")) {
      log_delay = val.toInt();
    } else if (var.equals("lc_logging")) {
      lc_logging = val.equals("true");
    } else if (var.equals("pwm")) {
      pwm = val.toFloat();
      pwm_update = true;
    } else if (var.equals("m_on")) {
      m_on = val.equals("true");
    } else if (var.equals("m_logging")) {
      m_logging = val.equals("true");
    }
  }

  m_signal();
  lc_log();
}
