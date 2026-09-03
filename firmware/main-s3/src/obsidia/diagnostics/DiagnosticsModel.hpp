#pragma once

#include "../services/FpgaService.hpp"
#include "../services/RadioService.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace obsidia {

class DiagnosticsModel {
public:
    enum class State : std::uint8_t { Unknown, Pass, Fail, Unavailable, Untested };
    enum class Component : std::uint8_t {
        MainMcu,
        BruceVersion,
        ObsidiaFirmwareVersion,
        Display,
        Input,
        Sd,
        Pn532,
        Ir,
        Wifi,
        Ble,
        Usb,
        RadioMcu,
        ObsidiaLinkVersion,
        Rdm6300,
        Cc1101,
        Nrf24,
        Fpga,
        FpgaVersion,
        FpgaIrq,
        FpgaFifo,
        Count,
    };

    static constexpr std::size_t kValueCapacity = 24;
    static constexpr std::size_t kEntryCount = static_cast<std::size_t>(Component::Count);

    struct Entry {
        const char *label;
        State state;
        std::array<char, kValueCapacity> value;
    };

    DiagnosticsModel();

    void set(Component component, State state, const char *value = nullptr);
    void updateRadio(const RadioService::Snapshot &snapshot);
    void updateFpga(const FpgaService::Snapshot &snapshot);
    const std::array<Entry, kEntryCount> &entries() const { return entries_; }
    static const char *stateText(State state);

private:
    static std::size_t index(Component component) { return static_cast<std::size_t>(component); }
    static State peripheralState(std::uint8_t ready, std::uint8_t supported, std::uint8_t bit);

    std::array<Entry, kEntryCount> entries_;
};

} // namespace obsidia
