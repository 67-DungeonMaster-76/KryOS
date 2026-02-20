/*
 * fat32.c - FAT32 filesystem driver implementation
 * Simplified FAT32 implementation for KryOS
 * No division operations to avoid division by zero exceptions
 */

#include "fat32.h"
#include "ramdisk.h"
#include "../../string.h"
#include "../video/fb_console.h"

/* FAT32 Boot Sector structure */
typedef struct {
    uint8_t jmp_boot[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    /* FAT32 extended */
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_num;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed)) fat_boot_sector_t;

/* Filesystem constants - fixed values to avoid division */
#define SECTORS_PER_CLUSTER 8
#define CLUSTER_SIZE 4096
#define ENTRIES_PER_CLUSTER 128

/* Filesystem state */
static fat_boot_sector_t boot_sector;
static uint32_t fat_start_sector;
static uint32_t data_start_sector;
static uint32_t root_cluster;
static uint32_t current_dir_cluster;
static char current_path[MAX_PATH_LENGTH];

/* Sector buffer */
static uint8_t sector_buffer[512];

/* Cluster buffer */
static uint8_t cluster_buffer[CLUSTER_SIZE];

/*
 * Read a sector from disk
 */
static int read_sector(uint32_t sector, void *buffer) {
    return ramdisk_read(sector, 1, buffer);
}

/*
 * Write a sector to disk
 */
static int write_sector(uint32_t sector, const void *buffer) {
    return ramdisk_write(sector, 1, buffer);
}

/*
 * Get FAT entry for a cluster
 */
static uint32_t get_fat_entry(uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat_start_sector + (fat_offset >> 9);  /* >> 9 = / 512 */
    uint32_t fat_offset_in_sector = fat_offset & 511;  /* & 511 = % 512 */
    
    read_sector(fat_sector, sector_buffer);
    uint32_t *fat = (uint32_t *)sector_buffer;
    return fat[fat_offset_in_sector >> 2] & 0x0FFFFFFF;  /* >> 2 = / 4 */
}

/*
 * Set FAT entry for a cluster
 */
static void set_fat_entry(uint32_t cluster, uint32_t value) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat_start_sector + (fat_offset >> 9);
    uint32_t fat_offset_in_sector = fat_offset & 511;
    
    read_sector(fat_sector, sector_buffer);
    uint32_t *fat = (uint32_t *)sector_buffer;
    fat[fat_offset_in_sector >> 2] = (fat[fat_offset_in_sector >> 2] & 0xF0000000) | (value & 0x0FFFFFFF);
    write_sector(fat_sector, sector_buffer);
}

/*
 * Find a free cluster
 */
static uint32_t find_free_cluster(void) {
    uint32_t cluster;
    uint32_t max_cluster = boot_sector.total_sectors_32 >> 3;  /* >> 3 = / 8 */
    
    for (cluster = 2; cluster < max_cluster; cluster++) {
        if (get_fat_entry(cluster) == 0) {
            return cluster;
        }
    }
    return 0;
}

/*
 * Convert cluster to sector
 */
static uint32_t cluster_to_sector(uint32_t cluster) {
    return data_start_sector + ((cluster - 2) << 3);  /* << 3 = * 8 */
}

/*
 * Read a cluster
 */
static int read_cluster(uint32_t cluster, void *buffer) {
    uint32_t sector = cluster_to_sector(cluster);
    int i;
    for (i = 0; i < SECTORS_PER_CLUSTER; i++) {
        if (read_sector(sector + i, (uint8_t*)buffer + (i << 9)) != 0) {
            return -1;
        }
    }
    return 0;
}

/*
 * Write a cluster
 */
static int write_cluster(uint32_t cluster, const void *buffer) {
    uint32_t sector = cluster_to_sector(cluster);
    int i;
    for (i = 0; i < SECTORS_PER_CLUSTER; i++) {
        if (write_sector(sector + i, (const uint8_t*)buffer + (i << 9)) != 0) {
            return -1;
        }
    }
    return 0;
}

/*
 * Format filename to 8.3 format
 */
