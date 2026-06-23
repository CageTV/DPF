#include "Settings.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>
#include <filesystem>

namespace {
    constexpr auto kSettingsPath = "Data/SKSE/Plugins/DPF_Settings.json";

    uint32_t ParseLocalId(const rapidjson::Value& value, const uint32_t fallback) {
        if (value.IsUint()) {
            return value.GetUint();
        }
        if (!value.IsString()) {
            return fallback;
        }

        try {
            return static_cast<uint32_t>(std::stoul(value.GetString(), nullptr, 0));
        } catch (...) {
            return fallback;
        }
    }
}

Settings::Config Settings::Load() {
    Config config;

    if (!std::filesystem::exists(kSettingsPath)) {
        return config;
    }

    FILE* file = nullptr;
    fopen_s(&file, kSettingsPath, "rb");
    if (!file) {
        logger::warn("Could not open {}", kSettingsPath);
        return config;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream stream(file, readBuffer, sizeof(readBuffer));
    rapidjson::Document document;
    document.ParseStream(stream);
    fclose(file);

    if (document.HasParseError() || !document.IsObject()) {
        logger::warn("{} is not valid JSON", kSettingsPath);
        return config;
    }

    if (document.HasMember("DynamicPluginFile") && document["DynamicPluginFile"].IsString()) {
        config.dynamicPluginFile = document["DynamicPluginFile"].GetString();
    }
    if (document.HasMember("JsonMirrorPath") && document["JsonMirrorPath"].IsString()) {
        config.jsonMirrorPath = document["JsonMirrorPath"].GetString();
    }
    if (document.HasMember("FirstDynamicLocalId")) {
        config.firstDynamicLocalId = ParseLocalId(document["FirstDynamicLocalId"], config.firstDynamicLocalId);
    }

    if (config.firstDynamicLocalId < 0x800) {
        config.firstDynamicLocalId = 0x801;
    }

    return config;
}
