#include <iostream>

struct SuperBlock{
    double block_size;
    unsigned short int root_pos;
    unsigned short int free_size;
    bool * free_blocks;
};

struct DirectoryEntry{
    char name[8];
    char time[6];
    char date[10];
    char attr[2];
    unsigned short int start_block;
    int size;
};

union Block{
    DirectoryEntry * ent;
    char * file;
};

class FS{
public:
    FS();
    FS(double block_size);
    ~FS();
    void load_file_system(char * filename);
    void save_file_system(char * filename);
    void recursive_traverse(unsigned short int index,int fd,int command);
    unsigned short int traverse_path(char * path);
    void allocate_new_block(unsigned short int index,char * attr);
    void dir_f(char * path);
    unsigned short int mkdir_f(char * path,char * dirname);
    void rmdir_f(char * path,char * dirname);
    void dumpe2fs_f();
    unsigned short int write_f(char * path,char * filename,char * file);
    void read_f(char * path,char * filename,char * file);
    void del_f(char * path,char * filename);
    unsigned short int * table;
    SuperBlock sb;
    Block * blocks;
    char filename[128];
};