static void format_filename(const char *name, char *output) {
    int i = 0;
    
    /* Clear output with spaces */
    for (i = 0; i < 11; i++) {
        output[i] = ' ';
    }
    output[11] = '\0';
    
    /* Handle special directory entries "." and ".." */
    if (name[0] == '.' && name[1] == '\0') {
        output[0] = '.';
        return;
    }
    if (name[0] == '.' && name[1] == '.' && name[2] == '\0') {
        output[0] = '.';
        output[1] = '.';
        return;
    }
    
    /* Copy name part (before dot) */
    i = 0;
    while (name[i] && name[i] != '.' && i < 8) {
        output[i] = name[i];
        if (output[i] >= 'a' && output[i] <= 'z') {
            output[i] -= 32;
        }
        i++;
    }
    
    /* Skip to extension */
    while (name[i] && name[i] != '.') {
        i++;
    }
    if (name[i] == '.') {
        i++;
        int j = 8;
        while (name[i] && j < 11) {
            output[j] = name[i];
            if (output[j] >= 'a' && output[j] <= 'z') {
                output[j] -= 32;
            }
            i++;
            j++;
        }
    }
}

/*
 * Parse filename from 8.3 format
 */
static void parse_filename(const char *fat_name, char *output) {
    int i, j = 0;
    
    for (i = 0; i < 8 && fat_name[i] != ' '; i++) {
        output[j++] = fat_name[i];
    }
    
    if (fat_name[8] != ' ') {
        output[j++] = '.';
        for (i = 8; i < 11 && fat_name[i] != ' '; i++) {
            output[j++] = fat_name[i];
        }
    }
    output[j] = '\0';
}

/*
 * Compare two filenames case-insensitively
 */
static int filename_cmp(const char *a, const char *b) {
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return *a - *b;
}

/*
 * Find directory entry by name
 */
static int find_dir_entry(uint32_t dir_cluster, const char *name, fat_dir_entry_t *entry, uint32_t *entry_cluster, uint32_t *entry_offset) {
    char fat_name[13];
    char formatted_name[12];
    uint32_t cluster = dir_cluster;
    int i;
    
    format_filename(name, formatted_name);
    
    while (cluster >= 2 && cluster < 0x0FFFFFF8) {
        read_cluster(cluster, cluster_buffer);
        
        for (i = 0; i < ENTRIES_PER_CLUSTER; i++) {
            fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)&cluster_buffer[i * 32];
            
            if ((uint8_t)dir_entry->name[0] == 0x00) {
                return -1;
            }
            
            if ((uint8_t)dir_entry->name[0] == 0xE5) {
                continue;
            }
            
            parse_filename(dir_entry->name, fat_name);
            if (filename_cmp(fat_name, name) == 0 || 
                memcmp(dir_entry->name, formatted_name, 11) == 0) {
                if (entry) *entry = *dir_entry;
                if (entry_cluster) *entry_cluster = cluster;
                if (entry_offset) *entry_offset = i * 32;
                return 0;
            }
        }
        
        cluster = get_fat_entry(cluster);
    }
    
    return -1;
}

/*
 * Find free directory entry
 */
static int find_free_dir_entry(uint32_t dir_cluster, uint32_t *entry_cluster, uint32_t *entry_offset) {
    uint32_t cluster = dir_cluster;
    int i;
    
    while (cluster >= 2 && cluster < 0x0FFFFFF8) {
        read_cluster(cluster, cluster_buffer);
        
        for (i = 0; i < ENTRIES_PER_CLUSTER; i++) {
            fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)&cluster_buffer[i * 32];
            
            if ((uint8_t)dir_entry->name[0] == 0x00 || (uint8_t)dir_entry->name[0] == 0xE5) {
                if (entry_cluster) *entry_cluster = cluster;
                if (entry_offset) *entry_offset = i * 32;
                return 0;
            }
        }
        
        cluster = get_fat_entry(cluster);
    }
    
    return -1;
}

/*
 * Initialize FAT32 filesystem
 */
