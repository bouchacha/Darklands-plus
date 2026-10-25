/* musicpack.h - music packs (musicpack.c) */
#ifndef DL_MUSICPACK_H
#define DL_MUSICPACK_H
#ifdef __cplusplus
extern "C" {
#endif
extern int musicpack_on;
void musicpack_init(void);
void musicpack_reload(void);
void musicpack_set(int on);
int musicpack_count(void);
const char *musicpack_file(int n);
int musicpack_versions(int n);
const char *musicpack_from(int n);
const char *musicpack_desc(int n);
int musicpack_loops(int n);
int musicpack_now(int *from_pack);
int musicpack_still_playing(void);          /* the piece started last has not yet come to its end */
void musicpack_cmd(int n);
void musicpack_driver(const char *filename);
void musicpack_mix(short *out, int frames);
int musicpack_mutes(void);
#ifdef __cplusplus
}
#endif
#endif
