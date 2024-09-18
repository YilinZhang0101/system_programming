/**
 * finding_filesystems
 * CS 341 - Fall 2023
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))

/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t used_num) __attribute__((unused));
static char *block_info_string(ssize_t used_num) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - used_num;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, used_num);
    return block_string;
}

// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    inode* new_inode = get_inode(fs, path);

    if (new_inode == NULL) {
        errno = ENOENT;
        return -1;
    } 

    // only overwrite the bottom RWX_BITS_NUMBER bytes.
    new_inode->mode = (new_inode->mode & 0xfe00) | new_permissions;
    // update the node's ctim
    clock_gettime(CLOCK_REALTIME, &new_inode->ctim);

    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    inode* new_inode = get_inode(fs, path);

    if (new_inode == NULL) {
        errno = ENOENT;
        return -1;
    } 

    // If owner is ((uid_t)-1), then don't change the node's uid.
    // If group is ((gid_t)-1), then don't change the node's gid.
    if (owner != (uid_t)-1) {
        new_inode->uid = owner;
    }
    if (group != (gid_t)-1) {
        new_inode->gid = group;
    }
    // update the node's ctim
    clock_gettime(CLOCK_REALTIME, &new_inode->ctim);

    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    inode* new_inode = get_inode(fs, path);
    // return NULL if inode already exists or cannot be created.
    if (new_inode != NULL) {
        return NULL;
    } 
    // if the file does not exist, you will need to find its parent inode,
    // add a new dirent to the parent and then increase the parent's size.
    inode_number get_num_unused = first_unused_inode(fs);
    if (get_num_unused == -1) {
        return NULL;
    }

    const char* get_name = NULL;
    inode* get_parent = parent_directory(fs, path, &get_name);
    if (valid_filename(get_name) == 0) {
        return NULL;
    }

    data_block_number idb = get_parent->size / sizeof(data_block);
    size_t offset_db = get_parent->size % sizeof(data_block);

    if (idb >= NUM_DIRECT_BLOCKS) {
        if (get_parent->indirect == UNASSIGNED_NODE){
            if (add_single_indirect_block(fs, get_parent) == -1) {
                return NULL;
            }
        }
    }
    
    inode* unused_inode = fs->inode_root + get_num_unused;
    init_inode(get_parent, unused_inode);

    data_block* new_db;
    if (idb < NUM_DIRECT_BLOCKS) {
        if (offset_db != 0) {
            new_db = (data_block*)(fs->data_root + get_parent->direct[idb]);
        }
        else {
            if ((add_data_block_to_inode(fs, get_parent)) == -1) {
                return NULL;
            }
            new_db = (data_block*)(fs->data_root);
        }
    } 
    else {
        if (offset_db != 0) {
            new_db = (data_block*)(fs->data_root + ((data_block_number*)(fs->data_root + get_parent->indirect))[idb - NUM_DIRECT_BLOCKS]);
        }
        else {
            if ((add_data_block_to_indirect_block(fs, (data_block_number*)(fs->data_root + get_parent->indirect))) == -1) {
                return NULL;
            }
            new_db = (data_block*)(fs->data_root);
        }
    }
    
    minixfs_dirent new_dirent;
    get_parent->size += FILE_NAME_ENTRY;
    char cp_name[strlen(get_name) + 1];
    memcpy(cp_name, get_name, strlen(get_name) + 1);
    new_dirent.name = cp_name;
    new_dirent.inode_num = get_num_unused;
    
    make_string_from_dirent(new_db->data + offset_db, new_dirent);

    clock_gettime(CLOCK_REALTIME, &get_parent->mtim);
    clock_gettime(CLOCK_REALTIME, &get_parent->atim);
    return unused_inode;
}

ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        char* new_dm = GET_DATA_MAP(fs->meta);
        ssize_t used_num = 0;
        for (size_t i = 0; i < fs->meta->dblock_count; i++) {
            if ((int)new_dm[i] == 0) {
                continue;
            }
            used_num++;
        }
        char* block_info = block_info_string(used_num);
        size_t tail = min(*off + count, strlen(block_info));
        
        if (tail <= (size_t)*off) {
            return 0;
        }
        memcpy(buf, block_info + *off, tail - (size_t)*off);
        *off += tail;
        return tail - *off;
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    // create the file if it doesn't exist
    inode* new_inode = get_inode(fs, path);
    if (new_inode == NULL) {
        new_inode = minixfs_create_inode_for_path(fs, path);
    }

    int num_block = (int) ((*off + count) / sizeof(data_block)) + 1;
    int inode_create = minixfs_min_blockcount(fs, path, num_block);
    if ((new_inode == NULL) || (inode_create != 0)) {
        errno = ENOSPC;
        return -1;
    }
    
    size_t init_count = count;
    size_t index_db = (*off) / sizeof(data_block);
    size_t offset_db = (*off) % sizeof(data_block);

    while (count > 0) {
        data_block* new_db;
        if (index_db >= NUM_DIRECT_BLOCKS) {
            size_t index_indir = index_db - NUM_DIRECT_BLOCKS;
            data_block_number* num_db = (data_block_number*)(fs->data_root + new_inode->indirect);
            new_db = (data_block*)(fs->data_root + num_db[index_indir]);
        }
        else {
            new_db = (data_block*)(fs->data_root + new_inode->direct[index_db]);
        }

        size_t need_write = min(sizeof(data_block) - offset_db, count);
        memcpy(new_db->data + offset_db, buf + init_count - count, need_write);
        *off += need_write;
        count -= need_write;
        index_db ++;
        offset_db = 0;  
    }
    
    size_t all_written = init_count - count;
    new_inode->size += all_written;
    clock_gettime(CLOCK_REALTIME, &new_inode->mtim);
    clock_gettime(CLOCK_REALTIME, &new_inode->atim);
    return all_written;
}

ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!

    // create the file if it doesn't exist
    inode* new_inode = get_inode(fs, path);
    if (new_inode == NULL) {
        errno = ENOENT;
        return -1;
    }

    size_t tail = min(new_inode->size, (size_t)*off + count);
    if (tail <= (size_t) *off) {
        return 0;
    }

    size_t index_db = (*off) / sizeof(data_block);
    size_t offset_db = (*off) % sizeof(data_block);

    size_t fill_read = tail - (size_t)*off;
    size_t init_fill_read = fill_read;
    while (fill_read > 0) {
        data_block* new_db;
        if (index_db >= NUM_DIRECT_BLOCKS) {
            size_t index_indir = index_db - NUM_DIRECT_BLOCKS;
            data_block_number* num_db = (data_block_number*)(fs->data_root + new_inode->indirect);
            new_db = (data_block*)(fs->data_root + num_db[index_indir]);
        }
        else {
            new_db = (data_block*)(fs->data_root + new_inode->direct[index_db]);
        }

        size_t need_write = min(sizeof(data_block) - offset_db, fill_read);
        memcpy(buf + init_fill_read - fill_read, new_db->data + offset_db, need_write);
        *off += need_write;
        fill_read -= need_write;
        index_db ++;
        offset_db = 0;  
    }

    clock_gettime(CLOCK_REALTIME, &new_inode->atim);
    return init_fill_read - fill_read;
}