int fat32_init(void) {
    /* Read boot sector */
    if (read_sector(0, &boot_sector) != 0) {
        return -1;
    }
    
    /* Check if already formatted */
    if (boot_sector.boot_signature != 0x29 && boot_sector.boot_signature != 0x28) {
        /* Not formatted, format it now */
        if (fat32_format() != 0) {
            return -1;
        }
    }
    
    /* Calculate filesystem positions */
    fat_start_sector = boot_sector.reserved_sectors;
    data_start_sector = fat_start_sector + boot_sector.num_fats * boot_sector.fat_size_32;
    root_cluster = boot_sector.root_cluster;
    current_dir_cluster = root_cluster;
    
    /* Set current path to root */
    strcpy(current_path, "/");
    
    return 0;
}

/*
 * Format RAM disk with FAT32
 */
int fat32_format(void) {
    uint32_t total_sectors = ramdisk_get_size_sectors();
    uint32_t reserved_sectors = 32;
    uint32_t num_fats = 2;
    
    /* Simple FAT size calculation: ~1% of data area for FAT */
    /* For 4MB disk: 8192 sectors, FAT size ~80 sectors */
    uint32_t fat_size = 80;  /* Fixed size for simplicity */
    
    /* Clear boot sector */
    memset(&boot_sector, 0, sizeof(boot_sector));
    
    /* Setup boot sector */
    boot_sector.jmp_boot[0] = 0xEB;
    boot_sector.jmp_boot[1] = 0x58;
    boot_sector.jmp_boot[2] = 0x90;
    memcpy(boot_sector.oem_name, "KRYOS   ", 8);
    boot_sector.bytes_per_sector = 512;
    boot_sector.sectors_per_cluster = SECTORS_PER_CLUSTER;
    boot_sector.reserved_sectors = reserved_sectors;
    boot_sector.num_fats = num_fats;
    boot_sector.root_entry_count = 0;
    boot_sector.total_sectors_16 = 0;
    boot_sector.media_type = 0xF8;
    boot_sector.fat_size_16 = 0;
    boot_sector.sectors_per_track = 63;
    boot_sector.num_heads = 255;
    boot_sector.hidden_sectors = 0;
    boot_sector.total_sectors_32 = total_sectors;
    boot_sector.fat_size_32 = fat_size;
    boot_sector.ext_flags = 0;
    boot_sector.fs_version = 0;
    boot_sector.root_cluster = 2;
    boot_sector.fs_info = 1;
    boot_sector.backup_boot_sector = 6;
    boot_sector.drive_num = 0x80;
    boot_sector.boot_signature = 0x29;
    boot_sector.volume_id = 0x12345678;
    memcpy(boot_sector.volume_label, "KRYOS DISK  ", 11);
    memcpy(boot_sector.fs_type, "FAT32   ", 8);
    
    /* Write boot sector */
    write_sector(0, &boot_sector);
    
    /* Initialize FAT - write zeros first */
    uint32_t fat_sector;
    memset(sector_buffer, 0, 512);
    
    /* Write empty FAT sectors */
    for (fat_sector = 0; fat_sector < fat_size; fat_sector++) {
        write_sector(reserved_sectors + fat_sector, sector_buffer);
        write_sector(reserved_sectors + fat_size + fat_sector, sector_buffer);
    }
    
    /* Write FAT header entries */
    uint32_t *fat = (uint32_t *)sector_buffer;
    fat[0] = 0x0FFFFFF8;  /* Media type */
    fat[1] = 0x0FFFFFFF;  /* End of chain marker */
    fat[2] = 0x0FFFFFFF;  /* Root directory */
    write_sector(reserved_sectors, sector_buffer);
    write_sector(reserved_sectors + fat_size, sector_buffer);
    
    /* Initialize root directory cluster */
    memset(sector_buffer, 0, 512);
    uint32_t root_sector = reserved_sectors + num_fats * fat_size;
    for (fat_sector = 0; fat_sector < SECTORS_PER_CLUSTER; fat_sector++) {
        write_sector(root_sector + fat_sector, sector_buffer);
    }
    
    /* Update filesystem positions */
    fat_start_sector = reserved_sectors;
    data_start_sector = fat_start_sector + num_fats * fat_size;
    root_cluster = 2;
    current_dir_cluster = root_cluster;
    strcpy(current_path, "/");
    
    return 0;
}

/*
 * Open a file
 */
