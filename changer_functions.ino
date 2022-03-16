//Multiplexier pins for relay board
const int selectPins[3] = {13, 15, 0};      // S0, S1, S2 | D7, D8, D3
const int aOutput = 2;                      //D4

const int dirty_float_leveler = A0;     //A0
const int empty_dirty_water_pump = 0;
const int empty_float_level = 500; //0


void set_float_level(struct tank* t) {
    t->empty_float_level = analogRead(dirty_float_leveler);
    EEPROM.put(t->offset, t->empty_float_level);
    EEPROM.commit();
}


/** Sets intital values of each tank structure */
void set_tanks(struct tank* small, struct tank* large) {
    large->clean_pump = 2;
    large->dirty_pump = 4;
    large->offset = 0;
    EEPROM.get(large->offset, large->empty_float_level);
    large->proximity_sensor = 5; //D1

    small->clean_pump = 1;
    small->dirty_pump = 3;
    small->offset = 4;
    EEPROM.get(small->offset, small->empty_float_level);
    small->proximity_sensor = 4; //D2
}

/* Turns on a single relay block for a specified value
 *  
 * PARAM: item_id, value acossiated to relay block
*/
void turn_on_item(int item_id) {
  selectMuxPin(item_id);
  digitalWrite(aOutput, LOW);
}

/* Turns off a single relay block for a specified value
 *  
 * PARAM: item_id, value acossiated to relay block
*/
void turn_off_item(int item_id) {
  selectMuxPin(item_id);
  digitalWrite(aOutput, HIGH);
}


void pump_clean_in_tank(struct tank* t) {
    selectMuxPin(t->clean_pump);
    while(digitalRead(t->proximity_sensor) == LOW) {
      digitalWrite(aOutput, LOW);
      delay(500);
    }
    digitalWrite(aOutput, HIGH);
}


void pump_dirty_out_tank(struct tank* t) {
    selectMuxPin(t->dirty_pump);
    while (analogRead(dirty_float_leveler) > t->empty_float_level) {
      digitalWrite(aOutput, LOW);
      delay(500);
    }
    digitalWrite(aOutput, HIGH);
}


void perform_whole_change(struct tank* t) {
    if (check_dirty_bucket) {
        return;
    }

    pump_dirty_out_tank(t);
    pump_clean_in_tank(t);
    empty_dirty_bucket();

    return;
}


/** checks if dirty bucket is full or not */
bool check_dirty_bucket() {
    Serial.println(analogRead(dirty_float_leveler));

    if (analogRead(dirty_float_leveler) < empty_float_level) {
        return true;
    } else {
        return false;
    }
}


/* Turns on the dirty emptying pump to empty dirty water */
void empty_dirty_bucket() {
    selectMuxPin(empty_dirty_water_pump);
    while (check_dirty_bucket()) {
      digitalWrite(aOutput, LOW);
      delay(500);
    }
    digitalWrite(aOutput, HIGH);
}


/* The selectMuxPin function sets the S0, S1, and S2 pins to select the give pin
 *  
 * PARAM: pin to select
 */
void selectMuxPin(byte pin)
{
  if (pin > 7) return; // Exit if pin is out of scope
  for (int i=0; i<3; i++)
  {
    if (pin & (1<<i))
      digitalWrite(selectPins[i], HIGH);
    else
      digitalWrite(selectPins[i], LOW);
  }
}


/* Sets all relays off first */
void set_relays() {
  for (int pin = 0; pin < 8; pin++) {
    selectMuxPin(pin);
    delay(100);
    digitalWrite(aOutput, HIGH);
    delay(100);
  }
}