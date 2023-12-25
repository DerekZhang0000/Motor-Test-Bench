void setup() {
  Serial.begin(57600);
  Serial.println("Starting...");
}

static String data = "";
static String var = "";
static String val = "";

bool await_data(String &var, String &val) {
  /*
  Waits for and processes data received from the serial port.
  Splits the data into a variable and a value using the '=' character.
  Idles until it receives the terminating character '\n'.
  Return true if data is received.
  */

  while (true) {
    while (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n') {
        if (data.indexOf('=') == -1) {
          Serial.print("err=Invalid data line (");
          Serial.print(data);
          Serial.println(")");
          data = "";
          continue;
        }
        var = data.substring(0, data.indexOf('='));
        val = data.substring(data.indexOf('=') + 1);
        return true;
      } else {
        data += c;
      }
    }
  }
}

void loop() {
  if (await_data(var, val)) {
    Serial.print("data=");
    Serial.print(data);
    Serial.print(" var=");
    Serial.print(var);
    Serial.print(" val=");
    Serial.println(val);
    data = "";
  }
  Serial.println("loop");
}