int fat32_open(const char *path, fat_file_t *file) {
    fat_dir_entry_t entry;
    uint32_t cluster = current_dir_cluster;
    
    if (path[0] == '/') {
        cluster = root_cluster;
        path++;
    }
    
    if (find_dir_entry(cluster, path, &entry, 0, 0) != 0) {
        return -1;
    }
    
    parse_filename(entry.name, file->name);
    file->first_cluster = ((uint32_t)entry.fst_clus_hi << 16) | entry.fst_clus_lo;
    file->current_cluster = file->first_cluster;
    file->file_size = entry.file_size;
    file->position = 0;
    file->is_directory = (entry.attr & ATTR_DIRECTORY) != 0;
    file->is_open = 1;
    
    return 0;
}

/*
 * Close a file
 */
void fat32_close(fat_file_t *file) {
    file->is_open = 0;
}

/*
 * Read from a file
 */
int fat32_read(fat_file_t *file, void *buffer, uint32_t count) {
    if (!file->is_open || file->is_directory) {
        return -1;
    }
    
    uint8_t *buf = (uint8_t *)buffer;
    uint32_t bytes_read = 0;
    
    while (bytes_read < count && file->position < file->file_size) {
        uint32_t cluster_offset = file->position & (CLUSTER_SIZE - 1);  /* & 4095 = % 4096 */
        uint32_t bytes_to_read = count - bytes_read;
        
        if (bytes_to_read > CLUSTER_SIZE - cluster_offset) {
            bytes_to_read = CLUSTER_SIZE - cluster_offset;
        }
        if (bytes_to_read > file->file_size - file->position) {
            bytes_to_read = file->file_size - file->position;
        }
        
        read_cluster(file->current_cluster, cluster_buffer);
        memcpy(buf + bytes_read, cluster_buffer + cluster_offset, bytes_to_read);
        
        bytes_read += bytes_to_read;
        file->position += bytes_to_read;
        
        if ((file->position & (CLUSTER_SIZE - 1)) == 0) {
            file->current_cluster = get_fat_entry(file->current_cluster);
            if (file->current_cluster >= 0x0FFFFFF8) {
                break;
            }
        }
    }
    
    return bytes_read;
}

/*
 * Write to a file
 */
int fat32_write(fat_file_t *file, const void *buffer, uint32_t count) {
    if (!file->is_open || file->is_directory) {
        return -1;
    }
    
    const uint8_t *buf = (const uint8_t *)buffer;
    uint32_t bytes_written = 0;
    
    while (bytes_written < count) {
        uint32_t cluster_offset = file->position & (CLUSTER_SIZE - 1);
        uint32_t bytes_to_write = count - bytes_written;
        
        if (bytes_to_write > CLUSTER_SIZE - cluster_offset) {
            bytes_to_write = CLUSTER_SIZE - cluster_offset;
        }
        
        if (cluster_offset != 0 || bytes_to_write < CLUSTER_SIZE) {
            read_cluster(file->current_cluster, cluster_buffer);
        } else {
            memset(cluster_buffer, 0, CLUSTER_SIZE);
        }
        
        memcpy(cluster_buffer + cluster_offset, buf + bytes_written, bytes_to_write);
        write_cluster(file->current_cluster, cluster_buffer);
        
        bytes_written += bytes_to_write;
        file->position += bytes_to_write;
        
        if ((file->position & (CLUSTER_SIZE - 1)) == 0 && bytes_written < count) {
            uint32_t next_cluster = get_fat_entry(file->current_cluster);
            if (next_cluster >= 0x0FFFFFF8) {
                next_cluster = find_free_cluster();
                if (next_cluster == 0) {
                    break;
                }
                set_fat_entry(file->current_cluster, next_cluster);
                set_fat_entry(next_cluster, 0x0FFFFFFF);
            }
            file->current_cluster = next_cluster;
        }
    }
    
    if (file->position > file->file_size) {
        file->file_size = file->position;
    }
    
    return bytes_written;
}

/*
 * Create a new file
 */
