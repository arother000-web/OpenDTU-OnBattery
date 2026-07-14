#include <Arduino.h>
#include <PZEM004Tv30.h>

#define PZEM_RX_PIN 25
#define PZEM_TX_PIN 26

PZEM004Tv30 pzem_hardware(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

float read_custom_pzem_watt() {
    float power = pzem_hardware.power();
    if (isnan(power)) {
        return 0.0;
    }
    return power;
}
