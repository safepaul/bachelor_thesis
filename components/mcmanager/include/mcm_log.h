#ifndef MCM_LOG_H
#define MCM_LOG_H

#define MCM_LOGI(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__);
#define MCM_LOGE(fmt, ...) printf("[ERR] " fmt "\n", ##__VA_ARGS__);
#define MCM_LOGW(fmt, ...) printf("[WARN] " fmt "\n", ##__VA_ARGS__);

#endif // !MCM_LOG_H

