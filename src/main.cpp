#include "main.hpp"
#include "ModConfig.hpp"

#include "GlobalNamespace/BeatmapObjectsInstaller.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ConditionalActivation.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/MeshFilter.hpp"

#include "UnityEngine/Resources.hpp"

#include "UnityEngine/UI/VerticalLayoutGroup.hpp"

#include "HMUI/ViewController.hpp"

#include "questui/shared/QuestUI.hpp"

#include "qosmetics-walls/shared/API.hpp"

#define MAKE_CLONE_AND_PARENT(identifier)                                                               \
    static UnityEngine::GameObject* parent_clone_##identifier = nullptr;                                \
    if (parent_clone_##identifier)                                                                      \
    {                                                                                                   \
        UnityEngine::Object::DestroyImmediate(parent_clone_##identifier);                               \
    }                                                                                                   \
    static ConstString parent_name_##identifier("parent_clone_" #identifier);                           \
    parent_clone_##identifier = UnityEngine::GameObject::New_ctor(parent_name_##identifier);            \
    parent_clone_##identifier->SetActive(false);                                                        \
    UnityEngine::Object::DontDestroyOnLoad(parent_clone_##identifier);                                  \
    UnityEngine::Object::Instantiate(identifier, parent_clone_##identifier->get_transform());           \
    auto type_##identifier = il2cpp_utils::GetSystemType(il2cpp_utils::ExtractType(identifier->klass)); \
    auto clone_##identifier = parent_clone_##identifier->GetComponentsInChildren(type_##identifier)[0]; \
    clone_##identifier->get_gameObject()->set_name(identifier->get_name());                             \
    reinterpret_cast<UnityEngine::Component*&>(self->identifier) = clone_##identifier;                  \
    self->identifier->get_gameObject()->SetActive(true)

DEFINE_CONFIG(ModConfig)

ConstString HideWrapper("HideWrapper");
ConstString ObstacleFrame("ObstacleFrame");

static ModInfo modInfo;

Configuration& getConfig() {
    static Configuration config(modInfo);
    return config;
}

Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (!firstActivation) return;
    auto vertical = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(self->get_transform());
    AddConfigValueToggle(vertical->get_transform(), getModConfig().enabled);
}

MAKE_HOOK_MATCH(BeatmapObjectsInstaller_InstallBindings, &GlobalNamespace::BeatmapObjectsInstaller::InstallBindings, void, GlobalNamespace::BeatmapObjectsInstaller* self) 
{

    std::optional<bool> qosDisabledOptional = Qosmetics::Walls::API::GetWallsDisabled();
    bool qosDisabled = qosDisabledOptional.value_or(true);

    auto obstaclePrefab = self->obstaclePrefab;

    if (qosDisabled) {
        MAKE_CLONE_AND_PARENT(obstaclePrefab);

        auto t = obstaclePrefab ? obstaclePrefab->get_transform() : nullptr;

        auto hideWrapperT = t ? t->Find(HideWrapper) : nullptr;
        auto obstacleFrameT = hideWrapperT ? hideWrapperT->Find(ObstacleFrame) : nullptr;

        auto conditionalActivation = obstacleFrameT->get_gameObject()->GetComponent<GlobalNamespace::ConditionalActivation*>();
        conditionalActivation->activateOnFalse = getModConfig().enabled.GetValue();
    }

    BeatmapObjectsInstaller_InstallBindings(self);

    if (qosDisabled) self->obstaclePrefab = obstaclePrefab;

}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load();
    getModConfig().Init(modInfo);
    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), BeatmapObjectsInstaller_InstallBindings);
    getLogger().info("Installed all hooks!");

    getLogger().info("Registering UI...");
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);
    getLogger().info("Registered UI!");
}