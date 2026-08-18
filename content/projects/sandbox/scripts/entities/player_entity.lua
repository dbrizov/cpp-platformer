DefineEntity.Player = {
    ticking = true,
    input = {},
    character_body = {
        collision_layer = CollisionLayer.Kinematic,
        collision_mask = CollisionLayer.Static | CollisionLayer.Dynamic | CollisionLayer.Trigger,
        solver_ignore_mask = CollisionLayer.Trigger,
        capsule = Capsule(Vector2.zero(), Vector2.zero(), 1.2),
    },
    sprite = {
        texture = Textures.PlayerIdle01,
        material = Materials.Distort,
        z_index = 1,
    },
    sprite_animator = {
        clips = {
            idle = AnimationClips.PlayerIdle,
            run = AnimationClips.PlayerRun,
        },
        default_clip = "idle",
    },
    audio = {
        clip = AudioClips.Whoosh,
    },
    lua_components = { Components.Player },
}
