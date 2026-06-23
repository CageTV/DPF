#include "Services.h"
#include "form.h"

RE::TESForm* Services::Create(RE::TESForm* form) { std::lock_guard lock(serviceMutex); return form ? AddForm(form) : nullptr; }
RE::TESForm* Services::CreateByType(uint32_t type) { std::lock_guard lock(serviceMutex); return AddFormByType(static_cast<RE::FormType>(type)); }
RE::TESForm* Services::GetOrCreateByLocalId(uint32_t id, uint32_t type) { std::lock_guard lock(serviceMutex); return GetOrCreateFormByLocalId(id, static_cast<RE::FormType>(type)); }
RE::TESForm* Services::GetOrCreateByFormId(RE::FormID id, uint32_t type) { std::lock_guard lock(serviceMutex); return GetOrCreateFormByFormId(id, static_cast<RE::FormType>(type)); }
RE::TESForm* Services::GetOrCreateByOwnerKey(const char* owner, const char* key, uint32_t type, uint32_t* id, bool* existed) { std::lock_guard lock(serviceMutex); return GetOrCreateFormByOwnerKey(owner, key, static_cast<RE::FormType>(type), id, existed); }
bool Services::ReleaseByOwnerKey(const char* owner, const char* key) { std::lock_guard lock(serviceMutex); return ReleaseFormByOwnerKey(owner, key); }
bool Services::ReleaseByLocalId(uint32_t id, const char* owner) { std::lock_guard lock(serviceMutex); return ReleaseFormByLocalId(id, owner); }
uint32_t Services::ReleaseOwner(const char* owner) { std::lock_guard lock(serviceMutex); return ReleaseFormsByOwner(owner); }
