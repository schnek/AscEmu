/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "MovementCodec.hpp"

#if VERSION_STRING == Classic
    using ActiveMovementCodec = MovementCodec<ClientVersion::_Classic>;
#elif VERSION_STRING == TBC
    using ActiveMovementCodec = MovementCodec<ClientVersion::_TBC>;
#elif VERSION_STRING == WotLK
    using ActiveMovementCodec = MovementCodec<ClientVersion::_WotLK>;
#elif VERSION_STRING == Cata
    using ActiveMovementCodec = MovementCodec<ClientVersion::_Cata>;
#elif VERSION_STRING == Mop
    using ActiveMovementCodec = MovementCodec<ClientVersion::_Mop>;
#endif
