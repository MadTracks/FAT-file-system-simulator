#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "filesystem.h"
#define TOTAL_BLOCKS 4096

void get_local_time(char * string);
void get_local_date(char * string);

int main(int argc,char ** argv){
    double block_size=0;
    char fs_name[256];
    sscanf(argv[1],"%lf",&block_size);
    sscanf(argv[2],"%s",fs_name);

    if(block_size==4 || block_size==2 || block_size ==1 || block_size==0.5){
        int fd=0;
        fd=open(fs_name,O_CREAT | O_RDWR | O_EXCL,0666);
        if(fd==-1){
            perror("Open error");
            exit(EXIT_FAILURE);
        }
        int i=0;
        for(i=0;i<block_size*1024*TOTAL_BLOCKS;i++){
            write(fd,"0",1);
        }
        close(fd);
        FS * file_sys = new FS(block_size);
        std::cout<<"File system created with block size:"<<block_size<<"\n";
        file_sys->save_file_system(fs_name);
        delete file_sys;
    }
    else{
        std::cerr<<"Block size must be 4, 2, 1 or 0.5\n";
        exit(EXIT_FAILURE);
    }

    
}

FS::FS(double block_size){
    if(block_size==4 || block_size==2 || block_size ==1 || block_size==0.5){
        this->sb.block_size=block_size;
        this->table = new unsigned short int[TOTAL_BLOCKS];
        int sb_size = ((sizeof(unsigned short int)*2+sizeof(double)+sizeof(bool)*TOTAL_BLOCKS)/(block_size*1024))+1;
        int fat_size = (sizeof(unsigned short int)*TOTAL_BLOCKS)/(block_size*1024);
        this->sb.root_pos=sb_size+fat_size+1;
        int i=0;
        for(i=0;i<TOTAL_BLOCKS;i++){
            if(i<sb_size-1){
                this->table[i]=i+1;
            }
            else if(i<sb_size){
                this->table[i]=-1;
            }
            else if(i<sb_size+fat_size-1){
                this->table[i]=i+1;
            }
            else if(i<sb_size+fat_size+1){
                this->table[i]=-1;
            }
            else{
                this->table[i]=-2;
            }
        }
        this->sb.free_blocks = new bool[TOTAL_BLOCKS];
        for(i=0;i<TOTAL_BLOCKS;i++){
            if(i<=sb_size+fat_size+1){
                this->sb.free_blocks[i]=false;
            }
            else{
                this->sb.free_blocks[i]=true;
            }
        }
        this->sb.free_size=TOTAL_BLOCKS-(sb_size+fat_size);
        this->blocks=new Block[TOTAL_BLOCKS];
        this->blocks[sb.root_pos].ent=new DirectoryEntry[(int)(block_size*1024/sizeof(DirectoryEntry))];
        strcpy(this->blocks[sb.root_pos].ent[0].name,"\\");
        this->blocks[sb.root_pos].ent[0].size=0;
        this->blocks[sb.root_pos].ent[0].start_block=sb.root_pos;
        strcpy(this->blocks[sb.root_pos].ent[0].attr,"D");
        get_local_date(this->blocks[sb.root_pos].ent[0].date);
        get_local_time(this->blocks[sb.root_pos].ent[0].time);
    }
}

FS::~FS(){
    delete this->table;
    delete this->sb.free_blocks;
}

void FS::save_file_system(char * filename){
    int fd = open(filename,O_RDWR,0666);
    write(fd,&this->sb.block_size,sizeof(double));
    write(fd,&this->sb.root_pos,sizeof(unsigned short int));
    write(fd,&this->sb.free_size,sizeof(unsigned short int));
    int i=0;
    for(i=0;i<TOTAL_BLOCKS;i++){
        write(fd,&this->sb.free_blocks[i],sizeof(bool));
    }
    off_t current_location = lseek(fd,0,SEEK_CUR);
    lseek(fd,((current_location/(sb.block_size*1024))+1)*sb.block_size*1024,SEEK_SET);
    for(i=0;i<TOTAL_BLOCKS;i++){
        write(fd,&this->table[i],sizeof(unsigned short int));
    }
    current_location = lseek(fd,0,SEEK_CUR);
    lseek(fd,((current_location/(sb.block_size*1024))+1)*sb.block_size*1024,SEEK_SET);
    for(i=0;i<(int)(sb.block_size*1024/sizeof(DirectoryEntry));i++){
        write(fd,&this->blocks[sb.root_pos].ent[i],sizeof(DirectoryEntry));
    }
    close(fd);
}

void get_local_time(char * string){
    time_t time_type = time(NULL);
    struct tm *time_tm = localtime(&time_type);
    strftime(string,6,"%H:%M",time_tm);
}

void get_local_date(char * string){
    time_t time_type = time(NULL);
    struct tm *time_tm = localtime(&time_type);
    strftime(string,10,"%d.%m.%y",time_tm);
}