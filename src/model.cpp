#include "model.h"
#include <algorithm>

namespace {
    constexpr uint32_t kMaxDynamicPluginCount = 255;
    constexpr uint32_t kMaxLocalId = 0x00ffffff;
    bool capacityWarningQueued = false;

    void QueueCapacityWarning(const std::string& nextPluginName) {
        if (capacityWarningQueued) {
            return;
        }
        capacityWarningQueued = true;

        const auto message = "Dynamic Persistent Forms has no available FormID slots. Install and enable " +
            nextPluginName + ", keep all previous DPF plugin files enabled, then restart Skyrim.";
        if (auto* taskInterface = SKSE::GetTaskInterface()) {
            taskInterface->AddTask([message]() {
                RE::DebugMessageBox(message.c_str());
            });
        } else {
            logger::error("Could not display the DPF capacity warning because the SKSE task interface is unavailable");
        }
    }

    void RemoveRuntimeRecordForSlot(const uint32_t pluginNumber, const uint32_t localId, const bool deleteActualForm) {
        const auto formId = MakeDynamicFormID(pluginNumber, localId);
        for (auto it = formData.begin(); it != formData.end();) {
            auto* item = *it;
            if (item && item->formId == formId) {
                if (deleteActualForm && item->actualForm) {
                    item->actualForm->SetDelete(true);
                }
                delete item;
                it = formData.erase(it);
            } else {
                ++it;
            }
        }
    }

    void RemoveOwnerKeyIndex(const DynamicSlot& slot) {
        if (!slot.owner.empty() || !slot.key.empty()) {
            dynamicOwnerKeyIndex.erase(MakeOwnerKey(slot.owner, slot.key));
        }
    }
}

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
    if (const auto pluginNumber = GetDynamicPluginNumber(item->formId);
        pluginNumber.has_value() && item->formType != RE::FormType::None) {
        RegisterDynamicSlot(pluginNumber.value(), ToDynamicLocalID(item->formId), item->formType);
    }
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

std::string FormatDynamicPluginName(const uint32_t pluginNumber) {
    if (pluginNumber == 0) {
        return {};
    }
    if (pluginNumber == 1) {
        return "DPF.esp";
    }
    return "DPF " + std::to_string(pluginNumber) + ".esp";
}

DynamicSlotId MakeDynamicSlotId(const uint32_t pluginNumber, const uint32_t localId) {
    return (static_cast<DynamicSlotId>(pluginNumber) << 24) | (localId & kMaxLocalId);
}

DynamicPlugin* GetDynamicPlugin(const uint32_t pluginNumber) {
    const auto it = std::ranges::find_if(dynamicPlugins, [pluginNumber](const auto& plugin) {
        return plugin.number == pluginNumber;
    });
    return it == dynamicPlugins.end() ? nullptr : &*it;
}

std::optional<uint32_t> GetDynamicPluginNumber(const RE::FormID formId) {
    if (formId == 0) {
        return std::nullopt;
    }

    const auto modId = (formId >> 24) & 0xff;
    const auto it = std::ranges::find_if(dynamicPlugins, [modId](const auto& plugin) {
        return plugin.modId == modId;
    });
    if (it == dynamicPlugins.end()) {
        return std::nullopt;
    }
    return it->number;
}

bool IsDynamicFormID(const RE::FormID formId) {
    return GetDynamicPluginNumber(formId).has_value();
}

uint32_t ToDynamicLocalID(const RE::FormID formId) {
    return formId & kMaxLocalId;
}

RE::FormID MakeDynamicFormID(const uint32_t pluginNumber, const uint32_t localId) {
    const auto* plugin = GetDynamicPlugin(pluginNumber);
    if (!plugin || localId == 0) {
        return 0;
    }
    return (plugin->modId << 24) | (localId & kMaxLocalId);
}

RE::FormID MakeDynamicFormID(const uint32_t localId) {
    return MakeDynamicFormID(1, localId);
}

void ReserveDynamicLocalID(const uint32_t pluginNumber, const uint32_t localId) {
    if (pluginNumber == 0 || localId == 0) {
        return;
    }

    const auto normalized = localId & kMaxLocalId;
    reservedDynamicSlots.insert(MakeDynamicSlotId(pluginNumber, normalized));
    if (auto* plugin = GetDynamicPlugin(pluginNumber); plugin && normalized >= plugin->nextLocalId) {
        plugin->nextLocalId = normalized + 1;
    }
}

