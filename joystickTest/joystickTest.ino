/*
This is test sketch for the WCRS custom joystick arduino shield.
12345678901234567890123456789012345678901234567890123456789012345678901234567890
This reports changing values for the configured joystick pins.  This includes
the X and Y axis position readings, as well as the state of the push button
switch.
*/

// Configure which arduino pins are connected to the joystick pins
const unsigned int xPin = A2; // joystick X axis analog output
const unsigned int yPin = A1; // joystick Y axis analog output
const unsigned int swPin = 14; // joystick push button switch
// The amount of change needed before new analog values are reported
const unsigned int deltaMin = 5; // Hysteresis limit to avoid reporting jitter

// Extra delay time after a new set of values are reported, before next possible
// report (milliseconds)
#define REPORTING_DELAY 100

// Varibles to hold the most recent *reported* joystick values
unsigned int joyX;
unsigned int joyY;
unsigned int joySw;

void setup() {
  Serial.begin ( 115200 );
  Serial.println ( F( "WCRS joystick shield test!" ));
  pinMode ( swPin, INPUT_PULLUP );  // The switch is digital.  Only on or off.

  // Set dummy values for the last reported readings, so that ANY reading will
  // be a new value
  joyX = 9999;
  joyY = 9999;
  joySw = 9999;
}// ./void setup()

void loop() {
  boolean newVal = false; // Nothing to report yet (this time through loop)
  unsigned int readX = analogRead ( xPin ); // Get raw readings from joystick
  unsigned int readY = analogRead ( yPin );
  unsigned int readSw = digitalRead ( swPin );

  // Any time one of the readings is enough different from the last reported
  // value, save the new value and remember that a new report is needed.
  if ( deltaOverHysteresis ( joyX, readX )) {
    joyX = readX;
    newVal = true;
  }
  if ( deltaOverHysteresis ( joyY, readY )) {
    joyY = readY;
    newVal = true;
  }
  if ( readSw != joySw ) {
    joySw = readSw;
    newVal = true;
  }

  if ( newVal ) {
    // A new (changed) value was noticed.  Report the whole set of values
    Serial.print ( F( "X =" ));
    Serial.print ( joyX );
    Serial.print ( F( ", Y =" ));
    Serial.print ( joyY );
    Serial.print ( F( ", SW =" ));
    Serial.println ( joySw );
    delay( REPORTING_DELAY );
  }
}// ./void loop()

/* Is the change more than the Hysteresis (stickiness) limit ?
 *
 * @param current the current (stuck) value
 * @param proposed the new value that might still be stuck
 * @return boolean true when proposed is far enough from current to unstick
 */
boolean deltaOverHysteresis ( unsigned int current, unsigned int proposed ) {
  // All values are unsigned, which means it is not safe to directly compare with
  // negative offsets.  The generated value could be negative, that with unsigned
  // variables results in a very large postive value.  The compare would not work.
  // To avoid that, offset (add to) all values by the maximum negative delta that
  // could be used in the comparisons.  The comparison expressions will then
  // always be positive values.
  unsigned int oCur = current + deltaMin;
  unsigned int oPro = proposed + deltaMin;
  return ( oPro < oCur - deltaMin || oPro > oCur + deltaMin );
}
