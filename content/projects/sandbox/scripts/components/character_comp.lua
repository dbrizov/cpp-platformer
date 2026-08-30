DefineComponent.CharacterComponent = {}
---@class CharacterComponent : LuaComponent
local CharacterComponent = CharacterComponent

function CharacterComponent:move(movement_input, fixed_delta_time)
    local movement = movement_input
    if movement:length_sqr() > 1.0 then
        movement = movement:normalized()
    end

    local velocity = movement * self.speed
    local character_body = self.entity:get_character_body()
    character_body:move_and_slide(velocity, fixed_delta_time)
end
