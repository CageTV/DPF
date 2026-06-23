#include "form.h"
#include "model.h"

void copyFormArmorModel(RE::TESForm* source, RE::TESForm* target) {
    const auto* sourceModelBipedForm = source->As<RE::TESObjectARMO>();
    auto* targeteModelBipedForm = target->As<RE::TESObjectARMO>();
    if (sourceModelBipedForm && targeteModelBipedForm) {
        logger::trace("armor");
        targeteModelBipedForm->armorAddons = sourceModelBipedForm->armorAddons;
    }
}

void copyFormObjectWeaponModel(RE::TESForm* source, RE::TESForm* target) {
    const auto* sourceModelWeapon = source->As<RE::TESObjectWEAP>();
    auto* targeteModelWeapon = target->As<RE::TESObjectWEAP>();
    if (sourceModelWeapon && targeteModelWeapon) {
        logger::trace("weapon");
        targeteModelWeapon->firstPersonModelObject = sourceModelWeapon->firstPersonModelObject;
        targeteModelWeapon->attackSound = sourceModelWeapon->attackSound;
        targeteModelWeapon->attackSound2D = sourceModelWeapon->attackSound2D;
        targeteModelWeapon->attackSound = sourceModelWeapon->attackSound;
        targeteModelWeapon->attackFailSound = sourceModelWeapon->attackFailSound;
        targeteModelWeapon->idleSound = sourceModelWeapon->idleSound;
        targeteModelWeapon->equipSound = sourceModelWeapon->equipSound;
        targeteModelWeapon->unequipSound = sourceModelWeapon->unequipSound;
        targeteModelWeapon->soundLevel = sourceModelWeapon->soundLevel;
    }
}

void copyMagicEffect(RE::TESForm* source, RE::TESForm* target) {
    const auto* sourceEffect = source->As<RE::EffectSetting>();
    auto* targetEffect = target->As<RE::EffectSetting>();
    if (sourceEffect && targetEffect) {
        targetEffect->effectSounds = sourceEffect->effectSounds;
        targetEffect->data.castingArt = sourceEffect->data.castingArt;
        targetEffect->data.light = sourceEffect->data.light;
        targetEffect->data.hitEffectArt = sourceEffect->data.hitEffectArt;
        targetEffect->data.effectShader = sourceEffect->data.effectShader;
        targetEffect->data.hitVisuals = sourceEffect->data.hitVisuals;
        targetEffect->data.enchantShader = sourceEffect->data.enchantShader;
        targetEffect->data.enchantEffectArt = sourceEffect->data.enchantEffectArt;
        targetEffect->data.enchantVisuals = sourceEffect->data.enchantVisuals;
        targetEffect->data.projectileBase = sourceEffect->data.projectileBase;
        targetEffect->data.explosion = sourceEffect->data.explosion;
        targetEffect->data.impactDataSet = sourceEffect->data.impactDataSet;
        targetEffect->data.imageSpaceMod = sourceEffect->data.imageSpaceMod;
    }
}

void copyBookAppearence(RE::TESForm* source, RE::TESForm* target) {
    const auto* sourceBook = source->As<RE::TESObjectBOOK>();
    auto* targetBook = target->As<RE::TESObjectBOOK>();
    if (sourceBook && targetBook) {
        targetBook->inventoryModel = sourceBook->inventoryModel;
    }
}

