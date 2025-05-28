#pragma once
#include <Engine/Asset/Asset.h>

enum class EPhysicsNodeType : uint8
{
    None,
    Bone,
    Constraint,
    Primitive,
    Body
};