#pragma once
#include "Core/HAL/PlatformType.h"

namespace EEndPlayReason
{
    enum Type : uint8
    {
        /** 명시적인 삭제가 일어났을 때, Destroy()등 */
        Destroyed,
        /** World가 바뀌었을 때 */
        WorldTransition,
        /** 프로그램을 종료했을 때 */
        Quit,
    };
}
namespace EWorldType
{
    enum Type
    {
        /** An untyped world, in most cases this will be the vestigial worlds of streamed in sub-levels */
        None,

        /** The game world */
        Game,

        /** A world being edited in the editor */
        Editor,

        /** A Play In Editor world */
        PIE,

        /** A preview world for an editor tool */
        EditorPreview,

        /** A preview world for a game */
        GamePreview,

        /** A minimal RPC world for a game */
        GameRPC,

        /** An editor world that was loaded but not currently being edited in the level editor */
        Inactive
    };
}

namespace ECollisionEnabled 
{ 
	enum Type : int
	{ 
	    /** 물리 엔진에 어떠한 표현도 생성하지 않습니다. 공간 쿼리(raycasts, sweeps, overlaps)나 시뮬레이션(강체, 제약)에 사용할 수 없습니다. (특히 움직이는 객체에서) 최고의 성능을 제공합니다. */
	    NoCollision,
        /** 공간 쿼리(raycasts, sweeps, overlaps)에만 사용됩니다. 시뮬레이션(강체, 제약)에는 사용할 수 없습니다. 물리 시뮬레이션이 필요 없는 캐릭터 이동 등에 유용합니다. 데이터가 시뮬레이션 트리에서 제외되어 성능이 향상됩니다. */
        QueryOnly,
        /** 물리 시뮬레이션(강체, 제약)에만 사용됩니다. 공간 쿼리(raycasts, sweeps, overlaps)에는 사용할 수 없습니다. 본 단위 감지가 필요 없는 흔들리는 부분 등에 유용합니다. 쿼리 트리에서 제외되어 성능이 향상됩니다. */
        PhysicsOnly,
        /** 공간 쿼리(raycasts, sweeps, overlaps)와 시뮬레이션(강체, 제약) 모두에 사용할 수 있습니다. */
        QueryAndPhysics,
        /** 물리 시뮬레이션(강체, 제약)을 탐색하기 위해서만 사용됩니다. 공간 쿼리(raycasts, sweeps, overlaps)에는 사용할 수 없습니다. 물리 상호작용을 탐지해 충돌 데이터를 히트 콜백이나 컨택트 수정에 전달하지만, 실제 물리 반응은 원치 않을 때 유용합니다. */
        ProbeOnly,
        /** 공간 쿼리(raycasts, sweeps, overlaps)와 물리 시뮬레이션 탐색(강체, 제약) 모두에 사용할 수 있습니다. 실제 물리 상호작용은 허용되지 않지만, 충돌 데이터를 생성하고 히트 콜백을 트리거하며 컨택트 수정에 충돌이 나타납니다. */
        QueryAndProbe
	}; 
} 