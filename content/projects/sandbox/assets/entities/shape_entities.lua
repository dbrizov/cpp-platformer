DefineEntity.StaticBox = {
    transform = {
        scale = Vector2(2, 2),
    },
    rigidbody = {},
    box_collider = {
        collision_layer = CollisionLayer.Static,
        collision_mask = CollisionLayer.Static | CollisionLayer.Dynamic | CollisionLayer.Kinematic,
    },
    sprite = {
        texture = Textures.WhiteRect,
        material = Material {
            tint = Color.orange(),
        },
    },
}

DefineEntity.DynamicBox = {
    rigidbody = {
        body_type = BodyType.Dynamic,
    },
    box_collider = {
        collision_layer = CollisionLayer.Dynamic,
        collision_mask = CollisionLayer.Static | CollisionLayer.Dynamic | CollisionLayer.Kinematic | CollisionLayer.Trigger,
    },
    sprite = {
        texture = Textures.WhiteRect,
        material = Material {
            tint = Color.green(),
        },
    },
}

DefineEntity.TriggerBox = {
    rigidbody = {},
    box_collider = {
        trigger = true,
        collision_layer = CollisionLayer.Trigger,
        collision_mask = CollisionLayer.Static | CollisionLayer.Dynamic | CollisionLayer.Kinematic,
    },
    sprite = {
        texture = Textures.WhiteRect,
        material = Material {
            tint = Color.cyan(),
        },
    },
}

DefineEntity.StaticCircle = {
    transform = {
        scale = Vector2(2, 2),
    },
    rigidbody = {},
    circle_collider = {
        collision_layer = CollisionLayer.Static,
        collision_mask = CollisionLayer.Static | CollisionLayer.Dynamic | CollisionLayer.Kinematic,
    },
}

DefineEntity.DynamicCircle = {
    rigidbody = {
        body_type = BodyType.Dynamic,
    },
    circle_collider = {
        collision_layer = CollisionLayer.Dynamic,
        collision_mask = CollisionLayer.Static | CollisionLayer.Dynamic | CollisionLayer.Kinematic | CollisionLayer.Trigger,
    },
}

DefineEntity.TriggerCircle = {
    transform = {
        scale = Vector2(2, 2),
    },
    rigidbody = {},
    circle_collider = {
        trigger = true,
        collision_layer = CollisionLayer.Trigger,
        collision_mask = CollisionLayer.Static | CollisionLayer.Dynamic | CollisionLayer.Kinematic,
    },
}
