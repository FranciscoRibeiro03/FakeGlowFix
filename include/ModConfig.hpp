#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,

    CONFIG_VALUE(enabled, bool, "Wall Frames Enabled", true);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(enabled);
    )

);