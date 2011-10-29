/*
 *  fs.h
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */



typedef unsigned int	__u32;
typedef unsigned short	__u16;

typedef int				__s32;
typedef short			__s16;

typedef struct block_group block_group;
typedef struct inode inode;


#define EXT2_NAME_LEN			255

#define FS_BLOCK_GROUP_COUNT	2
#define	FS_BLOCK_GROUP_SIZE		sizeof(block_group)

#define FS_BLOCK_SIZE		    1024
#define	FS_DATA_TABLE_SIZE	    7280

#define	FS_INODE_TABLE_SIZE		910
#define FS_INODE_SIZE			sizeof(inode)
#define FS_INODES_PER_BLOCK		8

#define FS_INODE_BITMAP_SIZE	(FS_DATA_TABLE_SIZE * FS_BLOCK_GROUP_COUNT)
#define FS_DATA_BITMAP_SIZE		(FS_INODE_TABLE_SIZE * FS_BLOCK_GROUP_COUNT * FS_INODES_PER_BLOCK)

#define FS_CACHE_SIZE			2048
#define FS_GLOB_GB_OFFSET		3

#define FS_INFO_LEN				8

#define ACTION_READ 		10
#define ACTION_WRITE 		11

#define EXT2_FT_UNKNOWN		0		// Unknown File Type
#define EXT2_FT_REG_FILE	1		// Regular File
#define EXT2_FT_DIR			2		// Directory File
#define EXT2_FT_CHRDEV		3		// Character Device
#define EXT2_FT_BLKDEV		4		// Block Device
#define EXT2_FT_FIFO		5		// Buffer File
#define EXT2_FT_SOCK		6		// Socket File
#define EXT2_FT_SYMLINK		7		// Symbolic Link

#define EXT2_S_IFSOCK	0xC000	// socket
#define EXT2_S_IFLNK	0xA000	// symbolic link
#define EXT2_S_IFREG	0x8000	// regular file
#define EXT2_S_IFBLK	0x6000	// block device
#define EXT2_S_IFDIR	0x4000	// directory
#define EXT2_S_IFCHR	0x2000	// character device
#define EXT2_S_IFIFO	0x1000	// fifo

#define EXT2_S_ISUID	0x0800	// Set process User ID
#define EXT2_S_ISGID	0x0400	// Set process Group ID
#define EXT2_S_ISVTX	0x0200	// sticky bit

#define EXT2_S_IRUSR	0x0100	// user read
#define EXT2_S_IWUSR	0x0080	// user write
#define EXT2_S_IXUSR	0x0040	// user execute
#define EXT2_S_IRGRP	0x0020	// group read
#define EXT2_S_IWGRP	0x0010	// group write
#define EXT2_S_IXGRP	0x0008	// group execute
#define EXT2_S_IROTH	0x0004	// others read
#define EXT2_S_IWOTH	0x0002	// others write
#define EXT2_S_IXOTH	0x0001	// others execute

typedef struct ext2_dir_entry {
	__u32   inode;                  /* Inode number */
	__u16   rec_len;                /* Directory entry length */
	unsigned char	name_len;               /* Name length */
	unsigned char	file_type;		
	char    name[EXT2_NAME_LEN];    /* File name */
} dir_entry;

typedef struct ext2_dir_block {
	char	data[FS_BLOCK_SIZE];
} dir_block;



