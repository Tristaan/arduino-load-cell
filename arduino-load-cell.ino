#include "HX711.h"
#include "lookup_tables.h"

// Constants
#define TEMPRESISTOR 10000

// Wiring
#define THERMISTOR_PIN A0
#define THERMISTOR_PULL_UP_PIN 2
#define LOADCELL_DOUT_PIN 8
#define LOADCELL_SCK_PIN 9
HX711 scale;

//Constant for get_units
float calibration_factor = 736.175;

// Temperature
float temperature = 0.0;

void setup() {
  pinMode(THERMISTOR_PULL_UP_PIN, OUTPUT);
  Serial.begin(115200);
  // Set scale data
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
  // Get temperature to global variable
  temperature = get_temp(thermistor_temperature);
}

void loop() {
  if(scale.is_ready()){
    // get_units function gets the reading and divides by the calibration factor,
    // this capability is built in and can be used. Then it divides by the water density
    // at a specific temperature and converts the reading into liters.
    float volume = scale.get_units(1)/get_density(temperature, water_density);
    // less than 0 cannot exist
    if(volume<0.0) {
      volume = 0;
    }
    Serial.print("Celsius: ");
    Serial.print(temperature, 4);
    Serial.print(", mililiter: ");
    Serial.println(volume, 4);
  } else {
    Serial.println("HX711 not found");
  }
  if(Serial.available() > 0) {
    char command;
    command = Serial.read();
    switch (command) {
      case 't':
        scale.tare();
        break;
      case 'm':
        temperature = get_temp(thermistor_temperature);
        break;
      default:
        break;
    };
  }
  delay(200);
}

// Function to get temperature from thermistor
float get_temp(float data[101][2]){
  float reading = 0;
  // Only 5V when reading from thermistor, prevent heating
  digitalWrite(THERMISTOR_PULL_UP_PIN, HIGH);
  // Read 10 values and calculate average
  for(int i = 0; i < 10; i++){
    reading += analogRead(THERMISTOR_PIN);
  }
  digitalWrite(THERMISTOR_PULL_UP_PIN, LOW);
  reading = reading/10;
  // Get resistance of thermistor from reading
  reading = TEMPRESISTOR / (1023/reading - 1);
  // Get temperature from lookup table (linear interpolation)
  for(int i=0; i<101; i++){
    if(reading <= data[i][1] && reading > data[i+1][1]) {
      return (data[i][0] + ((data[i+1][0]-data[i][0])*((reading-data[i][1])/(data[i+1][1]-data[i][1]))));
    }
  }
  return -1;
}

// Function to get density from temperature
float get_density(float temperature, float data[34][2]){
  for(int i=0; i<34; i++){
    if(temperature >= data[i][0] && temperature < data[i+1][0]) {
      return (data[i][1] + ((data[i+1][1]-data[i][1])*((temperature-data[i][0])/(data[i+1][0]-data[i][0]))));
    }
  }
  return -1;
}
