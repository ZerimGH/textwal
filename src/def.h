#ifndef DEF_H
#define DEF_H
// #define PERROR(fmt, ...) fprintf(stderr, "%s:%d(%s): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define PERROR(fmt, ...) fprintf(stderr, "textwal: " fmt, ##__VA_ARGS__)
#define PINFO(fmt, ...) fprintf(stdout, "(%s): " fmt, __func__, ##__VA_ARGS__)
#endif
