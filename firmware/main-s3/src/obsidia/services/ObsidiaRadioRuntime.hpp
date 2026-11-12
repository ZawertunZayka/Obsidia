#pragma once

#ifdef OBSIDIA_V1

#include "RadioService.hpp"

namespace obsidia {

void beginRadioRuntime();
void pollRadioRuntime();
RadioService::Snapshot radioRuntimeSnapshot();

struct RemoteNrf24Status {
    std::uint8_t status{0};
    std::uint8_t config{0};
    std::uint8_t rfChannel{0};
    std::uint8_t rfSetup{0};
};

bool remoteNrf24Status(RemoteNrf24Status &status);
bool remoteNrf24SetChannel(std::uint8_t channel);
bool remoteNrf24ReadRpd(std::uint8_t channel, std::uint8_t &rpd);
bool remoteNrf24StartCarrier(std::uint8_t channel, std::uint8_t power);
bool remoteNrf24Stop();

} // namespace obsidia

#endif
