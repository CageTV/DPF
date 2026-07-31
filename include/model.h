#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "form_record.h"

struct DynamicPlugin {
    uint32_t number = 0;
    uint32_t modId = 0;
    uint32_t nextLocalId = 0x801;
    std::string name;
};

struct DynamicSlotAddress {
    uint32_t pluginNumber = 0;
    uint32_t localId = 0;
};

struct DynamicSlot {
    uint32_t pluginNumber = 1;
    uint32_t localId = 0;
    RE::FormType formType = RE::FormType::None;
    std::string owner;
    std::string key;
};

using DynamicSlotId = uint64_t;

inline std::vector<FormRecord*> formData;
inline std::vector<FormRecord*> formRef;

// These primary-plugin values remain available for the legacy serializers.
inline bool espFound = false;
inline uint32_t firstDynamicLocalId = 0x801;
inline uint32_t dynamicModId = 0;
inline std::string dynamicPluginName = "DPF.esp";

inline uint32_t nextRecordOrder = 1;
inline std::vector<DynamicPlugin> dynamicPlugins;
inline std::unordered_set<DynamicSlotId> reservedDynamicSlots;
inline std::unordered_map<DynamicSlotId, DynamicSlot> dynamicSlots;
inline std::unordered_map<std::string, DynamicSlotId> dynamicOwnerKeyIndex;

void AddFormData(FormRecord* item);

void AddFormRef(FormRecord* item);

void EachFormData(const std::function<bool(FormRecord*)>& iteration);

void EachFormRef(const std::function<bool(FormRecord*)>& iteration);

std::string FormatDynamicPluginName(uint32_t pluginNumber);

DynamicSlotId MakeDynamicSlotId(uint32_t pluginNumber, uint32_t localId);

DynamicPlugin* GetDynamicPlugin(uint32_t pluginNumber);

std::optional<uint32_t> GetDynamicPluginNumber(RE::FormID formId);

bool IsDynamicFormID(RE::FormID formId);

uint32_t ToDynamicLocalID(RE::FormID formId);

RE::FormID MakeDynamicFormID(uint32_t pluginNumber, uint32_t localId);

RE::FormID MakeDynamicFormID(uint32_t localId);

void ReserveDynamicLocalID(uint32_t pluginNumber, uint32_t localId);

void ReserveDynamicFormID(RE::FormID formId);

std::string NormalizeOwnerKeyPart(const char* value);

std::string MakeOwnerKey(std::string_view owner, std::string_view key);

bool RegisterDynamicSlot(uint32_t pluginNumber, uint32_t localId, RE::FormType formType,
    std::string owner = {}, std::string key = {});

std::optional<DynamicSlot> GetRegisteredDynamicSlot(uint32_t pluginNumber, uint32_t localId);

std::optional<DynamicSlotAddress> FindDynamicSlotByOwnerKey(std::string_view owner, std::string_view key);

bool ReleaseDynamicSlot(uint32_t pluginNumber, uint32_t localId, std::string_view owner);

bool ReleaseDynamicSlotByOwnerKey(std::string_view owner, std::string_view key);

uint32_t ReleaseDynamicSlotsByOwner(std::string_view owner);

std::optional<DynamicSlotAddress> AllocateDynamicSlot();

void ReadFirstFormIdFromESP();

void ResetDynamicState();
