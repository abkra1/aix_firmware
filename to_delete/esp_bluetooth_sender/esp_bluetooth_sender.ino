
#include "BluetoothA2DPSource.h"

BluetoothA2DPSource a2dp_source;

// callback 
int32_t get_sound_data(Frame *data, int32_t len) {
    // generate your sound data 
    // return the length of the generated sound - which usually is identical with len
    return len;
}

void setup() {
  a2dp_source.start("MyMusic", get_sound_data);  
}

void loop() {
}
