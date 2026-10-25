/* playlist.h - the soundtrack's player in F12 > Settings > Sound (mods/playlist.c) */
#ifndef DL_PLAYLIST_H
#define DL_PLAYLIST_H
#ifdef __cplusplus
extern "C" {
#endif
void playlist_play(const char *dev, int n);      /* piece n through device dev ("mt32", "fm" ...) */
void playlist_pause(int on);
void playlist_stop(void);
void playlist_seek(int ms);
int playlist_now(void);                          /* the piece under way, -1 none */
int playlist_paused(void);
int playlist_measuring(void);                    /* run ahead to find its length (silent) */
int playlist_pos_ms(void);
int playlist_length_ms(int n);                   /* on the device last played: -1 not known yet, -2 it repeats (no length found) */
int playlist_length_dev_ms(const char *dev, int n);   /* the same on device dev (before anything is played) */
void playlist_frame(int settings_shown);
#ifdef __cplusplus
}
#endif
#endif
