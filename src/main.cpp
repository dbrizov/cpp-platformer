#include <cstring>

#include "engine/core/assert.h"
#include "engine/core/engine.h"
#include "engine/core/engine_config.h"
#include "engine/core/path_utils.h"

int main(int argc, char* argv[]) {
    const std::filesystem::path project_root = hob::PathUtils::resolve_project_root(argc, argv);
    HOB_CHECK(std::filesystem::exists(project_root / "scripts" / "main.lua"),
              "no game project found at '{}' (expected scripts/main.lua); pass --project <path>",
              project_root.string());
    hob::PathUtils::set_project_root(project_root);

    hob::EngineConfig config(hob::PathUtils::get_engine_config_path());
#ifdef HOB_EDITOR
    config.editor_enabled = true; // editor build: boot into the editor without needing --editor
#endif
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--editor") == 0) {
            config.editor_enabled = true;
        }
    }

    hob::Engine engine(config);

    engine.run();
    return 0;
}
