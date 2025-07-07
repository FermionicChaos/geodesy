#pragma once
#ifndef GEODESY_CORE_PHYS_FORCE_H
#define GEODESY_CORE_PHYS_FORCE_H

// Include include config.
#include "../../config.h"
// Include include math.
#include "../math.h"

namespace geodesy::core::phys {

	struct force {
		math::vec<float, 3> Position;   // [m] Position where force is applied
		math::vec<float, 3> Magnitude;  // [N] Magnitude of the force
	};

}

#endif // !GEODESY_CORE_PHYS_FORCE_H
