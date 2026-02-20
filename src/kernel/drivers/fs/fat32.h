/*
 * fat32.h - FAT32 filesystem driver header
 * Simplified FAT32 implementation for KryOS
 */

#ifndef FAT32_H
#define FAT32_H

#include "../../stdint.h"

/* FAT32 configuration */
#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 11    /* 8.3 format */
#define MAX_FILES 128

/* File attributes */
#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE   0x20

/* Directory entry structure (32 bytes) */
typedef struct {
    char name[11];           /* 8.3 filename */
    uint8_t attr;            /* Attributes */
    uint8_t reserved;        /* Reserved */
    uint8_t crt_time_tenth;  /* Creation time, tenth of a second */
    uint16_t crt_time;       /* Creation time */
    uint16_t crt_date;       /* Creation date */
    uint16_t lst_acc_date;   /* Last access date */
    uint16_t fst_clus_hi;    /* High 16 bits of first cluster */
    uint16_t wrt_time;       /* Last write time */
    uint16_t wrt_date;       /* Last write date */
    uint16_t fst_clus_lo;    /* Low 16 bits of first cluster */
    uint32_t file_size;      /* File size in bytes */
} __attribute__((packed)) fat_dir_entry_t;

/* File handle structure */
typedef struct {
    char name[MAX_FILENAME_LENGTH + 1];
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t file_size;
    uint32_t position;
    uint8_t is_directory;
    uint8_t is_open;
} fat_file_t;

/* Initialize FAT32 filesystem on RAM disk */
int fat32_init(void);

/* Format RAM disk with FAT32 */
int fat32_format(void);

/* Open a file */
int fat32_open(const char *path, fat_file_t *file);

/* Close a file */
void fat32_close(fat_file_t *file);

/* Read from a file */
int fat32_read(fat_file_t *file, void *buffer, uint32_t count);

/* Write to a file */
int fat32_write(fat_file_t *file, const void *buffer, uint32_t count);

/* Create a new file */
int fat32_create(const char *path);

/* Create a directory */
int fat32_mkdir(const char *path);

/* Delete a file */
int fat32_delete(const char *path);

/* List directory contents */
int fat32_list_dir(const char *path, void (*callback)(const char *name, uint8_t attr, uint32_t size));

/* Change current directory */
int fat32_chdir(const char *path);

/* Get current directory */
const char *fat32_getcwd(void);

#endif /* FAT32_H */
