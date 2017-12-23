#ifndef DISK_IO_H
#define DISK_IO_H


#include "c_types.h"


#define MAX_DISKS	2


typedef uint32_t disk_io_status_t(void *disk);
typedef uint32_t disk_io_init_t(void *disk);

typedef uint32_t disk_io_read_t(void *disk, uint32_t sector, uint8_t* buf, uint32_t count);
typedef uint32_t disk_io_write_t(void *disk, uint32_t sector, const uint8_t* buf, uint32_t count);

typedef uint32_t disk_io_ioctl_t(void *disk, uint32_t cmd, void* buf);

typedef struct disk_io_t {
	uint32_t type;
	void* data;

	uint32_t initialized;
	uint32_t blksize;

	disk_io_status_t* status;
	disk_io_init_t* init;
	disk_io_read_t* read;
	disk_io_write_t* write;
	disk_io_ioctl_t* ioctl;
	
} disk_io_t;


void disk_io_init();

extern disk_io_t disks[];


#endif

