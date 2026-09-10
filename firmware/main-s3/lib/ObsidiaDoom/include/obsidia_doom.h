#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void obsidia_doom_start(void);
bool obsidia_doom_wad_present(void);

/* Platform hooks called by the C engine. */
void obsidia_doom_present(const unsigned char *pixels, const short *palette);
int obsidia_doom_poll_key(void);

#ifdef __cplusplus
}
#endif
