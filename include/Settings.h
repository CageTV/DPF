#pragma once
#include <cstdint>
#include <string>

namespace Settings {
    struct Config {
        std::string dynamicPluginFile = "DPF.esp";
        std::string jsonMirrorPath;
        uint32_t firstDynamicLocalId = 0x801;
    };

    Config Load();
}