int fat32_create(const char *path) {
    fat_dir_entry_t entry;
    uint32_t dir_cluster = current_dir_cluster;
    uint32_t entry_cluster, entry_offset;
    
    if (path[0] == '/') {
        dir_cluster = root_cluster;
        path++;
    }
    
    if (find_dir_entry(dir_cluster, path, &entry, 0, 0) == 0) {
        return -1;
    }
    
    if (find_free_dir_entry(dir_cluster, &entry_cluster, &entry_offset) != 0) {
        return -1;
    }
    
    memset(&entry, 0, sizeof(entry));
    format_filename(path, entry.name);
    entry.attr = ATTR_ARCHIVE;
    entry.fst_clus_hi = 0;
    entry.fst_clus_lo = 0;
    entry.file_size = 0;
    
    read_cluster(entry_cluster, cluster_buffer);
    memcpy(cluster_buffer + entry_offset, &entry, sizeof(entry));
    write_cluster(entry_cluster, cluster_buffer);
    
    return 0;
}

/*
 * Create a directory
 */
int fat32_mkdir(const char *path) {
    fat_dir_entry_t entry;
    uint32_t dir_cluster = current_dir_cluster;
    uint32_t entry_cluster, entry_offset;
    uint32_t new_cluster;
    
    if (path[0] == '/') {
        dir_cluster = root_cluster;
        path++;
    }
    
    if (find_dir_entry(dir_cluster, path, &entry, 0, 0) == 0) {
        return -1;
    }
    
    new_cluster = find_free_cluster();
    if (new_cluster == 0) {
        return -1;
    }
    set_fat_entry(new_cluster, 0x0FFFFFFF);
    
    if (find_free_dir_entry(dir_cluster, &entry_cluster, &entry_offset) != 0) {
        set_fat_entry(new_cluster, 0);
        return -1;
    }
    
    memset(&entry, 0, sizeof(entry));
    format_filename(path, entry.name);
    entry.attr = ATTR_DIRECTORY;
    entry.fst_clus_hi = (new_cluster >> 16) & 0xFFFF;
    entry.fst_clus_lo = new_cluster & 0xFFFF;
    entry.file_size = 0;
    
    read_cluster(entry_cluster, cluster_buffer);
    memcpy(cluster_buffer + entry_offset, &entry, sizeof(entry));
    write_cluster(entry_cluster, cluster_buffer);
    
    /* Initialize new directory with . and .. */
    memset(cluster_buffer, 0, CLUSTER_SIZE);
    
    fat_dir_entry_t *dot = (fat_dir_entry_t *)cluster_buffer;
    memset(dot->name, ' ', 11);
    dot->name[0] = '.';
    dot->attr = ATTR_DIRECTORY;
    dot->fst_clus_hi = (new_cluster >> 16) & 0xFFFF;
    dot->fst_clus_lo = new_cluster & 0xFFFF;
    
    fat_dir_entry_t *dotdot = (fat_dir_entry_t *)(cluster_buffer + 32);
    memset(dotdot->name, ' ', 11);
    dotdot->name[0] = '.';
    dotdot->name[1] = '.';
    dotdot->attr = ATTR_DIRECTORY;
    dotdot->fst_clus_hi = (dir_cluster >> 16) & 0xFFFF;
    dotdot->fst_clus_lo = dir_cluster & 0xFFFF;
    
    write_cluster(new_cluster, cluster_buffer);
    
    return 0;
}

/*
 * Delete a file
 */
int fat32_delete(const char *path) {
    fat_dir_entry_t entry;
    uint32_t dir_cluster = current_dir_cluster;
    uint32_t entry_cluster, entry_offset;
    
    if (path[0] == '/') {
        dir_cluster = root_cluster;
        path++;
    }
    
    if (find_dir_entry(dir_cluster, path, &entry, &entry_cluster, &entry_offset) != 0) {
        return -1;
    }
    
    uint32_t cluster = ((uint32_t)entry.fst_clus_hi << 16) | entry.fst_clus_lo;
    while (cluster >= 2 && cluster < 0x0FFFFFF8) {
        uint32_t next = get_fat_entry(cluster);
        set_fat_entry(cluster, 0);
        cluster = next;
    }
    
    read_cluster(entry_cluster, cluster_buffer);
    cluster_buffer[entry_offset] = 0xE5;
    write_cluster(entry_cluster, cluster_buffer);
    
    return 0;
}

