/*
 * ramdisk.h - RAM disk driver header
 * Provides a virtual disk in memory for FAT32 filesystem
 */

#ifndef RAMDISK_H
#define RAMDISK_H

#include "../../stdint.h"

/* RAM disk configuration - 1MB to avoid memory issues */
#define RAMDISK_SIZE (1 * 1024 * 1024)  /* 1 MB RAM disk */
#define RAMDISK_SECTOR_SIZE 512

/* Initialize RAM disk */
int ramdisk_init(void);

/* Read sectors from RAM disk */
int ramdisk_read(uint32_t sector, uint32_t count, void *buffer);

/* Write sectors to RAM disk */
int ramdisk_write(uint32_t sector, uint32_t count, const void *buffer);

/* Get RAM disk size in sectors */
uint32_t ramdisk_get_size_sectors(void);

/* Get pointer to RAM disk memory (for direct access) */
uint8_t *ramdisk_get_ptr(void);

#endif /* RAMDISK_H */
