/*
 *  hdd.h
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */
#include "../../drivers/atadisk.h"

#ifndef _HDD_H_
#define _HDD_H_

#define SECTOR_SIZE 512

#define HDD_BLOCK_SIZE 1024		
#define HDD_CACHE_SIZE 4096

#define TRUE  1
#define FALSE 0

#define CACHE_SUCCESS  3
#define CACHE_HIT      2
#define CACHE_MISS	   1
#define CACHE_ERROR    0

#define NULL 0

typedef struct hdd_block {
	char data[HDD_BLOCK_SIZE];
} hdd_block;

typedef struct hdd_block_metadata {
	int    block;						// Block number inside the disk
	unsigned short  reads;				// Number of reads made inside the block.
	unsigned short  writes;				// Number of writes made inside the block since the last flush.
} hdd_block_metadata;

typedef struct hdd_cache {
	unsigned int		dirties;
	hdd_block_metadata	metadata[HDD_CACHE_SIZE];
	hdd_block			data  [HDD_CACHE_SIZE];
} hdd_cache;

void hdd_init();

void hdd_read(char * answer, unsigned int sector);

void hdd_write(char * buffer, unsigned int sector);

void hdd_close();

#endif