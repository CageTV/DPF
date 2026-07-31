#include "Services.h"
#include "form.h"
#include "model.h"

RE::TESForm* Services::Create(RE::TESForm* baseItem) {
    std::lock_guard lock(serviceMutex);
    try {
        if (!baseItem) return nullptr;

        auto* newForm = AddForm(baseItem);
        if (newForm) {
            logger::debug("new form id", newForm->GetFormID());
        }
        return newForm;
    } catch (const std::exception&) {
        return nullptr;
    }
}

RE::TESForm* Services::CreateByType(const uint32_t formType) {
    std::lock_guard lock(serviceMutex);
    try {
        auto* newForm = AddFormByType(static_cast<RE::FormType>(formType));
        if (newForm) {
            logger::debug("new form id", newForm->GetFormID());
        }
        return newForm;
    } catch (const std::exception&) {
        return nullptr;
    }
}

RE::TESForm* Services::GetOrCreateByPluginLocalId(const uint32_t pluginNumber, const uint32_t localId,
    const uint32_t formType) {
    std::lock_guard lock(serviceMutex);
    try {
        return GetOrCreateFormByPluginLocalId(pluginNumber, localId, static_cast<RE::FormType>(formType));
    } catch (const std::exception&) {
        return nullptr;
    }
}

RE::TESForm* Services::GetOrCreateByFormId(const RE::FormID formId, const uint32_t formType) {
    std::lock_guard lock(serviceMutex);
    try {
        return GetOrCreateFormByFormId(formId, static_cast<RE::FormType>(formType));
    } catch (const std::exception&) {
        return nullptr;
    }
}

RE::TESForm* Services::GetOrCreateByOwnerKey(const char* owner, const char* key, const uint32_t formType,
    uint32_t* pluginNumber, uint32_t* localId, bool* existed) {
    std::lock_guard lock(serviceMutex);
    try {
        return GetOrCreateFormByOwnerKey(owner, key, static_cast<RE::FormType>(formType), pluginNumber, localId,
            existed);
    } catch (const std::exception&) {
        if (pluginNumber) {
            *pluginNumber = 0;
        }
        if (localId) {
            *localId = 0;
        }
        if (existed) {
            *existed = false;
        }
        return nullptr;
    }
}

uint32_t Services::GetPluginNumberForFormId(const RE::FormID formId) {
    std::lock_guard lock(serviceMutex);
    return GetDynamicPluginNumber(formId).value_or(0);
}

bool Services::ReleaseByOwnerKey(const char* owner, const char* key) {
    std::lock_guard lock(serviceMutex);
    try {
        return ReleaseFormByOwnerKey(owner, key);
    } catch (const std::exception&) {
        return false;
    }
}

bool Services::ReleaseByPluginLocalId(const uint32_t pluginNumber, const uint32_t localId, const char* owner) {
    std::lock_guard lock(serviceMutex);
    try {
        return ReleaseFormByPluginLocalId(pluginNumber, localId, owner);
    } catch (const std::exception&) {
        return false;
    }
}

uint32_t Services::ReleaseOwner(const char* owner) {
    std::lock_guard lock(serviceMutex);
    try {
        return ReleaseFormsByOwner(owner);
    } catch (const std::exception&) {
        return 0;
    }
}
