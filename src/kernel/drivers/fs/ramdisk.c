/*
 * ramdisk.c - RAM disk driver implementation
 * Provides a virtual disk in memory for FAT32 filesystem
 */

#include "ramdisk.h"
#include "../../string.h"

/* RAM disk memory - 4MB static allocation */
static uint8_t ramdisk_memory[RAMDISK_SIZE];

/*
 * Initialize RAM disk
 */
int ramdisk_init(void) {
    /* Clear RAM disk memory */
    memset(ramdisk_memory, 0, RAMDISK_SIZE);
    return 0;
}

/*
 * Read sectors from RAM disk
 */
int ramdisk_read(uint32_t sector, uint32_t count, void *buffer) {
    uint32_t offset = sector * RAMDISK_SECTOR_SIZE;
    uint32_t size = count * RAMDISK_SECTOR_SIZE;
    
    /* Bounds check */
    if (offset + size > RAMDISK_SIZE) {
        return -1;
    }
    
    /* Copy data to buffer */
    memcpy(buffer, &ramdisk_memory[offset], size);
    return 0;
}

/*
 * Write sectors to RAM disk
 */
int ramdisk_write(uint32_t sector, uint32_t count, const void *buffer) {
    uint32_t offset = sector * RAMDISK_SECTOR_SIZE;
    uint32_t size = count * RAMDISK_SECTOR_SIZE;
    
    /* Bounds check */
    if (offset + size > RAMDISK_SIZE) {
        return -1;
    }
    
    /* Copy data from buffer */
    memcpy(&ramdisk_memory[offset], buffer, size);
    return 0;
}

/*
 * Get RAM disk size in sectors
 */
uint32_t ramdisk_get_size_sectors(void) {
    return RAMDISK_SIZE / RAMDISK_SECTOR_SIZE;
}

/*
 * Get pointer to RAM disk memory (for direct access)
 */
uint8_t *ramdisk_get_ptr(void) {
    return ramdisk_memory;
}
