#include "main.hpp"

#include "GlobalNamespace/BeatmapObjectsInstaller.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ConditionalActivation.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/MeshFilter.hpp"

#include "qosmetics-walls/shared/API.hpp"

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

MAKE_HOOK_MATCH(BeatmapObjectsInstaller_InstallBindings, &GlobalNamespace::BeatmapObjectsInstaller::InstallBindings, void, GlobalNamespace::BeatmapObjectsInstaller* self) 
{

    std::optional<bool> qosDisabledOptional = Qosmetics::Walls::API::GetWallsDisabled();
    bool qosDisabled = qosDisabledOptional.value_or(true);

    if (qosDisabled) {

        auto obstaclePrefab = self->obstaclePrefab;

        auto t = obstaclePrefab ? obstaclePrefab->get_transform() : nullptr;

        auto hideWrapperT = t ? t->Find(HideWrapper) : nullptr;
        auto obstacleFrameT = hideWrapperT ? hideWrapperT->Find(ObstacleFrame) : nullptr;

        auto conditionalActivation = obstacleFrameT->get_gameObject()->GetComponent<GlobalNamespace::ConditionalActivation*>();
        conditionalActivation->activateOnFalse = true;

    }

    BeatmapObjectsInstaller_InstallBindings(self);

}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load();
    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(getLogger(), BeatmapObjectsInstaller_InstallBindings);
    getLogger().info("Installed all hooks!");
}