#include <Arduino.h>
#include <PZEM004Tv30.h>

// Ambil data global dari OpenDTU pin mapping
extern "C" {
    int8_t pzem_rx_pin = 25;
    int8_t pzem_tx_pin = 26;
}

PZEM004Tv30 pzem_hardware;
bool is_pzem_initialized = false;

void init_custom_pzem() {
    if (!is_pzem_initialized) {
        // Menggunakan Hardware Serial 2 ESP32 (UART2)
        pzem_hardware = PZEM004Tv30(Serial2, pzem_rx_pin, pzem_tx_pin);
        is_pzem_initialized = true;
    }
}

float read_custom_pzem_watt() {
    init_custom_pzem();
    float power = pzem_hardware.power();
    if (isnan(power)) {
        return 0.0; // Kembalikan nilai nol jika sensor belum mendeteksi setrum AC 220V
    }
    return power;
}
