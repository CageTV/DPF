#include "persistence.h"
#include "model.h"
#include "serializer.h"
#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace {
constexpr uint32_t kMagic = 0x44504643;
constexpr uint8_t kVersion = 2;
std::mutex registryMutex;

template <class T> void WriteVarUInt(Serializer<T>* s, uint32_t value) {
    while (value >= 0x80) { s->template Write<uint8_t>(static_cast<uint8_t>(value | 0x80)); value >>= 7; }
    s->template Write<uint8_t>(static_cast<uint8_t>(value));
}
template <class T> uint32_t ReadVarUInt(Serializer<T>* s) {
    uint32_t result = 0, shift = 0;
    while (shift < 32) { const auto byte = s->template Read<uint8_t>(); result |= uint32_t(byte & 0x7f) << shift; if (!(byte & 0x80)) return result; shift += 7; }
    return 0;
}
template <class T> void WriteString(Serializer<T>* s, const std::string& value) {
    WriteVarUInt(s, static_cast<uint32_t>(value.size())); for (const auto ch : value) s->template Write<char>(ch);
}
template <class T> std::string ReadString(Serializer<T>* s) {
    std::string value(ReadVarUInt(s), '\0'); for (auto& ch : value) ch = s->template Read<char>(); return value;
}
void RecalculateNext() {
    for (uint32_t id = firstDynamicLocalId; id < 0x00ffffff; ++id) {
        if (!reservedDynamicLocalIds.contains(id) && !dynamicSlots.contains(id)) { nextDynamicLocalId = id; return; }
    }
    nextDynamicLocalId = 0x00ffffff;
}
template <class T> void Store(Serializer<T>* s) {
    std::vector<DynamicSlot> slots;
    for (const auto& [id, slot] : dynamicSlots) slots.push_back(slot);
    std::ranges::sort(slots, {}, &DynamicSlot::localId);
    std::unordered_map<std::string, uint32_t> indexes;
    std::vector<std::string> owners;
    for (const auto& slot : slots) if (!slot.owner.empty() && !indexes.contains(slot.owner)) { indexes[slot.owner] = static_cast<uint32_t>(owners.size()); owners.push_back(slot.owner); }
    s->template Write<uint32_t>(kMagic); s->template Write<uint8_t>(kVersion); WriteString(s, dynamicPluginName);
    WriteVarUInt(s, static_cast<uint32_t>(owners.size())); for (const auto& owner : owners) WriteString(s, owner);
    WriteVarUInt(s, static_cast<uint32_t>(slots.size()));
    for (const auto& slot : slots) { WriteVarUInt(s, slot.localId); WriteVarUInt(s, static_cast<uint32_t>(slot.formType)); WriteVarUInt(s, slot.owner.empty() ? 0 : indexes[slot.owner] + 1); WriteString(s, slot.key); }
}
template <class T> bool Restore(Serializer<T>* s) {
    if (s->template Read<uint32_t>() != kMagic || s->template Read<uint8_t>() != kVersion) { ResetDynamicState(); return SaveGlobalRegistry(); }
    ResetDynamicState(); ReadString(s); dynamicPluginName = "DPF.esp";
    std::vector<std::string> owners; const auto ownerCount = ReadVarUInt(s); owners.reserve(ownerCount);
    for (uint32_t i = 0; i < ownerCount; ++i) owners.push_back(ReadString(s));
    const auto slotCount = ReadVarUInt(s);
    for (uint32_t i = 0; i < slotCount; ++i) { const auto id = ReadVarUInt(s); const auto type = static_cast<RE::FormType>(ReadVarUInt(s)); const auto ownerIndex = ReadVarUInt(s); const auto key = ReadString(s); RegisterDynamicSlot(id, type, ownerIndex && ownerIndex <= owners.size() ? owners[ownerIndex - 1] : "", key); }
    RecalculateNext(); return true;
}
}

std::string GetGlobalRegistryPath() { return "Data/SKSE/Plugins/DPF_Cache.bin"; }
bool LoadGlobalRegistry() {
    std::lock_guard lock(registryMutex); const auto path = GetGlobalRegistryPath();
    if (!fs::exists(path)) { ResetDynamicState(); return SaveGlobalRegistry(); }
    FileReader reader(path, std::ios::in | std::ios::binary); return reader.IsOpen() && Restore(&reader);
}
bool SaveGlobalRegistry() {
    const auto path = GetGlobalRegistryPath();
    try { fs::create_directories(fs::path(path).parent_path()); FileWriter writer(path, std::ios::out | std::ios::binary | std::ios::trunc); if (!writer.IsOpen()) return false; Store(&writer); return true; } catch (...) { return false; }
}
