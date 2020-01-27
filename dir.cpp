#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <string.h>
using namespace std;

void listdir(const char *path, vector<string>& children){
    DIR         *ptr_dir;
    struct dirent   *dir_entry;
    ptr_dir = opendir(path);
    while((dir_entry = readdir(ptr_dir)) != NULL){
        if(dir_entry->d_type & DT_DIR){
            if(strcmp(dir_entry->d_name,".") == 0 ||
               strcmp(dir_entry->d_name,"..") == 0){
                continue;
            }
			children.emplace_back(dir_entry->d_name);
        }
    }
	sort(children.begin(), children.end());
}

void del_dir(const char *npath)
{
       char pathname[512];
       struct dirent *ent =NULL;
        DIR * ptr;
       DIR *dpin;
        ptr = opendir(npath);
        while((ent = readdir(ptr)) != NULL)
        {
             if(strcmp(ent->d_name,".")==0||strcmp(ent->d_name,"..")==0)
                  continue;
             strcpy(pathname,npath);
             strcat(pathname,"/");
             strcat(pathname,ent->d_name);
                   if(ent->d_type & DT_DIR)
                   {
                          del_dir(pathname);
                           remove(pathname);
                   }
                   else
                    {
                        remove(pathname);
                    }
        }
        remove(npath);
}

/*
int main(int argc, char* argv[]){
	del_dir("/home/ubuntu/temp/testrm");
    return 0;
}  */