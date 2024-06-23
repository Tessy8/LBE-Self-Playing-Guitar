#include <Arduino.h>

// Define pin connections & motor's steps per revolution
const int stepsPerRevolution = 9;  // Number of steps per revolution for the stepper motor
const int numStrings = 6;  // Number of guitar strings
const int numFrets = 9;  // Number of frets

// Pin assignments for step and direction pins for each stepper motor
int stepPins[numStrings] = {2, 4, 6, 8, 10, 12};  // Step pins
int dirPins[numStrings] = {3, 5, 7, 9, 11, 13};   // Direction pins

// Solenoid pins
int solenoids[numStrings] = {26, 27, 28, 29, 30, 31};  // Pins for solenoids

// Array to track the current fret position of each string
int currentFretPosition[numStrings] = {0, 0, 0, 0, 0, 0};

// Structure to hold note data
struct NoteData {
  int noteNumber;
  unsigned long ticks;
  float timeInSeconds;
  int stringNumber;
  int fretNumber;
};

// Define the array of notes (hard-coded)
NoteData notesArray[] = {
  {61, 0, 0.0, 4, 6},
  {62, 0, 0.25, 5, 2},
  {63, 0, 0.5, 4, 4},
  {64, 0, 1, 5, 9},
  {65, 0, 2, 4, 1},
  {66, 0, 2.25, 5, 1}
};

// Number of notes in the array
const int notesArrayLength = sizeof(notesArray) / sizeof(notesArray[0]);

// Variable to store the start time
unsigned long startTime = 0;

// Variables for non-blocking delays
unsigned long lastStepTime = 0;
int currentNoteIndex = 0;
bool stepperInProgress = false;
int stepsRemaining = 0;
bool solenoidActivated = false;
unsigned long solenoidActivationTime = 0;

void setup() {
  Serial.begin(9600);

  // Set up stepper motor pins
  for (int i = 0; i < numStrings; i++) {
    pinMode(stepPins[i], OUTPUT);
    pinMode(dirPins[i], OUTPUT);
    digitalWrite(stepPins[i], LOW);
    digitalWrite(dirPins[i], LOW);
  }

  // Set up solenoid pins as output
  for (int i = 0; i < numStrings; i++) {
    pinMode(solenoids[i], OUTPUT);
    digitalWrite(solenoids[i], LOW);  // Ensure solenoids are initially off
  }
}

// Function to move the stepper motor until all steps are done
void stepMotorNonBlocking(int stepPin) {
  unsigned long currentTime;
  while (stepsRemaining > 0) {
    currentTime = micros();
    if (currentTime - lastStepTime >= 100) {  // 100 microseconds delay per step
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(1);  // Short delay for the step pulse
      digitalWrite(stepPin, LOW);
      stepsRemaining--;  // Decrement the steps remaining
      lastStepTime = currentTime;  // Update the last step time

      // Print stepper motor movement
      Serial.print("Stepper motor moving, steps remaining: ");
      Serial.println(stepsRemaining);
    }
  }
  stepperInProgress = false;  // Stop the stepper motor if no steps are remaining
  Serial.println("Stepper motor movement complete");
}

void loop() {
  unsigned long currentTime = micros();

  if (currentNoteIndex < notesArrayLength) {
    NoteData &note = notesArray[currentNoteIndex];

    if (startTime == 0) {  // Initialize start time when playing the first note
      startTime = currentTime;
      Serial.println("Starting playback");
    }

    // Convert note time from seconds to microseconds for comparison
    unsigned long noteTimeInMicros = (unsigned long)(note.timeInSeconds * 1000000);

    if (currentTime - startTime >= noteTimeInMicros) {  // Check if it's time to play the current note
      if (!stepperInProgress && !solenoidActivated) {  // If the stepper motor is not moving and the solenoid is not activated
        Serial.println(currentTime / 1000);  // Added for debugging
        // Calculate the number of steps to move to the desired fret
        int stepsToMove = (note.fretNumber - currentFretPosition[note.stringNumber - 1]) * (stepsPerRevolution / numFrets);
        bool direction = stepsToMove >= 0;  // Determine the direction
        stepsToMove = abs(stepsToMove);  // Ensure stepsToMove is positive

        // Print note information
        Serial.print("Playing note: ");
        Serial.print(note.noteNumber);
        Serial.print(" on string: ");
        Serial.print(note.stringNumber);
        Serial.print(" at fret: ");
        Serial.print(note.fretNumber);
        Serial.print(" with steps: ");
        Serial.println(stepsToMove);

        // Set up the stepper motor movement
        stepsRemaining = stepsToMove;
        stepperInProgress = true;
        digitalWrite(dirPins[note.stringNumber - 1], direction);
        lastStepTime = micros();  // Set the initial step time

        // Update the current fret position
        currentFretPosition[note.stringNumber - 1] = note.fretNumber;

        // Perform stepper motor movement
        stepMotorNonBlocking(stepPins[note.stringNumber - 1]);
      }

      // Activate the solenoid after stepper motor movement is complete
      if (!stepperInProgress && !solenoidActivated) {
        digitalWrite(solenoids[note.stringNumber - 1], HIGH);
        solenoidActivationTime = millis();
        solenoidActivated = true;
        Serial.print("Solenoid activated on string: ");
        Serial.println(note.stringNumber);
      }

      // Deactivate the solenoid after 100 milliseconds
      if (solenoidActivated && millis() - solenoidActivationTime >= 100) {
        digitalWrite(solenoids[note.stringNumber - 1], LOW);
        solenoidActivated = false;
        Serial.print("Solenoid deactivated on string: ");
        Serial.println(note.stringNumber);
        currentNoteIndex++;  // Move to the next note
      }
    }
  }
}
