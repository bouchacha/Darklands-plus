#ifndef DL_CONFIG_H
#define DL_CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif
void config_load(const char *dir);
void config_save(void);
const char *config_get(const char *key, const char *def);
int config_get_int(const char *key, int def);
void config_set(const char *key, const char *val);
void config_set_int(const char *key, int val);
#ifdef __cplusplus
}
#endif
#endif
