int lightLevel = 0;
/* Pin allocation
light sensor - A0
dht11 - D0
powerPin - D6
speakerPin - D4
*/
int dht = D0;
int speakerPin = D4;
int relativeHumidity = 0;
int temp = 0;

// Threshold variables to be handled and tweaked remotely
int lightThreshold = 200;

// Count raw loops
int ticker = 0;

// Servo motor configuration
int servoPos = 90;
Servo myServo;

void setup() {
  // Setup power pin for sustained delivery
  pinMode(D6, OUTPUT);
  digitalWrite(D6, HIGH);
  pinMode(D1, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(speakerPin, OUTPUT);
  Spark.variable("rh", &relativeHumidity, INT);
  Spark.variable("temp", &temp, INT);
  Spark.variable("lightLevel", &lightLevel, INT);
  Spark.variable("servoPos", &servoPos, INT);
  // Sync thresholds
  Spark.variable("lightThreshold", &lightThreshold, INT);
  pinMode(dht, INPUT_PULLUP);
  // Setup servo motor
  myServo.attach(A5);
  myServo.write(servoPos);
}

void loop() {
  ticker += 1;
  // Handle light
  lightLevel = analogRead(A0);
  if (lightLevel < lightThreshold) {
    digitalWrite(D1, HIGH);
  } else {
    digitalWrite(D1, LOW);
  }
  // Handle temp and humidity
  read_dht(dht, &relativeHumidity, &temp);
  // Handle servo
  handle_servo();
  // Sustain power to power pin
  digitalWrite(D6, HIGH);
  // Pause for calibration
  delay(200);
}

// Control functions
void handle_servo() {
  int choice = 1;
  if (choice == 0) { 
    // Light level to position program
    int temp = servoPos;
    servoPos = (lightLevel / (10)) % 140;
    int diff = servoPos - temp;
    // Smooth movements
    if (diff > 20) {
      servoPos = temp + 20;
    } else if (diff < - 20) {
      servoPos = temp - 20;
    }
  } else if (choice == 1) {
    // Time tick program
    int steps = 10;
    int rawPosition = ticker / steps;
    // Curve and cap range from 10 to 150 degrees
    servoPos = rawPosition % 140 + 10;
  }
  myServo.write(servoPos);
}

// DHT11 Humidity and Temperature sensor reading
int read_dht(int pin, int *humidity, int *temperature)
{
    uint8_t data[5] = {0, 0, 0, 0, 0};

    noInterrupts();
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(20);
    pinMode(pin, INPUT_PULLUP);
    if (detect_edge(pin, HIGH, 10, 200) == -1) {
        return -1;
    }
    if (detect_edge(pin, LOW, 10, 200) == -1) {
        return -1;
    }
    if (detect_edge(pin, HIGH, 10, 200) == -1) {
        return -1;
    }
    for (uint8_t i = 0; i < 40; i++) {
        if (detect_edge(pin, LOW, 10, 200) == -1) {
            return -1;
        }
        int counter = detect_edge(pin, HIGH, 10, 200);
        if (counter == -1) {
            return -1;
        }
        data[i/8] <<= 1;
        if (counter > 4) {
            data[i/8] |= 1;
        }
    }
    interrupts();

    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        return -1;
    }

    *humidity = data[0];
    *temperature = data[2];
    return 0;
}

int detect_edge(int pin, int val, int interval, int timeout)
{
    int counter = 0;
    while (digitalRead(pin) == val && counter < timeout) {
        delayMicroseconds(interval);
        ++counter;
    }
    if (counter > timeout) {
        return -1;
    }
    return counter;
}

/* Piezo Sound Handling (inspired and using code from the Sparkfun project at https://learn.sparkfun.com/tutorials/sparkfun-inventors-kit-for-photon-experiment-guide/experiment-5-music-time */
void makeSound(duration, char note) {
  tone(speakerPin, frequency(note), duration);
}

int frequency(char note)
{
  // This function takes a note character (a-g), and returns the
  // corresponding frequency in Hz for the tone() function.

  int i;
  const int numNotes = 8;  // number of notes we're storing

  // The following arrays hold the note characters and their
  // corresponding frequencies. The last "C" note is uppercase
  // to separate it from the first lowercase "c". If you want to
  // add more notes, you'll need to use unique characters.

  // For the "char" (character) type, we put single characters
  // in single quotes.

  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523};

  // Now we'll search through the letters in the array, and if
  // we find it, we'll return the frequency for that note.

  for (i = 0; i < numNotes; i++)  // Step through the notes
  {
    if (names[i] == note)         // Is this the one?
    {
      return(frequencies[i]);     // Yes! Return the frequency
    }
  }
  return(0);  // We looked through everything and didn't find it,
              // but we still need to return a value, so return 0.
}
