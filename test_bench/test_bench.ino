#include <Arduino.h>
#include "HX711.h"
// #include "Servo.h"
#include "TimerOne.h"

//  any digital pin can be used
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;
const int PWM_OUTPUT_PIN = 3;
const int AUX_INPUT_PIN = 8;

HX711 scale;
// Servo esc;

int log_delay = 1000; // delay between logging in ms
bool lc_logging = true; // load cell logging on/off
float pwm_pct = 10; // pwm_pct represents the percentage of the duty cycle
bool pwm_update = true; // signal to update the pwm
bool m_on = true; // motor on/off
bool m_logging = true;  // motor logging on/off

void m_log()
{
  if (m_logging == true) {
    Serial.print("pwm=");
    Serial.println(pwm_pct);
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
      analogWrite(PWM_OUTPUT_PIN, 100);
      Serial.print("set pwm_pct to ");
      Serial.println(100);
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
  Timer1.initialize(12000);
  Timer1.pwm(PWM_OUTPUT_PIN, 20);

  // pinMode(PWM_OUTPUT_PIN, OUTPUT);
  // esc.attach(PWM_OUTPUT_PIN);
  pinMode(AUX_INPUT_PIN, INPUT);

  // analogWrite(PWM_OUTPUT_PIN, pwm_pct * 255);
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
  // if (Serial.available() > 0) {
  //   String data = Serial.readStringUntil('\n');
  //   String var = data.substring(0, data.indexOf('='));
  //   String val = data.substring(data.indexOf('=') + 1);

  //   if (var.equals("log_delay")) {
  //     log_delay = val.toInt();
  //   } else if (var.equals("lc_logging")) {
  //     lc_logging = val.equals("true");
  //   } else if (var.equals("pwm_pct")) {
  //     pwm_pct = val.toFloat();
  //     pwm_update = true;
  //   } else if (var.equals("m_on")) {
  //     m_on = val.equals("true");
  //   } else if (var.equals("m_logging")) {
  //     m_logging = val.equals("true");
  //   }
  // }

    // analogWrite(PWM_OUTPUT_PIN, 200);
    // int esc_val = map(10, 0, 100, 8000, 9000);
    // esc.writeMicroseconds(esc_val);

  // m_signal();
  // lc_log();
}
