/* sfx.h - battle sound effects (sfx.c) */
#ifndef DL_SFX_H
#define DL_SFX_H
#ifdef __cplusplus
extern "C" {
#endif
extern int sfx_mode, sfx_volume, sfx_added_hits, sfx_synth, SFX_COUNT;
void sfx_set_synth(int on);
extern const char *SFX_NAMES[], *SFX_DESC[];
void sfx_set(int mode);
void sfx_set_volume(int v);
void sfx_set_hits(int on);
void sfx_reload(void);
int sfx_user_count(void);
int sfx_mod_count(void);              /* sounds an enabled mod gives (they play without sound.effects) */
int sfx_export(char *msg, int n);
void sfx_mix(short *out, int frames);
#ifdef __cplusplus
}
#endif
#endif