/*
 * List directory contents
 */
int fat32_list_dir(const char *path, void (*callback)(const char *name, uint8_t attr, uint32_t size)) {
    uint32_t dir_cluster = current_dir_cluster;
    char name[13];
    uint32_t cluster;
    int i;
    
    if (path && path[0] == '/') {
        dir_cluster = root_cluster;
        path++;
    }
    
    if (path && path[0] != '\0') {
        fat_dir_entry_t entry;
        if (strcmp(path, ".") == 0) {
            dir_cluster = current_dir_cluster;
        } else if (strcmp(path, "..") == 0) {
            if (find_dir_entry(current_dir_cluster, "..", &entry, 0, 0) == 0) {
                dir_cluster = ((uint32_t)entry.fst_clus_hi << 16) | entry.fst_clus_lo;
                if (dir_cluster == 0) dir_cluster = root_cluster;
            }
        } else {
            if (find_dir_entry(dir_cluster, path, &entry, 0, 0) != 0) {
                return -1;
            }
            if (!(entry.attr & ATTR_DIRECTORY)) {
                return -1;
            }
            dir_cluster = ((uint32_t)entry.fst_clus_hi << 16) | entry.fst_clus_lo;
        }
    }
    
    cluster = dir_cluster;
    
    while (cluster >= 2 && cluster < 0x0FFFFFF8) {
        read_cluster(cluster, cluster_buffer);
        
        for (i = 0; i < ENTRIES_PER_CLUSTER; i++) {
            fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)&cluster_buffer[i * 32];
            
            if ((uint8_t)dir_entry->name[0] == 0x00) {
                return 0;
            }
            
            if ((uint8_t)dir_entry->name[0] == 0xE5) {
                continue;
            }
            
            if ((dir_entry->attr & ATTR_VOLUME_ID) || (dir_entry->attr & ATTR_HIDDEN)) {
                continue;
            }
            
            parse_filename(dir_entry->name, name);
            callback(name, dir_entry->attr, dir_entry->file_size);
        }
        
        cluster = get_fat_entry(cluster);
    }
    
    return 0;
}

/*
 * Change current directory
 */
int fat32_chdir(const char *path) {
    fat_dir_entry_t entry;
    uint32_t dir_cluster = current_dir_cluster;
    
    if (path[0] == '/') {
        dir_cluster = root_cluster;
        path++;
        strcpy(current_path, "/");
    }
    
    if (path[0] == '\0') {
        current_dir_cluster = root_cluster;
        strcpy(current_path, "/");
        return 0;
    }
    
    if (strcmp(path, ".") == 0) {
        return 0;
    }
    
    if (strcmp(path, "..") == 0) {
        if (find_dir_entry(current_dir_cluster, "..", &entry, 0, 0) == 0) {
            uint32_t parent = ((uint32_t)entry.fst_clus_hi << 16) | entry.fst_clus_lo;
            if (parent == 0) parent = root_cluster;
            current_dir_cluster = parent;
            
            int len = strlen(current_path);
            if (len > 1) {
                /* Find the last slash (not the trailing one) */
                char *last_slash = current_path;
                char *p = current_path + 1;
                while (*p) {
                    if (*p == '/') last_slash = p;
                    p++;
                }
                /* If last_slash still points to root, set path to "/" */
                if (last_slash == current_path) {
                    current_path[1] = '\0';
                } else {
                    *last_slash = '\0';
                }
            }
        }
        return 0;
    }
    
    if (find_dir_entry(dir_cluster, path, &entry, 0, 0) != 0) {
        return -1;
    }
    
    if (!(entry.attr & ATTR_DIRECTORY)) {
        return -1;
    }
    
    current_dir_cluster = ((uint32_t)entry.fst_clus_hi << 16) | entry.fst_clus_lo;
    
    int len = strlen(current_path);
    if (len > 1) {
        current_path[len] = '/';
        strcpy(current_path + len + 1, path);
    } else {
        strcpy(current_path + 1, path);
    }
    
    return 0;
}

/*
 * Get current directory
 */
const char *fat32_getcwd(void) {
    return current_path;
}
