const int AUX_INPUT_PIN = 8;

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read the pulse width on the AUX INPUT PIN
  int pulseWidth = pulseIn(AUX_INPUT_PIN, HIGH);

  // Calculate RPM based on the pulse width and the number of magnetic pole pairs
  // Replace 2 with your actual number of magnetic pole pairs
  int polePairs = 6;
  int rpm = 60 * 1000000 / (pulseWidth * polePairs);

  // Print the RPM to the Serial Monitor
  Serial.print("RPM: ");
  Serial.println(rpm);

  // Add a delay or other logic as needed
  delay(1000);
}