/*
 *  fs.h
 *  SO-FS
 *
 *  Created by Cristian Pereyra on 19/10/11.
 *  Copyright 2011 My Own. All rights reserved.
 *
 */

// Typedefs to make copy paste of the structs :p
typedef unsigned int	__u32;
typedef unsigned short	__u16;

typedef int				__s32;
typedef short			__s16;

typedef struct block_group block_group;
typedef struct inode inode;

// Max length for the name of a directory entry
#define EXT2_NAME_LEN			255

// Group count: 8 x 6 = 48MB, the disk didn't like to have more :(
#define FS_BLOCK_GROUP_COUNT	1

#define	FS_BLOCK_GROUP_SIZE		sizeof(block_group)

// Size of a block: 1K
#define FS_BLOCK_SIZE		    1024
// Data blocks inside a block group: 7280K
#define	FS_DATA_TABLE_SIZE	    7280

// Inode blocks inside a block group: 7280K (910K x 8)
#define	FS_INODE_TABLE_SIZE		910
#define FS_INODE_SIZE			sizeof(inode)
// Inode size: 128bytes, block size: 1kbyte => 8
#define FS_INODES_PER_BLOCK		8

// Calculates the sizes described before.
#define FS_INODE_BITMAP_SIZE	(FS_DATA_TABLE_SIZE * FS_BLOCK_GROUP_COUNT)
#define FS_DATA_BITMAP_SIZE		(FS_INODE_TABLE_SIZE * FS_BLOCK_GROUP_COUNT * FS_INODES_PER_BLOCK)

// Declared but never used.
#define FS_CACHE_SIZE			2048

// Offset for the block group inside the disk.
#define FS_GLOB_GB_OFFSET		3

// Length of the FS info array.
#define FS_INFO_LEN				8

// FS action permissions.
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