typedef struct ext2_super_block {
	__u32   s_inodes_count;         /* Inodes count */
	__u32   s_blocks_count;         /* Blocks count */
	__u32   s_r_blocks_count;       /* Reserved blocks count */
	__u32   s_free_blocks_count;    /* Free blocks count */
	__u32   s_free_inodes_count;    /* Free inodes count */
	__u32   s_first_data_block;     /* First Data Block */
	__u32   s_log_block_size;       /* Block size */
	__s32   s_log_frag_size;        /* Fragment size */
	__u32   s_blocks_per_group;     /* # Blocks per group */
	__u32   s_frags_per_group;      /* # Fragments per group */
	__u32   s_inodes_per_group;     /* # Inodes per group */
	__u32   s_mtime;                /* Mount time */
	__u32   s_wtime;                /* Write time */
	__u16   s_mnt_count;            /* Mount count */
	__s16   s_max_mnt_count;        /* Maximal mount count */
	__u16   s_magic;                /* Magic signature */
	__u16   s_state;                /* File system state */
	__u16   s_errors;               /* Behaviour when detecting errors */
	__u16   s_pad;
	__u32   s_lastcheck;            /* time of last check */
	__u32   s_checkinterval;        /* max. time between checks */
	__u32   s_creator_os;           /* OS */
	__u32   s_rev_level;            /* Revision level */
	__u16   s_def_resuid;           /* Default uid for reserved blocks */
	__u16   s_def_resgid;           /* Default gid for reserved blocks */
	__u32   s_reserved[235];        /* Padding to the end of the block */
} super_block;


typedef struct ext2_group_desc
{
	__u32   bg_block_bitmap;        /* Blocks bitmap block */
	__u32   bg_inode_bitmap;        /* Inodes bitmap block */
	__u32   bg_inode_table;         /* Inodes table block */
	__u16   bg_free_blocks_count;   /* Free blocks count */
	__u16   bg_free_inodes_count;   /* Free inodes count */
	__u16   bg_used_dirs_count;     /* Directories count */
	__u16   bg_pad;
	__u32   bg_reserved[3];
} group_descriptor;


struct inode {
	int	mode;
	
	unsigned int		size;
	
	int		lasta_date;
	int		create_date;
	int		modify_date;
	int		delete_date;
	
	int		uid;
	int		gid;
	short 	links;
	
	int		blocks;
	
	int		data_blocks[15];
	
	int		generation;
	
	int		i_file_acl;
	int		i_dir_acl;
	short	i_faddr;
	
	// Ousize EXT2 Standard, used by our OS
	int		_dir_log_block;
	int		_last_write_offset;
	int		_dir_inode;
};

typedef struct block {
	char data[FS_BLOCK_SIZE];
} block;


struct block_group {
	block	data_bitmap;
	block	inode_bitmap;
	block	inode_table[FS_INODE_TABLE_SIZE];
	block	data_table[FS_DATA_TABLE_SIZE];
};

typedef struct fs_header {
	block			_start;
	block			_super;
	block			_descriptors;
} fs_header;

typedef struct fs_data {
	fs_header		head;
	block			cache[FS_CACHE_SIZE];
} fs_data;

typedef struct hdd {
	fs_header		head;
	block_group		data[FS_BLOCK_GROUP_COUNT];
} hdd;

void fs_init();

unsigned int fs_mkdir(char * name, unsigned int parent_inode);

unsigned int fs_indir(char * name, int folder_inode);

unsigned int fs_open_file(char * name, unsigned int folder_inode, int mode, int type);

unsigned int fs_write_file(int inode, char * data, int size);

unsigned int fs_read_file(int inode, char * data, int size, unsigned long * f_offset);

unsigned int fs_rm(unsigned int inode, int recursive);

char *       fs_pwd();

unsigned int folder_rem_direntry(unsigned int file_inode, unsigned int folder_inode);

unsigned int fs_cp(char * name, char * newname, int from_inode, int to_inode);

unsigned int fs_mv(char * name, char * newname, int from_inode);

unsigned int fs_open_link(char * name, char * target_name);

unsigned int fs_open_fifo(char * name, unsigned int folder_inode, int mode);

unsigned int fs_open_reg_file(char * name, unsigned int folder_inode, int mode);

unsigned int fs_chown(char * filename, char * username);

unsigned int fs_chmod(char * filename, int perms);

unsigned int fs_getown(char * filename);

unsigned int fs_getmod(char * filename);

unsigned int fs_has_perms(inode * n, int for_what);

unsigned int fs_is_fifo(int inode);