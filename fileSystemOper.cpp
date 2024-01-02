#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "filesystem.h"
#define TOTAL_BLOCKS 4096

void get_local_time(char * string);
void get_local_date(char * string);

int main(int argc,char ** argv){
    char fs_name[256];
    char operation[256];
    char parameters1[256];
    char parameters2[256];
    sscanf(argv[1],"%s",fs_name);
    sscanf(argv[2],"%s",operation);
    FS * file_sys = new FS();
    file_sys->load_file_system(fs_name);
    if(strcmp(operation,"dir")==0){
        sscanf(argv[3],"%s",parameters1);
        file_sys->dir_f(parameters1);
    }
    else if(strcmp(operation,"mkdir")==0){
        sscanf(argv[3],"%s",parameters1);
        int i=0;
        int index=0;
        int occur=0;
        do{
            if(parameters1[i]=='\\'){
                occur++;
                index=i;
            }
            i++;
        }
        while(parameters1[i]!='\0');
        char * dirname = &parameters1[index+1];
        if(occur==1){
            char root[5];
            strcpy(root,"\\");
            file_sys->mkdir_f(root,dirname);
        }
        else{
            parameters1[index]='\0';
            file_sys->mkdir_f(parameters1,dirname);
        }
        std::cout<<"Directory created successfully.\n";  
    }
    else if(strcmp(operation,"rmdir")==0){
        sscanf(argv[3],"%s",parameters1);
        int i=0;
        int index=0;
        int occur=0;
        do{
            if(parameters1[i]=='\\'){
                occur++;
                index=i;
            }
            i++;
        }
        while(parameters1[i]!='\0');
        char * dirname = &parameters1[index+1];
        if(occur==1){
            char root[5];
            strcpy(root,"\\");
            file_sys->rmdir_f(root,dirname);
            exit(EXIT_FAILURE);
        }
        else{
            parameters1[index]='\0';
            file_sys->rmdir_f(parameters1,dirname);
        }
        std::cout<<"Directory removed successfully.\n";  
    }
    else if(strcmp(operation,"dumpe2fs")==0){
        file_sys->dumpe2fs_f();
    }
    else if(strcmp(operation,"write")==0){
        sscanf(argv[3],"%s",parameters1);
        sscanf(argv[4],"%s",parameters2);
        int i=0;
        int index=0;
        int occur=0;
        do{
            if(parameters1[i]=='\\'){
                occur++;
                index=i;
            }
            i++;
        }
        while(parameters1[i]!='\0');
        char * filename = &parameters1[index+1];
        if(occur==1){
            char root[5];
            strcpy(root,"\\");
            file_sys->write_f(root,filename,parameters2);
        }
        else{
            parameters1[index]='\0';
            file_sys->write_f(parameters1,filename,parameters2);
        }
        std::cout<<"File written successfully.\n";  
    }
    else if(strcmp(operation,"read")==0){
        sscanf(argv[3],"%s",parameters1);
        sscanf(argv[4],"%s",parameters2);
        int i=0;
        int index=0;
        int occur=0;
        do{
            if(parameters1[i]=='\\'){
                occur++;
                index=i;
            }
            i++;
        }
        while(parameters1[i]!='\0');
        char * filename = &parameters1[index+1];
        if(occur==1){
            char root[5];
            strcpy(root,"\\");
            file_sys->read_f(root,filename,parameters2);
        }
        else{
            parameters1[index]='\0';
            file_sys->read_f(parameters1,filename,parameters2);
        }
        std::cout<<"File read successfully.\n";  
    }
    else if(strcmp(operation,"del")==0){
        sscanf(argv[3],"%s",parameters1);
        int i=0;
        int index=0;
        int occur=0;
        do{
            if(parameters1[i]=='\\'){
                occur++;
                index=i;
            }
            i++;
        }
        while(parameters1[i]!='\0');
        char * filename = &parameters1[index+1];
        if(occur==1){
            char root[5];
            strcpy(root,"\\");
            file_sys->del_f(root,filename);
        }
        else{
            parameters1[index]='\0';
            file_sys->del_f(parameters1,filename);
        }
        std::cout<<"File deleted successfully.\n";        
    }
    else{
        std::cerr<<"Unknown operation.\n";
        exit(EXIT_FAILURE);
    }
    file_sys->save_file_system(fs_name);
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

FS::FS(){
    this->table = new unsigned short int[TOTAL_BLOCKS];
    sb.free_blocks = new bool[TOTAL_BLOCKS];
}

FS::~FS(){
    delete this->table;
    delete this->sb.free_blocks;
}

void FS::recursive_traverse(unsigned short int index,int fd,int command){
    int j=1;
    while(blocks[index].ent[j].start_block!=0){
        if(strcmp(blocks[index].ent[j].attr,"D") == 0){
            int index2=blocks[index].ent[j].start_block;
            if(command==0){
                this->blocks[index2].ent=new DirectoryEntry[(int)(sb.block_size*1024/sizeof(DirectoryEntry))];
                int i=0;
                lseek(fd,index2*sb.block_size*1024,SEEK_SET);
                for(i=0;i<(int)(sb.block_size*1024/sizeof(DirectoryEntry));i++){
                    read(fd,&this->blocks[index2].ent[i],sizeof(DirectoryEntry));
                }
                recursive_traverse(index2,fd,0);
            }
            else if(command==1){
                int i=0;
                lseek(fd,index2*sb.block_size*1024,SEEK_SET);
                for(i=0;i<(int)(sb.block_size*1024/sizeof(DirectoryEntry));i++){  
                    write(fd,&this->blocks[index2].ent[i],sizeof(DirectoryEntry));
                }
                recursive_traverse(index2,fd,1);
            }
        }
        else if(strcmp(blocks[index].ent[j].attr,"F") == 0){
            int index2=blocks[index].ent[j].start_block;
            if(command==0){
                lseek(fd,index2*sb.block_size*1024,SEEK_SET);
                this->blocks[index2].file=new char[(int)(sb.block_size*1024)];
                if(blocks[index].ent[j].size<sb.block_size*1024){
                    read(fd,this->blocks[index2].file,sizeof(char)*(blocks[index].ent[j].size));
                }
                else{
                    read(fd,this->blocks[index2].file,sizeof(char)*(sb.block_size*1024));
                    while(this->table[index2]!=(unsigned short int)-1){
                        index2=table[index2];
                        read(fd,this->blocks[index2].file,sizeof(char)*(sb.block_size*1024));
                    }
                }
            }
            else if(command==1){
                lseek(fd,index2*sb.block_size*1024,SEEK_SET);
                if(blocks[index].ent[j].size<sb.block_size*1024){
                    write(fd,this->blocks[index2].file,sizeof(char)*(blocks[index].ent[j].size));
                }
                else{
                write(fd,this->blocks[index2].file,sizeof(char)*(sb.block_size*1024));
                    while(this->table[index2]!=(unsigned short int)-1){
                        index2=table[index2];
                        write(fd,this->blocks[index2].file,sizeof(char)*(sb.block_size*1024));
                    }
                }
            }
            
        }
        j++;
    }
}

void FS::allocate_new_block(unsigned short int index,char * attr){
    int i=0;
    do{
        if(sb.free_blocks[i]==true){
            sb.free_blocks[i]=false;
            sb.free_size--;
            this->table[i]=-1;
            this->table[index]=i;
            if(strcmp(attr,"D")==0){
                this->blocks[i].ent=new DirectoryEntry[(int)(sb.block_size*1024/sizeof(DirectoryEntry))];
            }
            else{
                this->blocks[i].file=new char[(int)(sb.block_size*1024)];
            }
            break;
        }
        i++;
    }
    while(i<TOTAL_BLOCKS);
}

void FS::load_file_system(char * filename){
    int fd = open(filename,O_RDWR,0666);
    if(fd==-1){
        perror("Open error");
        exit(EXIT_FAILURE);
    }
    strcpy(this->filename,filename);
    read(fd,&this->sb.block_size,sizeof(double));
    read(fd,&this->sb.root_pos,sizeof(unsigned short int));
    read(fd,&this->sb.free_size,sizeof(unsigned short int));
    int i=0;
    for(i=0;i<TOTAL_BLOCKS;i++){
        read(fd,&this->sb.free_blocks[i],sizeof(bool));
    }
    off_t current_location = lseek(fd,0,SEEK_CUR);
    lseek(fd,((current_location/(sb.block_size*1024))+1)*sb.block_size*1024,SEEK_SET);
    for(i=0;i<TOTAL_BLOCKS;i++){
        read(fd,&this->table[i],sizeof(unsigned short int));
    }
    this->blocks=new Block[TOTAL_BLOCKS];
    this->blocks[sb.root_pos].ent=new DirectoryEntry[(int)(sb.block_size*1024/sizeof(DirectoryEntry))];
    current_location = lseek(fd,0,SEEK_CUR);
    lseek(fd,((current_location/(sb.block_size*1024))+1)*sb.block_size*1024,SEEK_SET);
    for(i=0;i<(int)(sb.block_size*1024/sizeof(DirectoryEntry));i++){
        read(fd,&this->blocks[sb.root_pos].ent[i],sizeof(DirectoryEntry));
    }
    recursive_traverse(sb.root_pos,fd,0);
    close(fd);
}

unsigned short int FS::traverse_path(char * path){
    char temp[100];
    char c;
    int i=0;
    int k=1;
    unsigned short int curr_block=this->sb.root_pos;
    if(strcmp(path,"\\")!=0){
        do{
            int flag=0;
            c=path[k];
            temp[i]=c;
            i++;
            k++;
            if(c=='\\' || c=='\0'){
                temp[i-1]='\0';
                int j=1;
                while(blocks[curr_block].ent[j].start_block!=0){
                    if(strcmp(blocks[curr_block].ent[j].name,temp) == 0){
                        curr_block=blocks[curr_block].ent[j].start_block;
                        flag=1;
                        break;
                    }
                    j++;
                }
                if(flag==0){
                    std::cerr<<"There is no directory as: "<<path<<"\n";
                    exit(EXIT_FAILURE);
                }
                j=1;
                i=0;
            }
        }
        while(c!='\0');
    }
    return curr_block;
}

void FS::dir_f(char * path){
    unsigned short int curr_block = traverse_path(path);
    int j=1;
    std::cout<<"Inside \""<<path<<"\" directory:\n";
    while(blocks[curr_block].ent[j].start_block!=0){
        std::cout<<blocks[curr_block].ent[j].date<<" "<<blocks[curr_block].ent[j].time<<" "<<blocks[curr_block].ent[j].attr<<" "<<blocks[curr_block].ent[j].size<<" "<<blocks[curr_block].ent[j].name<<"\n";
        j++;
    }
    if(j==1){
        std::cout<<"Empty.\n";
    }
}

unsigned short int FS::mkdir_f(char * path,char * dirname){
    unsigned short int curr_block = traverse_path(path);
    int j=1;
    while(blocks[curr_block].ent[j].start_block!=0){
        if(strcmp(blocks[curr_block].ent[j].name,dirname)==0){
            std::cerr<<"The directory already created.\n";
            exit(EXIT_FAILURE);
        }
        j++;
        if(j>=(int)(sb.block_size*1024/sizeof(DirectoryEntry)) && table[curr_block]!=-1){
            curr_block=table[curr_block];
            j=1;
        }
    }
    if(j>=(int)(sb.block_size*1024/sizeof(DirectoryEntry))){
        char attr[5];
        strcpy(attr,"D");
        allocate_new_block(curr_block,attr);
    }
    int i=0;
    do{
        if(sb.free_blocks[i]==true){
            sb.free_blocks[i]=false;
            sb.free_size--;
            this->table[i]=-1;
            this->blocks[i].ent=new DirectoryEntry[(int)(sb.block_size*1024/sizeof(DirectoryEntry))];
            strcpy(this->blocks[i].ent[0].name,dirname);
            this->blocks[i].ent[0].size=0;
            this->blocks[i].ent[0].start_block=i;
            strcpy(this->blocks[i].ent[0].attr,"D");
            get_local_date(this->blocks[i].ent[0].date);
            get_local_time(this->blocks[i].ent[0].time);
            break;
        }
        i++;
    }
    while(i<TOTAL_BLOCKS);
    strcpy(this->blocks[curr_block].ent[j].name,dirname);
    this->blocks[curr_block].ent[j].size=0;
    this->blocks[curr_block].ent[j].start_block=i;
    strcpy(this->blocks[curr_block].ent[j].attr,"D");
    get_local_date(this->blocks[curr_block].ent[j].date);
    get_local_time(this->blocks[curr_block].ent[j].time);

    return curr_block;
}

void FS::rmdir_f(char * path,char * dirname){
    unsigned short int curr_block = traverse_path(path);
    int j=1;
    while(blocks[curr_block].ent[j].start_block!=0){
        if(strcmp(blocks[curr_block].ent[j].name,dirname)==0){
            break;
        }
        j++;
        if(j>=(int)(sb.block_size*1024/sizeof(DirectoryEntry)) && table[curr_block]!=-1){
            curr_block=table[curr_block];
            j=1;
        }
    }
    if(j>=(int)(sb.block_size*1024/sizeof(DirectoryEntry)) && table[curr_block]!=-1){
        std::cerr<<"There is no directory named: "<<dirname<<" in path: "<<path<<"\n";
        exit(EXIT_FAILURE);
    }
    unsigned short int start=blocks[curr_block].ent[j].start_block;
    blocks[curr_block].ent[j].start_block=0;
    unsigned short int end=start;
    while(table[end]!=(unsigned short int)-1){
        sb.free_blocks[end]=true;
        start=table[end];
        table[end]=-2;
        end=start;
    }
    table[end]=-2;
    blocks[curr_block].ent[j].start_block=0;
}
void FS::dumpe2fs_f(){
    std::cout<<"Block size:"<<sb.block_size<<"\n";
    std::cout<<"Total number of blocks:"<<TOTAL_BLOCKS<<"\n";
    std::cout<<"Free size:"<<sb.free_size<<"\n";
    std::cout<<"Root block position:"<<sb.root_pos<<"\n";
    char root[5];
    strcpy(root,"\\");
    dir_f(root);
}
unsigned short int FS::write_f(char * path,char * filename,char * file){
    unsigned short int curr_block = traverse_path(path);
    int j=1;
    while(blocks[curr_block].ent[j].start_block!=0){
        if(strcmp(blocks[curr_block].ent[j].name,filename)==0){
            std::cerr<<"The file already created.\n";
            exit(EXIT_FAILURE);
        }
        j++;
    }
    int fd1=open(file,O_RDONLY,0666);
    if(fd1==-1){
        perror("Open error");
        exit(EXIT_FAILURE);
    }
    int i=0;
    off_t file_size=lseek(fd1,0,SEEK_END);
    lseek(fd1,0,SEEK_SET);
    int file_block=0;
    int fp=0;
    int start_index=0;
    int prev_index=curr_block;
    if(file_size/(sb.block_size*1024)>0){
        file_block=file_size/(sb.block_size*1024);
    }
    do{
        if(sb.free_blocks[i]==true){
            if(start_index==0){
                start_index=i;
            }
            sb.free_blocks[i]=false;
            sb.free_size--;
            this->table[prev_index]=i;
            this->table[i]=-1;
            this->blocks[i].file=new char[(int)(sb.block_size*1024)];
            read(fd1,this->blocks[i].file,sb.block_size*1024);
            if(fp>=file_block){
                break;
            }
            prev_index=i; 
        }
        i++;
    }
    while(i<TOTAL_BLOCKS);
    strcpy(this->blocks[curr_block].ent[j].name,filename);
    this->blocks[curr_block].ent[j].size=file_size;
    this->blocks[curr_block].ent[j].start_block=start_index;
    strcpy(this->blocks[curr_block].ent[j].attr,"F");
    get_local_date(this->blocks[curr_block].ent[j].date);
    get_local_time(this->blocks[curr_block].ent[j].time);
    close(fd1);

    return curr_block;
}
void FS::read_f(char * path,char * filename,char * file){
    unsigned short int curr_block = traverse_path(path);
    int fd1=open(file,O_WRONLY | O_CREAT,0666);
    if(fd1==-1){
        perror("Open error");
        exit(EXIT_FAILURE);
    }
    int j=1;
    while(blocks[curr_block].ent[j].start_block!=0){
        if(strcmp(blocks[curr_block].ent[j].name,filename)==0){
            if(blocks[curr_block].ent[j].size<(sb.block_size*1024)){
                write(fd1,this->blocks[blocks[curr_block].ent[j].start_block].file,sizeof(char)*blocks[curr_block].ent[j].size);
            }
            else{
                write(fd1,this->blocks[blocks[curr_block].ent[j].start_block].file,sb.block_size*1024*sizeof(char));
            }
            break;
        }
        j++;
    }
    int table_index=blocks[curr_block].ent[j].start_block;
    while(table[table_index]!=(unsigned short int)-1){
        table_index=table[table_index];
        write(fd1,this->blocks[table_index].file,sb.block_size*1024);
    }
    close(fd1);
}
void FS::del_f(char * path,char * filename){
    unsigned short int curr_block = traverse_path(path);
    int j=1;
    while(blocks[curr_block].ent[j].start_block!=0){
        if(strcmp(blocks[curr_block].ent[j].name,filename)==0){
            break;
        }
        j++;
        if(j>=(int)(sb.block_size*1024/sizeof(DirectoryEntry)) && table[curr_block]!=-1){
            curr_block=table[curr_block];
            j=1;
        }
    }
    if(j>=(int)(sb.block_size*1024/sizeof(DirectoryEntry)) && table[curr_block]!=-1){
        std::cerr<<"There is no file named: "<<filename<<" in path: "<<path<<"\n";
        exit(EXIT_FAILURE);
    }
    unsigned short int start=blocks[curr_block].ent[j].start_block;
    blocks[curr_block].ent[j].start_block=0;
    unsigned short int end=start;
    while(table[end]!=(unsigned short int)-1){
        sb.free_blocks[end]=true;
        start=table[end];
        table[end]=-2;
        end=start;
    }
    table[end]=-2;
    blocks[curr_block].ent[j].start_block=0;
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
    recursive_traverse(sb.root_pos,fd,1);
}