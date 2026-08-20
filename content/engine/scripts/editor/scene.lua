_G.Editor = _G.Editor or {}

local scene_state = _G.__editor_scene_state or {
    name = nil,
    is_dirty = false,
    instance_count = 0,
    next_instance_id = 1,
    instance_id_by_index = {},
    instance_id_by_entity_id = {},
    entity_id_by_instance_id = {},
    entity_def_by_instance_id = {},
}
_G.__editor_scene_state = scene_state

local function alloc_instance_id()
    local instance_id = scene_state.next_instance_id
    scene_state.next_instance_id = instance_id + 1

    return instance_id
end

---@return string[]
function Editor.get_scene_names()
    local names = {}
    for name in pairs(_G.__scene_registry) do
        names[#names + 1] = name
    end

    table.sort(names)

    return names
end

---@return string|nil
function Editor.get_current_scene()
    return scene_state.name
end

---@return boolean
function Editor.is_scene_dirty()
    return scene_state.is_dirty
end

function Editor.mark_scene_dirty()
    scene_state.is_dirty = true
end

function Editor.mark_scene_saved()
    scene_state.is_dirty = false
end

function Editor.clear_world()
    scene_state.instance_count = 0
    scene_state.instance_id_by_index = {}
    scene_state.instance_id_by_entity_id = {}
    scene_state.entity_id_by_instance_id = {}
    scene_state.entity_def_by_instance_id = {}

    _G.__scene_overrides_by_entity_id = {}
    EntitySpawner.clear()
end

function Editor.load_scene()
    local name = scene_state.name
    if name == nil then
        return
    end

    local scene_def = _G.__scene_registry[name]
    if scene_def == nil then
        Log.error("Editor.load_scene: scene '" .. tostring(name) .. "' is no longer registered")
        scene_state.name = nil
        return
    end

    local spawned = Scene.load(name)
    if spawned == nil then
        return
    end

    scene_state.instance_count = #scene_def.entities

    for _, entry in ipairs(spawned) do
        local inst = scene_def.entities[entry.index]
        local instance_id = inst.__instance_id or alloc_instance_id()
        local entity_id = entry.entity:get_id()

        inst.__instance_id = instance_id
        scene_state.instance_id_by_index[entry.index] = instance_id
        scene_state.instance_id_by_entity_id[entity_id] = instance_id
        scene_state.entity_id_by_instance_id[instance_id] = entity_id
        scene_state.entity_def_by_instance_id[instance_id] = inst
    end
end

---@param name string
---@return boolean
function Editor.open_scene(name)
    if _G.__scene_registry[name] == nil then
        Log.error("Editor.open_scene: scene '" .. tostring(name) .. "' is not registered")
        scene_state.name = nil
        return false
    end

    Editor.clear_world()
    scene_state.name = name
    scene_state.is_dirty = false
    Editor.load_scene()

    return true
end

function Editor.reload_scene()
    Editor.clear_world()
    Editor.load_scene()
end

---@param instance_id integer
---@return integer|nil
function Editor.get_entity_id(instance_id)
    return scene_state.entity_id_by_instance_id[instance_id]
end

---@param entity_id integer
---@return integer|nil
function Editor.get_instance_id(entity_id)
    return scene_state.instance_id_by_entity_id[entity_id]
end

---@param instance_id integer
---@return table|nil
function Editor.get_instance_def(instance_id)
    return scene_state.entity_def_by_instance_id[instance_id]
end

-- Re-points the orphaned defs at the reloaded document; false means the caller must respawn instead.
---@return boolean
function Editor.rebind_instance_defs()
    local name = scene_state.name
    if name == nil then
        return true
    end

    local scene_def = _G.__scene_registry[name]
    if scene_def == nil then
        return false
    end

    local fresh = scene_def.entities
    if #fresh ~= scene_state.instance_count then
        return false
    end

    for index, inst in ipairs(fresh) do
        local instance_id = scene_state.instance_id_by_index[index]
        local old = instance_id and scene_state.entity_def_by_instance_id[instance_id]
        if old == nil or old.prefab ~= inst.prefab then
            return false
        end
    end

    for index, inst in ipairs(fresh) do
        local instance_id = scene_state.instance_id_by_index[index]
        inst.__instance_id = instance_id
        scene_state.entity_def_by_instance_id[instance_id] = inst
    end

    return true
end
