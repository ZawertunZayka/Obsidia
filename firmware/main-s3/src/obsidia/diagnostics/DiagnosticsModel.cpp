#include "DiagnosticsModel.hpp"

#include <cstdio>

namespace obsidia {

DiagnosticsModel::DiagnosticsModel() : entries_{{
    {"Main MCU", State::Unknown, {}},
    {"Bruce version", State::Unknown, {}},
    {"Obsidia firmware", State::Unknown, {}},
    {"Display", State::Untested, {}},
    {"Input", State::Untested, {}},
    {"SD", State::Untested, {}},
    {"PN532", State::Untested, {}},
    {"IR", State::Untested, {}},
    {"Wi-Fi", State::Untested, {}},
    {"BLE", State::Untested, {}},
    {"USB", State::Untested, {}},
    {"Radio MCU", State::Unavailable, {}},
    {"ObsidiaLink version", State::Unknown, {}},
    {"RDM6300", State::Untested, {}},
    {"CC1101", State::Untested, {}},
    {"NRF24", State::Untested, {}},
    {"FPGA", State::Unavailable, {}},
    {"FPGA version", State::Unknown, {}},
    {"FPGA IRQ", State::Untested, {}},
    {"FPGA FIFO", State::Untested, {}},
}} {}

void DiagnosticsModel::set(Component component, State state, const char *value) {
    auto &entry = entries_[index(component)];
    entry.state = state;
    entry.value.fill('\0');
    if (value == nullptr) return;
    std::size_t position = 0;
    while (value[position] != '\0' && position + 1 < entry.value.size()) {
        entry.value[position] = value[position];
        ++position;
    }
}

DiagnosticsModel::State DiagnosticsModel::peripheralState(
    std::uint8_t ready,
    std::uint8_t supported,
    std::uint8_t bit
) {
    if ((supported & bit) == 0) return State::Unavailable;
    return (ready & bit) != 0 ? State::Pass : State::Untested;
}

void DiagnosticsModel::updateRadio(const RadioService::Snapshot &snapshot) {
    char version[kValueCapacity]{};
    std::snprintf(
        version,
        sizeof(version),
        "%u.%u.%u",
        static_cast<unsigned>(snapshot.firmwareVersion[0]),
        static_cast<unsigned>(snapshot.firmwareVersion[1]),
        static_cast<unsigned>(snapshot.firmwareVersion[2])
    );
    const bool ready = snapshot.health == RadioService::Health::Ready;
    set(Component::RadioMcu, ready ? State::Pass : State::Unavailable, ready ? version : nullptr);

    char protocol[kValueCapacity]{};
    std::snprintf(protocol, sizeof(protocol), "%u", static_cast<unsigned>(snapshot.protocolVersion));
    set(Component::ObsidiaLinkVersion, ready ? State::Pass : State::Unknown, ready ? protocol : nullptr);
    set(
        Component::Rdm6300,
        ready ? peripheralState(snapshot.readyMask, snapshot.supportedMask, 1U << 0U) : State::Unavailable
    );
    set(
        Component::Cc1101,
        ready ? peripheralState(snapshot.readyMask, snapshot.supportedMask, 1U << 1U) : State::Unavailable
    );
    set(
        Component::Nrf24,
        ready ? peripheralState(snapshot.readyMask, snapshot.supportedMask, 1U << 2U) : State::Unavailable
    );
}

void DiagnosticsModel::updateFpga(const FpgaService::Snapshot &snapshot) {
    const bool ready = snapshot.health == FpgaService::Health::Ready;
    set(Component::Fpga, ready ? State::Pass : State::Unavailable);
    char version[kValueCapacity]{};
    std::snprintf(
        version,
        sizeof(version),
        "%u.%u",
        static_cast<unsigned>(snapshot.version[0]),
        static_cast<unsigned>(snapshot.version[1])
    );
    set(Component::FpgaVersion, ready ? State::Pass : State::Unknown, ready ? version : nullptr);
    // IRQ and FIFO require explicit standalone hardware evidence; discovery
    // alone must never promote either diagnostic to PASS.
    if (!ready) {
        set(Component::FpgaIrq, State::Unavailable);
        set(Component::FpgaFifo, State::Unavailable);
    }
}

const char *DiagnosticsModel::stateText(State state) {
    switch (state) {
    case State::Unknown: return "UNKNOWN";
    case State::Pass: return "OK";
    case State::Fail: return "FAIL";
    case State::Unavailable: return "N/A";
    case State::Untested: return "UNTESTED";
    }
    return "UNKNOWN";
}

} // namespace obsidia
