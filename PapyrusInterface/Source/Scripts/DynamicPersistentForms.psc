scriptName DynamicPersistentForms hidden

; Creates a new form that is a copy of the given base form.
Form function Create(Form item) global native

; Creates a new empty form of the requested FormType and lets DPF own its persistent FormID slot.
Form function CreateByType(int formType) global native

Form function GetOrCreateByLocalId(int localId, int formType) global native
Form function GetOrCreateByFormId(Form formIdSource, int formType) global native
Form function GetOrCreateByOwnerKey(String owner, String key, int formType) global native
bool function ReleaseByOwnerKey(String owner, String key) global native
bool function ReleaseByLocalId(int localId, String owner) global native
int function ReleaseOwner(String owner) global native

function AddMagicEffect(Form item, MagicEffect effect, float magnitude, int area, int duration, float cost) global native
function ClearMagicEffects(Form item) global native
function CopyMagicEffects(Form from, Form to) global native
function CopyAppearance(Form source, Form target) global native

function SetSpellTomeSpell(Book target, Spell teaches) global native

function SetSpellAutoCalculate(Spell spell, bool value) global native
function SetSpellCostOverride(Spell spell, int value) global native
function SetSpellChargeTime(Spell spell, float value) global native
function SetSpellCastDuration(Spell spell, int value) global native
function SetSpellRange(Spell spell, int value) global native
function SetSpellCastingPerk(Spell spell, Perk value) global native

function SetEnchantmentAutoCalculate(Enchantment enchantment, bool value) global native
function SetEnchantmentChargeOverride(Enchantment enchantment, int value) global native
function SetEnchantmentCostOverride(Enchantment enchantment, int value) global native
function SetEnchantmentChargeTime(Enchantment enchantment, float value) global native

function SetAmmoDamage(Ammo ammo, float damage) global native
function SetAmmoProjectile(Ammo ammo, Projectile projectile) global native

; None = 0, Petty = 1, Lesser = 2, Common = 3, Greater = 4, Grand = 5
function SetSoulGemCapacity(SoulGem gem, int capacity) global native
function SetSoulGemCurrentSoul(SoulGem gem, int capacity) global native
function LinkSoulGems(SoulGem empty, SoulGem filled) global native
