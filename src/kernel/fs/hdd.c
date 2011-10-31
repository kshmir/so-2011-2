/*
 *  hdd.c
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

// Made for debugging purposes, used as RAM on OSX, and as a wrapper in real life, probably used for cache later on.


#include "hdd.h"
#include "fs.h"



hdd_cache cache;

hdd_block buffer[64];

int _cache = 1;

// Cache Add After Read			- Single block
// Cache Get   Block			- Get block from cache
// Cache Write Block			- Write a block inside the cache.
// Cache Flush					- Drop all blocks down.

int hdd_blocks(hdd_block * b1, hdd_block * b2) {

}

int least_reads_family_index() {
	int i = 0;
	int j = 0;
	int min_reads = 100000;
	int min_reads_index = -1;
	for (; i < 64; i++) {
		int reads = 0;

		int valid = 0;
		for (j = 0; j < 64; j++) {
			if (cache.metadata[i * 64 + j].block == -1) {
				valid = 1;
				reads += cache.metadata[i * 64 + j].reads;
			}
		}
		if (reads < min_reads && valid) {
			min_reads = reads;
			min_reads_index = i;
			if (!reads) {
				break;
			}
		}
	}

	if (min_reads_index == -1) {
		for (i = 0; i < 64; i++) {
			int reads = 0;
			for (j = 0; j < 64; j++) {
				reads += cache.metadata[i * 64 + j].reads;
			}
			if (reads < min_reads) {
				min_reads = reads;
				min_reads_index = i;
				if (!reads) {
					break;
				}
			}
		}

	}

	return min_reads_index;
}

int hdd_cache_add(hdd_block * buff, int len, int start_block, int write) {
	int least_reads_fam = least_reads_family_index();

	hdd_flush_family(least_reads_fam * 64, FALSE);

	int i = 0;
	int j = 0;	
	for (; i < len; i++) {
		cache.metadata[least_reads_fam * 64 + i].block   = start_block + i;
		cache.metadata[least_reads_fam * 64 + i].reads   = 0;
		cache.metadata[least_reads_fam * 64 + i].writes  = 0;

		for (j = 0; j < HDD_BLOCK_SIZE; j++) {
			cache.data[least_reads_fam * 64 + i].data[j] = buff[i].data[j];
		}
	}
}

void * hdd_get_block(int block_id, int write) {
	int i = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		if (cache.metadata[i].block == block_id) {
			printf("Cache hit %d\n", block_id);
			if (!write) {
				cache.metadata[i].reads++;
			}
			return &cache.data[i];
		}
	}

	_disk_read(ATA0, (void *)&buffer, (block_id / 64) * 64 * 2 + 1, 126);

	hdd_cache_add((void *)&buffer, 64, (block_id / 64) * 64, write);
	i = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		if (cache.metadata[i].block == block_id) {
			if (!write) {
				cache.metadata[i].reads++;
			}
			return &cache.data[i];
		}
	}

	return 0;
}

int hdd_write_block(char * data, int block_id) {
	hdd_get_block(block_id, TRUE);
	int i = 0;
	int j = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		if (cache.metadata[i].block == block_id) {
			cache.metadata[i].writes++;
			for (j = 0; j < HDD_BLOCK_SIZE; j++) {
				cache.data[i].data[j] = data[j];
			}
			return 1;
		}
	}	
	return 0;
}


int hdd_flush_family(int family_id, int flush_force) {
	int i = 0;
	int nwrites = 0;
	for (; i < 64; i++) {
		nwrites += cache.metadata[family_id + i].writes;
		cache.metadata[family_id + i].writes = 0;
	}
	if (nwrites) {
		_disk_write(ATA0, (void *)&cache.data[family_id], cache.metadata[family_id].block * 2 + 1, 126);
	}
}


int hdd_cache_sync(int hard) {
	int i = 0;
	for (; i < 64; i++) {
		if (cache.metadata[i * 64].block != -1) {
			hdd_flush_family(i * 64, TRUE);
		}
	}
}

void hdd_init() {
	int i = 0;
	for (; i < HDD_CACHE_SIZE; i++) {
		cache.metadata[i].block  = -1;
		cache.metadata[i].reads  = 0;
		cache.metadata[i].writes = 0;
	}
}

void hdd_read(char * answer, unsigned int sector) {
	if (!sector) {
		return;
	}
	
	if (_cache) {
		hdd_write_block((void *)buffer, (sector - 1) / 2);
	} else {
		_disk_read(ATA0, (void *)answer, 2, sector);
	}
}

void hdd_write(char * buffer, unsigned int sector) {
	
	if (!sector) {
		return;
	}
	if (_cache) {
		char * data = hdd_get_block((sector - 1) / 2, FALSE);
		int i = 0;
		for (; i < HDD_BLOCK_SIZE; i++) {
			buffer[i] = data[i];
		}
	} else {
		_disk_write(ATA0, (void *)buffer, 2, sector);
	}
	

}

void hdd_close() {
	// Nothing to do on this end.
}