void applyPattern(FormRecord* instance) {
    if (!instance) {
        return;
    }

    const auto baseForm = instance->baseForm;
    const auto newForm = instance->actualForm;

    if (baseForm && newForm) {
        const auto weaponBaseForm = baseForm->As<RE::TESObjectWEAP>();
        const auto weaponNewForm = newForm->As<RE::TESObjectWEAP>();

        const auto bookBaseForm = baseForm->As<RE::TESObjectBOOK>();
        const auto bookNewForm = newForm->As<RE::TESObjectBOOK>();

        const auto ammoBaseForm = baseForm->As<RE::TESAmmo>();
        const auto ammoNewForm = newForm->As<RE::TESAmmo>();

        if (weaponNewForm && weaponBaseForm) {
            weaponNewForm->firstPersonModelObject = weaponBaseForm->firstPersonModelObject;

            weaponNewForm->weaponData = weaponBaseForm->weaponData;
            weaponNewForm->criticalData = weaponBaseForm->criticalData;

            weaponNewForm->attackSound = weaponBaseForm->attackSound;
            weaponNewForm->attackSound2D = weaponBaseForm->attackSound2D;
            weaponNewForm->attackSound = weaponBaseForm->attackSound;
            weaponNewForm->attackFailSound = weaponBaseForm->attackFailSound;
            weaponNewForm->idleSound = weaponBaseForm->idleSound;
            weaponNewForm->equipSound = weaponBaseForm->equipSound;
            weaponNewForm->unequipSound = weaponBaseForm->unequipSound;
            weaponNewForm->soundLevel = weaponBaseForm->soundLevel;

            weaponNewForm->impactDataSet = weaponBaseForm->impactDataSet;
            weaponNewForm->templateWeapon = weaponBaseForm->templateWeapon;
            weaponNewForm->embeddedNode = weaponBaseForm->embeddedNode;
        } else if (bookBaseForm && bookNewForm) {
            bookNewForm->data.flags = bookBaseForm->data.flags;
            bookNewForm->data.teaches.spell = bookBaseForm->data.teaches.spell;
            bookNewForm->data.teaches.actorValueToAdvance = bookBaseForm->data.teaches.actorValueToAdvance;
            bookNewForm->data.type = bookBaseForm->data.type;
            bookNewForm->inventoryModel = bookBaseForm->inventoryModel;
            bookNewForm->itemCardDescription = bookBaseForm->itemCardDescription;
        } else if (ammoBaseForm && ammoNewForm) {
            ammoNewForm->GetRuntimeData().data.damage = ammoBaseForm->GetRuntimeData().data.damage;
            ammoNewForm->GetRuntimeData().data.flags = ammoBaseForm->GetRuntimeData().data.flags;
            ammoNewForm->GetRuntimeData().data.projectile = ammoBaseForm->GetRuntimeData().data.projectile;
        } else {
            newForm->Copy(baseForm);
        }

        copyComponent<RE::TESDescription>(baseForm, newForm);
        copyComponent<RE::BGSKeywordForm>(baseForm, newForm);
        copyComponent<RE::BGSPickupPutdownSounds>(baseForm, newForm);
        copyComponent<RE::TESModelTextureSwap>(baseForm, newForm);
        copyComponent<RE::TESModel>(baseForm, newForm);
        copyComponent<RE::BGSMessageIcon>(baseForm, newForm);
        copyComponent<RE::TESIcon>(baseForm, newForm);
        copyComponent<RE::TESFullName>(baseForm, newForm);
        copyComponent<RE::TESValueForm>(baseForm, newForm);
        copyComponent<RE::TESWeightForm>(baseForm, newForm);
        copyComponent<RE::BGSDestructibleObjectForm>(baseForm, newForm);
        copyComponent<RE::TESEnchantableForm>(baseForm, newForm);
        copyComponent<RE::BGSBlockBashData>(baseForm, newForm);
        copyComponent<RE::BGSEquipType>(baseForm, newForm);
        copyComponent<RE::TESAttackDamageForm>(baseForm, newForm);
        copyComponent<RE::TESBipedModelForm>(baseForm, newForm);
    }
    if (newForm && instance->modelForm) {
        copyAppearence(instance->modelForm, newForm);
    }
}

void copyAppearence(RE::TESForm* source, RE::TESForm* target) {
    copyFormArmorModel(source, target);
    copyFormObjectWeaponModel(source, target);
    copyMagicEffect(source, target);
    copyBookAppearence(source, target);
    copyComponent<RE::BGSPickupPutdownSounds>(source, target);
    copyComponent<RE::BGSMenuDisplayObject>(source, target);
    copyComponent<RE::TESModel>(source, target);
    copyComponent<RE::TESBipedModelForm>(source, target);
}


namespace {
    bool EnsureFormInDataHandler(RE::TESForm* form) {
        if (!form) return false;
        auto* handler = RE::TESDataHandler::GetSingleton();
        if (!handler) return false;
        auto& forms = handler->GetFormArray(form->GetFormType());
        if (std::ranges::find(forms, form) == forms.end()) handler->AddFormToDataHandler(form);
        return true;
    }

    RE::TESForm* CreateFormInstance(const RE::FormType formType, const RE::FormID formId) {
        const auto factory = RE::IFormFactory::GetFormFactoryByType(formType);
        if (!factory) {
            logger::error("No form factory for form type {}", static_cast<uint32_t>(formType));
            return nullptr;
        }

        auto* result = factory->Create();
        if (!result) {
            logger::error("Factory returned null for form type {}", static_cast<uint32_t>(formType));
            return nullptr;
        }

        result->SetFormID(formId, false);
        return result;
    }

