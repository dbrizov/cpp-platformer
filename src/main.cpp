#include <cstring>

#include "engine/core/assert.h"
#include "engine/core/engine.h"
#include "engine/core/engine_config.h"
#include "engine/core/path_utils.h"
#include "engine/editor/editor_config.h"

int main(int argc, char* argv[]) {
    const std::filesystem::path project_root = hob::PathUtils::resolve_project_root(argc, argv);
    HOB_CHECK(std::filesystem::exists(project_root / "scripts" / "main.lua"),
              "no game project found at '{}' (expected scripts/main.lua); pass --project <path>",
              project_root.string());
    hob::PathUtils::set_project_root(project_root);

    const hob::EngineConfig config(hob::PathUtils::get_engine_config_path());

    bool editor_enabled = false;
#ifdef HOB_EDITOR
    editor_enabled = true;
#endif
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--editor") == 0) {
            editor_enabled = true;
        }
    }

    hob::EditorConfig editor_config;
    if (editor_enabled) {
        editor_config = hob::EditorConfig(hob::PathUtils::get_editor_config_path());
        editor_config.enabled = true;
    }

    hob::Engine engine(config, editor_config);

    engine.run();
    return 0;
}
