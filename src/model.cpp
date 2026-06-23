#include "model.h"
#include "Settings.h"
#include <algorithm>
#include <array>

void AddFormData(FormRecord* item) {
    if (!item) {
        return;
    }
    if (item->order == 0) {
        item->order = nextRecordOrder++;
    } else if (item->order >= nextRecordOrder) {
        nextRecordOrder = item->order + 1;
    }
    formData.push_back(item);
    ReserveDynamicFormID(item->formId);
}

void AddFormRef(FormRecord* item) {
    if (!item) {
        return;
    }
    if (item->order == 0) {
        item->order = nextRecordOrder++;
    } else if (item->order >= nextRecordOrder) {
        nextRecordOrder = item->order + 1;
    }
    formRef.push_back(item);
}

void EachFormData(const std::function<bool(FormRecord*)>& iteration) {
    for (const auto item : formData) {
        if (!iteration(item)) {
            return;
        }
    }
}

void EachFormRef(const std::function<bool(FormRecord*)>& iteration) {
    for (const auto item : formRef) {
        if (!iteration(item)) {
            return;
        }
    }
}

bool IsDynamicFormID(const RE::FormID formId) {
    if (!espFound || formId == 0) {
        return false;
    }
    return ((formId >> 24) & 0xff) == dynamicModId;
}

uint32_t ToDynamicLocalID(const RE::FormID formId) {
    return formId & 0x00ffffff;
}

RE::FormID MakeDynamicFormID(const uint32_t localId) {
    return (dynamicModId << 24) | (localId & 0x00ffffff);
}

void ReserveDynamicLocalID(const uint32_t localId) {
    if (localId == 0) {
        return;
    }
    reservedDynamicLocalIds.insert(localId & 0x00ffffff);
    if (normalized >= nextDynamicLocalId) {
        nextDynamicLocalId = normalized + 1;
    }
}

void ReserveDynamicFormID(const RE::FormID formId) {
    if (IsDynamicFormID(formId)) {
        ReserveDynamicLocalID(ToDynamicLocalID(formId));
    }
}

RE::FormID AllocateDynamicFormID() {
    if (!espFound) {
        logger::error("Cannot allocate DPF FormID because the dynamic ESP was not found");
        return 0;
    }

    uint32_t localId = std::max(nextDynamicLocalId, firstDynamicLocalId);
    while (localId < 0x00ffffff) {
        const auto fullId = MakeDynamicFormID(localId);
        const bool reserved = reservedDynamicLocalIds.contains(localId) || dynamicSlots.contains(localId);
        const bool existsInGame = RE::TESForm::LookupByID(fullId) != nullptr;
        if (!reserved && !existsInGame) {
            ReserveDynamicLocalID(localId);
            logger::info("Allocated DPF FormID {:08X}", fullId);
            return fullId;
        }
        ++localId;
    }

    logger::error("DPF dynamic ESP has no available FormID slots");
    return 0;
}

void ReadFirstFormIdFromESP() {
    const auto settings = Settings::Load();
    const auto dataHandler = RE::TESDataHandler::GetSingleton();
    const std::array candidates{
        std::string("DPF.esp")
    };

    espFound = false;
    dynamicModId = 0;
    firstDynamicLocalId = settings.firstDynamicLocalId;
    nextDynamicLocalId = firstDynamicLocalId;

    for (const auto& pluginName : candidates) {
        if (pluginName.empty()) {
            continue;
        }

        const auto modIndex = dataHandler->GetLoadedModIndex(pluginName);
        if (!modIndex.has_value()) {
            continue;
        }

        espFound = true;
        dynamicPluginName = "DPF.esp";
        dynamicModId = modIndex.value();
        logger::info("DPF dynamic ESP '{}' found at mod index {:02X}", dynamicPluginName, dynamicModId);
        return;
    }

    logger::error("DPF.esp was not found");
}

void ResetDynamicState() {
    reservedDynamicLocalIds.clear();
    dynamicSlots.clear();
    dynamicOwnerKeyIndex.clear();
    nextDynamicLocalId = firstDynamicLocalId;
    nextRecordOrder = 1;
}

std::string NormalizeOwnerKeyPart(const char* value) { return value ? std::string(value) : std::string(); }

std::string MakeOwnerKey(std::string_view owner, std::string_view key) {
    return std::string(owner) + '\x1f' + std::string(key);
}

bool RegisterDynamicSlot(const uint32_t localId, const RE::FormType formType, std::string owner, std::string key) {
    const auto id = localId & 0x00ffffff;
    if (!id || formType == RE::FormType::None) return false;
    if (!owner.empty() || !key.empty()) {
        const auto ownerKey = MakeOwnerKey(owner, key);
        if (const auto it = dynamicOwnerKeyIndex.find(ownerKey); it != dynamicOwnerKeyIndex.end() && it->second != id) return false;
    }
    if (const auto it = dynamicSlots.find(id); it != dynamicSlots.end()) {
        if (it->second.formType != formType) return false;
        if ((!owner.empty() || !key.empty()) && (it->second.owner != owner || it->second.key != key)) return false;
    } else {
        dynamicSlots.emplace(id, DynamicSlot{ id, formType, std::move(owner), std::move(key) });
    }
    const auto& slot = dynamicSlots.at(id);
    if (!slot.owner.empty() || !slot.key.empty()) dynamicOwnerKeyIndex[MakeOwnerKey(slot.owner, slot.key)] = id;
    ReserveDynamicLocalID(id);
    return true;
}

std::optional<DynamicSlot> GetRegisteredDynamicSlot(const uint32_t localId) {
    const auto it = dynamicSlots.find(localId & 0x00ffffff);
    return it == dynamicSlots.end() ? std::nullopt : std::optional<DynamicSlot>(it->second);
}

std::optional<uint32_t> FindDynamicSlotByOwnerKey(std::string_view owner, std::string_view key) {
    const auto it = dynamicOwnerKeyIndex.find(MakeOwnerKey(owner, key));
    return it == dynamicOwnerKeyIndex.end() ? std::nullopt : std::optional<uint32_t>(it->second);
}

bool ReleaseDynamicSlot(const uint32_t localId, std::string_view owner) {
    const auto id = localId & 0x00ffffff;
    const auto it = dynamicSlots.find(id);
    if (it == dynamicSlots.end() || (!owner.empty() && it->second.owner != owner)) return false;
    if (!it->second.owner.empty() || !it->second.key.empty()) dynamicOwnerKeyIndex.erase(MakeOwnerKey(it->second.owner, it->second.key));
    dynamicSlots.erase(it);
    reservedDynamicLocalIds.erase(id);
    nextDynamicLocalId = std::min(nextDynamicLocalId, id);
    return true;
}

bool ReleaseDynamicSlotByOwnerKey(std::string_view owner, std::string_view key) {
    const auto id = FindDynamicSlotByOwnerKey(owner, key);
    return id && ReleaseDynamicSlot(*id, owner);
}

uint32_t ReleaseDynamicSlotsByOwner(std::string_view owner) {
    std::vector<uint32_t> ids;
    for (const auto& [id, slot] : dynamicSlots) if (slot.owner == owner) ids.push_back(id);
    for (const auto id : ids) ReleaseDynamicSlot(id, owner);
    return static_cast<uint32_t>(ids.size());
}