    RE::TESForm* ReuseDeletedSlot(const RE::FormType formType, RE::TESForm* baseItem) {
        RE::TESForm* result = nullptr;
        EachFormData([&](FormRecord* item) {
            if (!item->deleted || item->formType != formType || !IsDynamicFormID(item->formId)) {
                return true;
            }

            logger::info("Reusing deleted DPF slot {:08X}", item->formId);
            result = CreateFormInstance(formType, item->formId);
            if (!result) {
                return false;
            }

            item->Undelete(result, formType);
            item->baseForm = baseItem;
            item->modelForm = nullptr;
            if (baseItem) {
                applyPattern(item);
            }
            return false;
        });
        return result;
    }
}

RE::TESForm* AddForm(RE::TESForm* baseItem) {
    if (!baseItem) {
        logger::error("Create(baseItem) was called with a null baseItem. Use CreateByType for empty forms.");
        return nullptr;
    }

    const auto formId = AllocateDynamicFormID();
    auto* form = formId ? GetOrCreateFormByLocalId(ToDynamicLocalID(formId), baseItem->GetFormType()) : nullptr;
    if (form) form->Copy(baseItem);
    return form;
}

RE::TESForm* AddFormByType(const RE::FormType formType) {
    if (formType == RE::FormType::None) {
        logger::error("CreateByType called with FormType::None");
        return nullptr;
    }

    const auto formId = AllocateDynamicFormID();
    return formId ? GetOrCreateFormByLocalId(ToDynamicLocalID(formId), formType) : nullptr;
}

RE::TESForm* GetOrCreateFormByLocalId(uint32_t localId, const RE::FormType type) {
    localId &= 0x00ffffff;
    if (!espFound || !localId || type == RE::FormType::None) return nullptr;
    if (const auto slot = GetRegisteredDynamicSlot(localId); slot && slot->formType != type) return nullptr;
    const auto id = MakeDynamicFormID(localId);
    if (auto* form = RE::TESForm::LookupByID(id)) {
        if (form->GetFormType() != type || !RegisterDynamicSlot(localId, type)) return nullptr;
        EnsureFormInDataHandler(form);
        return form;
    }
    if (!RegisterDynamicSlot(localId, type)) return nullptr;
    auto* form = CreateFormInstance(type, id);
    if (!form) return nullptr;
    EnsureFormInDataHandler(form);
    AddFormData(FormRecord::CreateNew(form, type, id));
    SaveGlobalRegistry();
    return form;
}

RE::TESForm* GetOrCreateFormByFormId(const RE::FormID id, const RE::FormType type) {
    return IsDynamicFormID(id) ? GetOrCreateFormByLocalId(ToDynamicLocalID(id), type) : nullptr;
}

RE::TESForm* GetOrCreateFormByOwnerKey(const char* ownerRaw, const char* keyRaw, const RE::FormType type, uint32_t* localId, bool* existed) {
    if (localId) *localId = 0; if (existed) *existed = false;
    const auto owner = NormalizeOwnerKeyPart(ownerRaw), key = NormalizeOwnerKeyPart(keyRaw);
    if (owner.empty() || key.empty()) return nullptr;
    if (const auto existing = FindDynamicSlotByOwnerKey(owner, key)) {
        if (localId) *localId = *existing; if (existed) *existed = true;
        return GetOrCreateFormByLocalId(*existing, type);
    }
    const auto id = AllocateDynamicFormID(); if (!id) return nullptr;
    const auto slot = ToDynamicLocalID(id);
    if (!RegisterDynamicSlot(slot, type, owner, key)) return nullptr;
    auto* form = GetOrCreateFormByLocalId(slot, type);
    if (!form) return nullptr;
    if (localId) *localId = slot;
    SaveGlobalRegistry();
    return form;
}

bool ReleaseFormByOwnerKey(const char* owner, const char* key) { const auto ok = ReleaseDynamicSlotByOwnerKey(NormalizeOwnerKeyPart(owner), NormalizeOwnerKeyPart(key)); if (ok) SaveGlobalRegistry(); return ok; }
bool ReleaseFormByLocalId(uint32_t id, const char* owner) { const auto ok = ReleaseDynamicSlot(id, NormalizeOwnerKeyPart(owner)); if (ok) SaveGlobalRegistry(); return ok; }
uint32_t ReleaseFormsByOwner(const char* owner) { const auto count = ReleaseDynamicSlotsByOwner(NormalizeOwnerKeyPart(owner)); if (count) SaveGlobalRegistry(); return count; }
