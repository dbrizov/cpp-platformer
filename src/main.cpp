#include <cstring>
#include <memory>

#include "engine/core/assert.h"
#include "engine/core/engine.h"
#include "engine/core/engine_config.h"
#include "engine/core/path_utils.h"
#include "engine/editor/editor.h"
#include "engine/editor/editor_config.h"

int main(int argc, char* argv[]) {
    const std::filesystem::path project_root = hob::PathUtils::resolve_project_root(argc, argv);
    HOB_CHECK(std::filesystem::exists(project_root / "scripts" / "main.lua"),
              "no game project found at '{}' (expected scripts/main.lua); pass --project <path>",
              project_root.string());
    hob::PathUtils::set_project_root(project_root);

    hob::EngineConfig config(hob::PathUtils::get_engine_config_path());

    bool editor_enabled = false;
#ifdef HOB_EDITOR
    editor_enabled = true;
#endif
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "--editor") == 0) {
            editor_enabled = true;
        }
    }

    if (editor_enabled) {
        const hob::editor::EditorConfig editor_config(hob::editor::get_editor_config_path());
        config.host_config = hob::editor::make_editor_host_config(config.graphics_config, editor_config);
    }

    hob::Engine engine(config);

    std::unique_ptr<hob::editor::Editor> editor;
    if (editor_enabled) {
        editor = std::make_unique<hob::editor::Editor>(engine);
        engine.set_hooks(editor.get());
    }

    engine.run();

    engine.set_hooks(nullptr);
    editor.reset();

    return 0;
}
