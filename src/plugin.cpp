#include "Services.h"
#include "logger.h"
#include "model.h"
#include "papyrus.h"
#include "persistence.h"

class DPFInterfaceImpl : public DPF::IDynamicPersistentForms {
public:
    static DPFInterfaceImpl* GetSingleton() {
        static DPFInterfaceImpl instance;
        return &instance;
    }

    uint32_t GetVersion() const override {
        return DPF::InterfaceVersion;
    }

    RE::TESForm* Create(RE::TESForm* baseItem) override {
        return Services::Create(baseItem);
    }

    RE::TESForm* CreateByType(const uint32_t formType) override {
        return Services::CreateByType(formType);
    }

    RE::TESForm* GetOrCreateByPluginLocalId(const uint32_t pluginNumber, const uint32_t localId,
        const uint32_t formType) override {
        return Services::GetOrCreateByPluginLocalId(pluginNumber, localId, formType);
    }

    RE::TESForm* GetOrCreateByFormId(const RE::FormID formId, const uint32_t formType) override {
        return Services::GetOrCreateByFormId(formId, formType);
    }

    RE::TESForm* GetOrCreateByOwnerKey(const char* owner, const char* key, const uint32_t formType,
        uint32_t* pluginNumber, uint32_t* localId, bool* existed) override {
        return Services::GetOrCreateByOwnerKey(owner, key, formType, pluginNumber, localId, existed);
    }

    uint32_t GetPluginNumberForFormId(const RE::FormID formId) const override {
        return Services::GetPluginNumberForFormId(formId);
    }

    bool ReleaseByOwnerKey(const char* owner, const char* key) override {
        return Services::ReleaseByOwnerKey(owner, key);
    }

    bool ReleaseByPluginLocalId(const uint32_t pluginNumber, const uint32_t localId, const char* owner) override {
        return Services::ReleaseByPluginLocalId(pluginNumber, localId, owner);
    }

    uint32_t ReleaseOwner(const char* owner) override {
        return Services::ReleaseOwner(owner);
    }
};

extern "C" __declspec(dllexport) void* GetDPFAPI() {
    return DPFInterfaceImpl::GetSingleton();
}

namespace {
    void OnMessage(SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            ReadFirstFormIdFromESP();
            LoadGlobalRegistry();
            logger::info("DynamicPersistentForms data loaded");
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SetupLog();
    logger::info("Plugin loaded");

    SKSE::Init(skse);
    SKSE::GetPapyrusInterface()->Register(PapyrusFunctions);
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);

    return true;
}
