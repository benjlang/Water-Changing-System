#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <EEPROM.h>

//ir remote sensor
const uint16_t ir_sensor = 12;

IRrecv irrecv(ir_sensor);
decode_results results;

extern const int selectPins[3]; // S0, S1, S2
extern const int aOutput; // Connect common (Z) to 5 (PIM-capable)
extern const int dirty_float_leveler;

struct tank {
    int proximity_sensor;
    int empty_float_level;
    int offset;
    int clean_pump;
    int dirty_pump;
};

//Water sensors
struct tank small;
struct tank large;


void setup() {
    EEPROM.begin(12);

    set_tanks(&small, &large);

    pinMode(large.proximity_sensor, INPUT);
    pinMode(small.proximity_sensor, INPUT);
    pinMode(ir_sensor, INPUT);
    pinMode(dirty_float_leveler, INPUT);
    
    // Set up the select pins, as outputs
    for (int i=0; i<3; i++)
    {
        pinMode(selectPins[i], OUTPUT);
        digitalWrite(selectPins[i], LOW);
    }
    pinMode(aOutput, OUTPUT); // Set up Z as an output

    set_relays();

    irrecv.enableIRIn();  // Start the receiver
    
    Serial.begin(9600);
}

void loop() {
  if (irrecv.decode(&results)) {
    uint64_t number = results.value;
    unsigned long long1 = (unsigned long)((number & 0xFFFF0000) >> 16 );
    unsigned long long2 = (unsigned long)((number & 0x0000FFFF));

    String hex = String(long1, HEX) + String(long2, HEX); // six octets
  
    Serial.println(hex);

    //TODO: SET BUTTONS ONCE HAVE REMOTE
    
    if (results.value == 0x807f40bf) {
      Serial.println("OK Pressed");

      //turns all relays off
      set_relays();
    }
    else if (results.value == 0x807fc03f) {
      Serial.println("1 Pressed");
      
      perform_whole_change(&small);
    }
    else if (results.value == 0x807f827d) {
      Serial.println("2 Pressed");
      
      perform_whole_change(&large);
    }
    else if (results.value == 0x807f609f) {
      Serial.println("3 Pressed");
      
      turn_on_item(small.dirty_pump);
    }
    else if (results.value == 0x807fa25d) {
      Serial.println("4 Pressed");
      
      turn_on_item(large.dirty_pump);
    }
    else if (results.value == 0x807f50af) {
      Serial.println("TODO");
      
      turn_on_item(large.clean_pump);
    }
    else if (results.value == 0x807f50af) {
      Serial.println("TODO");
      
      turn_on_item(small.clean_pump);
    }
    else if (results.value == 0x807f50af) {
      Serial.println("TODO");
      
      set_float_level(&large);
    }
    else if (results.value == 0x807f50af) {
      Serial.println("TODO");
      
      set_float_level(&small);
    }
    else if (results.value == 0x807f50af) {
      Serial.println("0 Pressed");
      
      empty_dirty_bucket();
    }
    
    delay(100);
    irrecv.resume();  // Receive the next
  }
}