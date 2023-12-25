#include <TimerOne.h>

const int pwmPin = 3;
const int pwmFrequency = 8000;  // 8 kHz
static int pwm = 0;
int initiation_pwm = 150;
const int AUX_INPUT_PIN = 8;

void pwm_func()
{
  analogWrite(pwmPin, pwm);
}

void setup() {
  Serial.begin(57600);
  pinMode(pwmPin, OUTPUT);

  // Configure Timer1 for PWM generation
  Timer1.initialize(1000000 / pwmFrequency); // Set PWM frequency
  pwm = initiation_pwm;
  // Timer1.attachInterrupt(pwm_func); // Set the ISR
  delay(1000);
  Serial.println("Starting...");
  pwm = 126;
  // Serial.println(pwm);
}

void loop()
{
  // You can adjust the throttle as needed in the loop
  // For example, gradually increase throttle from 0% to 100% in 10% increments
  // pwm = 150;
  // Serial.println("190");
  // delay(3000);
  // for (int i = 190; i >= 150; i--)
  // {
  //   pwm = i;
  //   Serial.println(i);
  //   delay(5);
  // }
  // delay(3000);
  // pwm = 120;
  // Serial.println("120");
  // delay(9000);
  int pulseWidth = pulseIn(AUX_INPUT_PIN, HIGH);

  // Calculate RPM based on the pulse width and the number of magnetic pole pairs
  int polePairs = 6;
  int rpm = 60 * 1000000 / (pulseWidth * polePairs);

  // Print the RPM to the Serial Monitor
  Serial.print("RPM: ");
  Serial.println(rpm);

  // Add a delay or other logic as needed
  delay(1000);
}