void ReserveDynamicFormID(const RE::FormID formId) {
    if (const auto pluginNumber = GetDynamicPluginNumber(formId)) {
        ReserveDynamicLocalID(pluginNumber.value(), ToDynamicLocalID(formId));
    }
}

std::string NormalizeOwnerKeyPart(const char* value) {
    return value ? std::string(value) : std::string();
}

std::string MakeOwnerKey(std::string_view owner, std::string_view key) {
    std::string result;
    result.reserve(owner.size() + key.size() + 1);
    result.append(owner.data(), owner.size());
    result.push_back('\x1f');
    result.append(key.data(), key.size());
    return result;
}

bool RegisterDynamicSlot(const uint32_t pluginNumber, const uint32_t localId, const RE::FormType formType,
    std::string owner, std::string key) {
    const auto normalized = localId & kMaxLocalId;
    if (pluginNumber == 0 || normalized == 0 || formType == RE::FormType::None) {
        return false;
    }

    const auto slotId = MakeDynamicSlotId(pluginNumber, normalized);
    const auto ownerKey = MakeOwnerKey(owner, key);
    const bool hasOwnerKey = !owner.empty() || !key.empty();
    if (hasOwnerKey) {
        const auto existingByOwner = dynamicOwnerKeyIndex.find(ownerKey);
        if (existingByOwner != dynamicOwnerKeyIndex.end() && existingByOwner->second != slotId) {
            logger::error("Owner/key '{}'/'{}' is already registered to DPF slot {}:{:06X}", owner, key,
                existingByOwner->second >> 24, existingByOwner->second & kMaxLocalId);
            return false;
        }
    }

    const auto existing = dynamicSlots.find(slotId);
    if (existing != dynamicSlots.end()) {
        const auto& slot = existing->second;
        if (slot.formType != formType) {
            logger::error("Dynamic slot {}:{:06X} is already registered as type {}, requested {}", pluginNumber,
                normalized, static_cast<uint32_t>(slot.formType), static_cast<uint32_t>(formType));
            return false;
        }
        if (hasOwnerKey && (slot.owner != owner || slot.key != key)) {
            logger::error("Dynamic slot {}:{:06X} is owned by '{}'/'{}', requested '{}'/'{}'", pluginNumber,
                normalized, slot.owner, slot.key, owner, key);
            return false;
        }
        ReserveDynamicLocalID(pluginNumber, normalized);
        return true;
    }

    DynamicSlot slot;
    slot.pluginNumber = pluginNumber;
    slot.localId = normalized;
    slot.formType = formType;
    slot.owner = std::move(owner);
    slot.key = std::move(key);
    dynamicSlots[slotId] = slot;
    if (!slot.owner.empty() || !slot.key.empty()) {
        dynamicOwnerKeyIndex[MakeOwnerKey(slot.owner, slot.key)] = slotId;
    }
    ReserveDynamicLocalID(pluginNumber, normalized);
    return true;
}

std::optional<DynamicSlot> GetRegisteredDynamicSlot(const uint32_t pluginNumber, const uint32_t localId) {
    const auto existing = dynamicSlots.find(MakeDynamicSlotId(pluginNumber, localId));
    if (existing == dynamicSlots.end()) {
        return std::nullopt;
    }
    return existing->second;
}

std::optional<DynamicSlotAddress> FindDynamicSlotByOwnerKey(std::string_view owner, std::string_view key) {
    const auto existing = dynamicOwnerKeyIndex.find(MakeOwnerKey(owner, key));
    if (existing == dynamicOwnerKeyIndex.end()) {
        return std::nullopt;
    }

    return DynamicSlotAddress{
        static_cast<uint32_t>(existing->second >> 24),
        static_cast<uint32_t>(existing->second & kMaxLocalId)
    };
}

bool ReleaseDynamicSlot(const uint32_t pluginNumber, const uint32_t localId, std::string_view owner) {
    const auto normalized = localId & kMaxLocalId;
    const auto slotId = MakeDynamicSlotId(pluginNumber, normalized);
    const auto existing = dynamicSlots.find(slotId);
    if (existing == dynamicSlots.end()) {
        return false;
    }
    if (existing->second.owner != owner) {
        logger::error("Owner '{}' cannot release slot {}:{:06X} owned by '{}'", owner, pluginNumber, normalized,
            existing->second.owner);
        return false;
    }

    RemoveRuntimeRecordForSlot(pluginNumber, normalized, true);
    RemoveOwnerKeyIndex(existing->second);
    dynamicSlots.erase(existing);
    reservedDynamicSlots.erase(slotId);
    if (auto* plugin = GetDynamicPlugin(pluginNumber)) {
        plugin->nextLocalId = std::min(plugin->nextLocalId, normalized);
    }
    return true;
}

