// SPDX-License-Identifier: GPL-2.0-or-later
#include <powermeter/sdm/serial/Provider.h>
#include <PinMapping.h>
#include <LogHelper.h>

#undef TAG
static const char* TAG = "powerMeter";
static const char* SUBTAG = "SDM";

// Ambil jembatan fungsi dari Pzem.cpp
extern float read_custom_pzem_watt();

namespace PowerMeters::Sdm::Serial {

Provider::~Provider() {
    _taskDone = false;
    std::unique_lock<std::mutex> lock(_pollingMutex);
    _stopPolling = true;
    lock.unlock();
    _cv.notify_all();

    if (_taskHandle != nullptr) {
        while (!_taskDone) {
            delay(10);
        }
        _taskHandle = nullptr;
    }

    if (_upSdmSerial) {
        _upSdmSerial->end();
        _upSdmSerial = nullptr;
    }
}

bool Provider::init() {
    return true; // Bypass untuk kestabilan modul 4MB
}

void Provider::loop() {
    if (_taskHandle != nullptr) {
        return;
    }
    std::unique_lock<std::mutex> lock(_pollingMutex);
    _stopPolling = false;
    lock.unlock();

    uint32_t constexpr stackSize = 3072;
    xTaskCreate(Provider::pollingLoopHelper, "PM:SDM", stackSize, this, 1/*prio*/, &_taskHandle);
}

bool Provider::isDataValid() const {
    return true; // Paksa status data selalu online
}

void Provider::pollingLoopHelper(void* context) {
    auto pInstance = static_cast<Provider*>(context);
    pInstance->pollingLoop();
    pInstance->_taskDone = true;
    vTaskDelete(nullptr);
}

bool Provider::readValue(std::unique_lock<std::mutex>& lock, uint16_t reg, float& targetVar) {
    return true;
}

void Provider::pollingLoop() {
    std::unique_lock<std::mutex> lock(_pollingMutex);
    while (!_stopPolling) {
        // Panggil fungsi pembacaan fisik PZEM-004T Anda
        float phase1Power = read_custom_pzem_watt();
        float phase1Voltage = 220.0;
        float energyImport = 0.0;
        float energyExport = 0.0;

        {
            auto scopedLock = _dataCurrent.lock();
            _dataCurrent.add<DataPointLabel::PowerL1>(phase1Power);
            _dataCurrent.add<DataPointLabel::VoltageL1>(phase1Voltage);
            _dataCurrent.add<DataPointLabel::Import>(energyImport);
            _dataCurrent.add<DataPointLabel::Export>(energyExport);
        }

        DTU_LOGD("Suntikan PZEM Sukses: %5.2f W", phase1Power);
        _cv.wait_for(lock, std::chrono::milliseconds(2000), [this] { return _stopPolling; });
    }
}

} // namespace PowerMeters::Sdm::Serial
