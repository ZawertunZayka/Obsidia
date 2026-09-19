#pragma once

#ifdef OBSIDIA_V1

#include "RadioService.hpp"

namespace obsidia {

void beginRadioRuntime();
void pollRadioRuntime();
const RadioService::Snapshot &radioRuntimeSnapshot();

} // namespace obsidia

#endif
