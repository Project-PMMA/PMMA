#pragma once

#include <map>

#include "Internal/Rendering/Core2D/Base.hpp"

/*
Notes:
    > Internal events MUST have a default 'safe value' to return before the event manager is initialized.
*/

namespace PMMA {
class Display;
class FastRandom;
class Passport;
} // namespace PMMA

namespace PMMA::Internal {
class LoggingManager;
class AnimationManager;
class ParallelWorker;
class PowerSavingManager;
class NetworkingManager;
class SavedDataManager;
} // namespace PMMA::Internal

namespace PMMA::Internal::Events {
class InternalController;
class InternalControllerManager;
} // namespace PMMA::Internal::Events

namespace PMMA::Events {
class Controller;
}

namespace PMMA::Graphics {
class Shader;
}

namespace PMMA::Core {
extern PMMA::Display *ActiveDisplayInstance;
extern PMMA::Display *MasterDisplayInstance;

extern PMMA::Passport *PassportInstance;
extern PMMA::Internal::LoggingManager *LoggingManagerInstance;

extern PMMA::Internal::PowerSavingManager *PowerSavingManagerInstance;

extern PMMA::Internal::AnimationManager *AnimationManagerInstance;

extern PMMA::FastRandom *RandomGenerator;

extern std::vector<PMMA::Internal::Events::InternalController *> InternalControllerEventInstances;
extern std::vector<PMMA::Events::Controller *> ControllerEvent_Instances;

extern PMMA::Internal::Events::InternalControllerManager *ControllerManagerInstance;

extern std::map<std::string, PMMA::Internal::Rendering::Core2D::CompressedTextureProperty> CompressedTextureCatalogue;
extern std::map<std::string, PMMA::Internal::Rendering::Core2D::GeneratedTextureProperty> GeneratedTextureCatalogue;

extern PMMA::Internal::ParallelWorker *ParallelWorkerInstance;

extern PMMA::Graphics::Shader *Core2D_ShapeSDF_Program;

extern PMMA::Internal::NetworkingManager *NetworkingManagerInstance;
extern PMMA::Internal::SavedDataManager *SavedDataManagerInstance;
} // namespace PMMA::Core