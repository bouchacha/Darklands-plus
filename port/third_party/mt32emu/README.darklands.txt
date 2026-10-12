libmt32emu 2.8.3 (the Munt project, https://github.com/munt/munt, commit 6e7c01f,
mt32emu/src), LGPL 2.1 or later: an emulation of the Roland MT-32, CM-32L and LAPC-I.

Taken as it is, without: test/, the libsoxr and libsamplerate adapters (the built-in
resampler is used), VersionTagging.cpp, config.h.in (config.h is filled in by hand for a
static build) and mt32emu.pc.in. Darklands+ uses it through runtime/mt32.c.

The ROM images of the synthesizers are Roland's and are not part of this library or of
Darklands+: the player's own files are used.