bool ReleaseDynamicSlotByOwnerKey(std::string_view owner, std::string_view key) {
    const auto address = FindDynamicSlotByOwnerKey(owner, key);
    if (!address.has_value()) {
        return false;
    }
    return ReleaseDynamicSlot(address->pluginNumber, address->localId, owner);
}

uint32_t ReleaseDynamicSlotsByOwner(std::string_view owner) {
    std::vector<DynamicSlotAddress> toRelease;
    for (const auto& [slotId, slot] : dynamicSlots) {
        if (slot.owner == owner) {
            toRelease.push_back({slot.pluginNumber, slot.localId});
        }
    }

    uint32_t released = 0;
    for (const auto& address : toRelease) {
        if (ReleaseDynamicSlot(address.pluginNumber, address.localId, owner)) {
            ++released;
        }
    }
    return released;
}

std::optional<DynamicSlotAddress> AllocateDynamicSlot() {
    if (!espFound || dynamicPlugins.empty()) {
        logger::error("Cannot allocate a DynamicPersistentForms slot because DPF.esp was not found");
        return std::nullopt;
    }

    for (auto& plugin : dynamicPlugins) {
        uint32_t localId = std::max(plugin.nextLocalId, firstDynamicLocalId);
        while (localId <= kMaxLocalId) {
            const auto slotId = MakeDynamicSlotId(plugin.number, localId);
            const auto fullId = MakeDynamicFormID(plugin.number, localId);
            const bool reserved = reservedDynamicSlots.contains(slotId);
            const bool registered = dynamicSlots.contains(slotId);
            const bool existsInGame = RE::TESForm::LookupByID(fullId) != nullptr;
            if (!reserved && !registered && !existsInGame) {
                ReserveDynamicLocalID(plugin.number, localId);
                logger::info("Allocated DynamicPersistentForms slot {}:{:06X} ({:08X}) from {}", plugin.number,
                    localId, fullId, plugin.name);
                return DynamicSlotAddress{plugin.number, localId};
            }
            ++localId;
        }

        plugin.nextLocalId = kMaxLocalId + 1;
        logger::warn("{} has no available FormID slots; trying the next DPF plugin", plugin.name);
    }

    const auto nextPluginName = FormatDynamicPluginName(dynamicPlugins.back().number + 1);
    logger::error("All loaded DPF plugins are full. Install and enable {}", nextPluginName);
    QueueCapacityWarning(nextPluginName);
    return std::nullopt;
}

void ReadFirstFormIdFromESP() {
    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    espFound = false;
    dynamicModId = 0;
    dynamicPluginName = FormatDynamicPluginName(1);
    dynamicPlugins.clear();
    capacityWarningQueued = false;

    if (!dataHandler) {
        logger::error("TESDataHandler is unavailable while discovering DPF plugins");
        return;
    }

    for (uint32_t pluginNumber = 1; pluginNumber <= kMaxDynamicPluginCount; ++pluginNumber) {
        const auto pluginName = FormatDynamicPluginName(pluginNumber);
        const auto modIndex = dataHandler->GetLoadedModIndex(pluginName);
        if (!modIndex.has_value()) {
            if (pluginNumber == 1) {
                logger::error("DPF.esp was not found");
            }
            break;
        }

        if (modIndex.value() == 0xfe) {
            logger::error("{} is light/ESL. DPF plugin files must be full plugins", pluginName);
            break;
        }

        dynamicPlugins.push_back({pluginNumber, modIndex.value(), firstDynamicLocalId, pluginName});
        logger::info("{} found at mod index {:02X}", pluginName, modIndex.value());
    }

    if (dynamicPlugins.empty()) {
        return;
    }

    espFound = true;
    dynamicModId = dynamicPlugins.front().modId;
    logger::info("Discovered {} contiguous DPF plugin file(s)", dynamicPlugins.size());
}

void ResetDynamicState() {
    reservedDynamicSlots.clear();
    dynamicSlots.clear();
    dynamicOwnerKeyIndex.clear();
    for (auto& plugin : dynamicPlugins) {
        plugin.nextLocalId = firstDynamicLocalId;
    }
    nextRecordOrder = 1;
}
