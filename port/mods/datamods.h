/* datamods.h - mod folders (datamods.c) */
#ifndef DL_DATAMODS_H
#define DL_DATAMODS_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
#define DATAMODS_MAX 64
typedef struct { char folder[96], path[1100], title[96], desc[400]; int on; } DataMod;
extern DataMod datamods[DATAMODS_MAX];
extern int datamods_n;
void datamods_init(void);
void datamods_scan(void);
void datamods_set(int i, int on);
int datamods_active(void);
int datamods_enabled(void);          /* not in "Play the original game" */
int datamods_generation(void);
void datamods_folder(char *out, size_t n);
int datamods_dirs(const char *sub, char out[][1200], int max);
const char *datamods_redirect(const char *rel, const char *game_path);
int datamods_export_cards(char *where, size_t wn);
int datamods_new(const char *name, char *where, size_t wn);
#ifdef __cplusplus
}
#endif
#endif