//////////// Sample of a EXT2 filesystem allocated in disk
// NO BLOCK GROUP                              |BLOCK GROUP 0                                              | LOCK GROUP 1 
// Block 0       |Block 1       |Block 2       |Block 3       |Block 4       |Block 5       |Block 915     |Block 8199    |
// MBR and Parti_|Superblock:   |Group Block   |Data Bitmap   |Inode Bitmap  |Start of inode|Start of data | Goes on......|
// tion BR       |Stores all the|Descriptor    |Tells which   |Same as       |blocks        |Blocks        |              |
//               |key data to   |Table:        |data is stored|the Data Bit_ |              |              |              |
//               |start the fi_ |Stores the key|and which not |map           |              |              |              |
//               |lesystem      |info for each |              |              |              |              |              |
//               |              |group block   |              |              |              |              |              |
// 
typedef struct ext2_dir_entry {
	__u32   inode;                  /* Inode number */
	__u16   rec_len;                /* Directory entry length */
	unsigned char	name_len;       /* Name length */
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

// Pretty much like an EXT2 inode, but with some weird changes because we wanted to.
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


// Starts the filesystem, checks wether the magic number is set or not and builds all the bitmaps, group blocks and stuff.
void fs_init();

// fs_mkdir
// Makes a directory
// name: Name of the directory
// parent_inode: Inode of the folder in which the directory is done, if it's 0, then it's a root folder (/)
unsigned int fs_mkdir(char * name, unsigned int parent_inode);

// fs_indir
// Tells wether a directory entry name is inside a folder
// name: Name of the entry
// folder_inode: Inode of the folder in which the entry is searched.
unsigned int fs_indir(char * name, int folder_inode);

// fs_open_file
// Starts a new file or opens it with certain permissions
// name: Name of the file to open.
// folder_inode: Inode in which the file will be opened.
// mode: Open mode, following MONIX compliants
// #define	O_CREAT					0x800		// Deletes the old file if exists.
// #define	O_RD					0x400		// Reads from file.
// #define	O_WR					0x200		// Writes to file.
// #define	O_NEW					0x100		// Creates a new only if exists.
// type: Type of file to open
// #define EXT2_S_IFSOCK	0xC000	// socket
// #define EXT2_S_IFLNK	0xA000	// symbolic link
// #define EXT2_S_IFREG	0x8000	// regular file
// #define EXT2_S_IFBLK	0x6000	// block device
// #define EXT2_S_IFDIR	0x4000	// directory
// #define EXT2_S_IFCHR	0x2000	// character device
// #define EXT2_S_IFIFO	0x1000	// fifo
// Can return:
// #define	ERR_GENERAL			-1
// #define	ERR_PERMS			-2
// #define	ERR_NO_EXIST		-3
// #define	ERR_EXISTS			-4
// #define	ERR_REPEATED		-5
// #define	ERR_INVALID_TYPE	-6
// #define	ERR_INVALID_PATH	-7
unsigned int fs_open_file(char * name, unsigned int folder_inode, int mode, int type);

// fs_write_file
// inode: Inode to write in
// data: Data to write
// size: Side of the data to write
// returns: Size of the data written, -1 if error
unsigned int fs_write_file(int inode, char * data, int size);

// fs_read_file
// inode: Inode to read from
// data: Data to read
// size: Side of the data to read
// returns: Size of the data read, -1 if error
unsigned int fs_read_file(int inode, char * data, int size, unsigned long * f_offset);

// fs_rm
// Removes a file or folder (recursively) from the filesystem
// inode: Inode to delete
// in_recursion: Used for making the recursive deletion
// returns: a MONIX error code (like open) or something > 1 if OK.
unsigned int fs_rm(unsigned int inode, int in_recursion);

// fs_pwd
// Tells the current position in the filesystem
// Returns the PWD.
char *       fs_pwd();

// folder_rem_direntry
// Deletes a directory entry from a directory
// file_inode: Inode to delete from the folder
// folder_inode: Inode of the folder to delete from.
// returns: a MONIX error code or > 1 if OK
unsigned int folder_rem_direntry(unsigned int file_inode, unsigned int folder_inode);

// fs_cp
// Copies (even recursively) a folder or file from a name to another.
// name: name of the file to copy
// newname: new name of the file
// from_inode: inode folder from which to copy
// to_inode: inode folder to which we copy
// returns: a MONIX error code or > 1 if OK
unsigned int fs_cp(char * name, char * newname, int from_inode, int to_inode);

// fs_mv
// Moves (even recursively) a folder or file from a name to another.
// name: name of the file to move
// newname: new name of the file
// from_inode: inode folder from which to copy
// returns: a MONIX error code or > 1 if OK
unsigned int fs_mv(char * name, char * newname, int from_inode);

// fs_open_link
// Makes a symlink between two files in the current inode.
// name: name of the link
// target_name: name of the file to link
// returns: a MONIX error code or > 1 if OK
unsigned int fs_open_link(char * name, char * target_name);

// fs_open_fifo
// Makes a fifo
// name: name of the fifo
// folder_inode: folder in which the fifo will be done.
// returns: a MONIX error code or > 1 if OK
unsigned int fs_open_fifo(char * name, unsigned int folder_inode, int mode);

// fs_open_reg_file
// Makes a regular file
// name: name of the file
// target_name: name of the file to link
// returns: a MONIX error code or > 1 if OK
unsigned int fs_open_reg_file(char * name, unsigned int folder_inode, int mode);

// fs_chown
// Changes the owner of a file
// filename: name of the file to change
// username: name of the user to make owner of the file
// returns: a MONIX error code or > 1 if OK
unsigned int fs_chown(char * filename, char * username);

// fs_chmod
// Changes the permissions of a file
// filename: name of the file to change
// perms: permissions to change
// returns: a MONIX error code or > 1 if OK
unsigned int fs_chmod(char * filename, int perms);

// fs_getown
// Changes the permissions of a file
// filename: name of the file to query
// returns: the owner's id
unsigned int fs_getown(char * filename);

// fs_getmod
// Changes the permissions of a file
// filename: name of the file to query
// returns: the file's permissions
unsigned int fs_getmod(char * filename);

// fs_has_perms
// Handles the ACL of a file, *nix like
// n: inode pointer to the inode to check
// for_what: ACTION_READ or ACTION_WRITE, actually :p
// returns: 0 or 1
unsigned int fs_has_perms(inode * n, int for_what);

// fs_is_fifo
// Clear name huh? :)
unsigned int fs_is_fifo(int inode);