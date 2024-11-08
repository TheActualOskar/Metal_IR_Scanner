// Pin definitions
int IR_LED_PIN = 13; 
int IR_SENSE_PIN = 2; // Digital pin for the IR sensor
#define capPin A1
#define pulsePin A0

// Global variables
volatile byte breakCount = 0; // Number of breaks detected by the IR sensor
long sumExpect = 0; // Running sum of capacitor readings
long ignor = 0;     // Number of ignored sums
long diff = 0;      // Difference between sum and avgsum
int metalCount = 0; // Counter for metal detection events
int nonMetalCount = 0; // Counter for non-metal detection events
int itemCount = 0;  // Total item count based on metal and non-metal counts
int detectionThreshold = 50; // Adjust for sensitivity

// Previous counts to check changes
int prevMetalCount = 0; // Store previous metal count
int IR_count = 0; // Counter for IR breaks

// Interrupt service routine for counting breaks
void breakcount()
{
    breakCount++; // Increment the break count
}

void setup() 
{
    Serial.begin(9600);
    pinMode(IR_LED_PIN, OUTPUT);
    digitalWrite(IR_LED_PIN, HIGH);

    // Attach interrupt to the IR sensor pin
    attachInterrupt(digitalPinToInterrupt(IR_SENSE_PIN), breakcount, FALLING);
  
    pinMode(pulsePin, OUTPUT); 
    digitalWrite(pulsePin, LOW);
    pinMode(capPin, INPUT);
}

void loop() 
{
    // Metal detection logic
    int minval = 1023;
    int maxval = 0;
    long unsigned int sum = 0;

    for (int i = 0; i < 256; i++) 
    {
        // Reset the capacitor
        pinMode(capPin, OUTPUT);
        digitalWrite(capPin, LOW);
        delayMicroseconds(20);
        pinMode(capPin, INPUT);
        applyPulses();

        // Read the charge of the capacitor
        int val = analogRead(capPin); 
        minval = min(val, minval);
        maxval = max(val, maxval);
        sum += val;
    }

    sum -= minval; // Subtract minimum value
    sum -= maxval; // Subtract maximum value

    if (sumExpect == 0) 
        sumExpect = sum << 6; // Set sumExpect to expected value

    long int avgsum = (sumExpect + 32) >> 6;
    diff = sum - avgsum;

    // Metal detection condition
    if (abs(diff) > detectionThreshold) 
    {
        metalCount++; // Increment metal count
        delay(1000); // Prevent multiple counts for a single detection
    }

    // Update expected sum for less significant differences
    if (abs(diff) < avgsum >> 10) 
    {
        sumExpect = sumExpect + sum - avgsum;
        ignor = 0;
    } 
    else 
    {
        ignor++;
        if (ignor > 64) 
        { 
            sumExpect = sum << 6;
            ignor = 0;
        }
    }

    // Check if an IR break has occurred
    if (breakCount > 0) {
        IR_count++; // Increment the IR break count

        // Determine if an item is counted as metal or non-metal
        if (metalCount > prevMetalCount) {
            // Metal detected and IR break detected
            itemCount++; // Increment item count for metal detection
        } else {
            // No metal detected, count as non-metal
            nonMetalCount++; // Increment non-metal count
        }

        // Reset break count after processing
        breakCount = 0; // Reset break count to avoid multiple increments for the same break
    }

    // Update previous counts
    prevMetalCount = metalCount;

    // Print current counts
    Serial.print("Metal Detected: ");
    Serial.print(metalCount);
    Serial.print(" | Non-Metal: ");
    Serial.print(nonMetalCount);
    Serial.print(" | Item thrown: ");
    Serial.print(IR_count);
    Serial.print(" | Total Metal item: ");
    Serial.println(itemCount);

    delay(1000); // Control the update rate
}

void applyPulses() 
{
    for (int i = 0; i < 3; i++) 
    {
        digitalWrite(pulsePin, HIGH);
        delayMicroseconds(3);
        digitalWrite(pulsePin, LOW);
        delayMicroseconds(3);
    }
}