#pragma once

#include <agz/editor/common.h>

AGZ_EDITOR_BEGIN

enum class AssetSectionType
{
    MaterialPool  = 0,
    MediumPool    = 1,
    GeometryPool  = 2,
    Texture2DPool = 3,
    Texture3DPool = 4,

    Entities      = 6,
    EnvirLight    = 7
};

AGZ_EDITOR_END