#include "Plugin.h"

#include <utility>

#include <ll/api/command/DynamicCommand.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/service/Bedrock.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/world/actor/player/Player.h>

namespace plugin {

Plugin::~Plugin() = default;

static std::unique_ptr<Plugin> plugin{};

Plugin& Plugin::getInstance() { return *plugin; }

Plugin::Plugin(ll::plugin::NativePlugin& self) : mSelf(self) {
    mSelf.getLogger().info("loading...");

    // Code for loading the plugin goes here.
}

ll::plugin::NativePlugin& Plugin::getSelf() const { return mSelf; }

bool Plugin::enable() {
    auto& logger = mSelf.getLogger();

    // ...

    // Register commands.
    auto commandRegistry = ll::service::getCommandRegistry();
    if (!commandRegistry) {
        throw std::runtime_error("failed to get command registry");
    }

    auto command =
        DynamicCommand::createCommand(commandRegistry, "suicide", "Commits suicide.", CommandPermissionLevel::Any);
    command->addOverload();
    command->setCallback(
        [&logger](DynamicCommand const&, CommandOrigin const& origin, CommandOutput& output, std::unordered_map<std::string, DynamicCommand::Result>&) {
            auto* entity = origin.getEntity();
            if (entity == nullptr || !entity->isType(ActorType::Player)) {
                output.error("Only players can commit suicide");
                return;
            }

            auto* player = static_cast<Player*>(entity);
            player->kill();

            logger.info("{} killed themselves", player->getRealName());
        }
    );
    DynamicCommand::setup(commandRegistry, std::move(command));

    // ...

    return true;
}

bool Plugin::disable() {
    mSelf.getLogger().info("disabling...");

    // Unregister commands.
    auto commandRegistry = ll::service::getCommandRegistry();
    if (!commandRegistry) {
        throw std::runtime_error("failed to get command registry");
    }

    commandRegistry->unregisterCommand("suicide");

    return true;
}

extern "C" {
_declspec(dllexport) bool ll_plugin_load(ll::plugin::NativePlugin& self) {
    plugin = std::make_unique<plugin::Plugin>(self);
    return true;
}

/// @warning Unloading the plugin may cause a crash if the plugin has not released all of its
/// resources. If you are unsure, keep this function commented out.
// _declspec(dllexport) bool ll_plugin_unload(ll::plugin::Plugin&) {
//     plugin.reset();
//
//     return true;
// }

_declspec(dllexport) bool ll_plugin_enable(ll::plugin::NativePlugin&) { return plugin->enable(); }

_declspec(dllexport) bool ll_plugin_disable(ll::plugin::NativePlugin&) { return plugin->disable(); }
}

} // namespace plugin
