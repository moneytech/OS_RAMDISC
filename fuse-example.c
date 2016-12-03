//based on http://engineering.facile.it/blog/eng/write-filesystem-fuse/



#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
static const char *filepath = "/file";
static const char *filename = "file";
static char* filecontent;
static int filesize=0;
struct dir
{
	int dir_size;
	char **filenames; // 2D array contains files
	struct file
	{
		char *data;
		struct stat *stat; 
	} *filecontent;
}root_dir;


int get_id_from_path(char* path) // return adress in filecontent array based on name -1 mean no file founded;
{
	char *fname;
	fname=(char*)malloc(strlen(path));
	memcpy(fname,path+1,strlen(path));
	for (int i=0; i<root_dir.dir_size;i++)
	{
		if(strcmp(root_dir.filenames[i],fname)==0)
		{
			// we got file return id of it
			return i;
		}
	}
	return -1;
}

int add_file



static int getattr_callback(const char *path, struct stat *stbuf) {
  memset(stbuf, 0, sizeof(struct stat));

  if (strcmp(path, "/") == 0) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    return 0;
  }

  if (strcmp(path, filepath) == 0) {
    stbuf->st_mode = S_IFREG | 0757;
    stbuf->st_nlink = 1;
    stbuf->st_size = filesize;
    return 0;
  }

  return -ENOENT;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi) 
{
	(void) offset;
	(void) fi;
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, filename, NULL, 0);
	return 0;
}

static int open_callback(const char *path, struct fuse_file_info *fi) 
{
	return 0;
}

//static int setattr_callback(



static int truncate_callback(const char *path,off_t size) // change file size 
{
	
	if (size==0)
	{

		printf("FILE TRUNKATED TO:%ld bytes ",size);
		printf("Just emualte trucate\n");
		free(filecontent);
		filecontent=(char*)malloc(1);
		filesize=0;
		return 0;

	}
	filecontent=(char*)realloc((void*)filecontent,size);
	filesize=size;
	printf("FILE TRUNKATED TO:%ld bytes",size);
	if (filecontent==NULL)
	{
		printf(" and filepointer is NULL\n");
		return -EIO;
	}  
	printf ("\n");
	return 0;
}

static int read_callback(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi) 
{

	printf("CURRENT PATH:%s \n",path);
  if (strcmp(path, filepath) == 0) {
    size_t len = filesize;
    if (offset >= len) {
      return 0;
    }

    if (offset + size > len) {
      memcpy(buf, filecontent + offset, len - offset);
      return len - offset;
    }

    memcpy(buf, filecontent + offset, size);
    return size;
  }

  return -ENOENT;
}

static int flush_callback ( const char* path, struct fuse_file_info* fi)
{
	return 0;
}

static int write_callback(const char *path,const char *data,size_t data_size, off_t offset, struct fuse_file_info *fi)
{
	if(strcmp(path,filepath)==0 && filecontent != NULL )
	{
			
		
		printf("WWWWW s:%d o:%ld fs:%d\n",data_size,offset,filesize);
		if(filesize < offset+data_size )
		{
			truncate_callback(path,offset+data_size);
		} 
		memcpy((char*)(filecontent+offset),data,data_size);
		return data_size;
	
	//	return -ENOSPC; // no left space	
	}
	return -ENOENT;
}

static struct fuse_operations fuse_example_operations = {
	.getattr = getattr_callback,
	.open = open_callback,
	.read = read_callback,
	.write = write_callback,
	.readdir = readdir_callback,
	.truncate = truncate_callback,
	.flush = flush_callback,
	
};

int main(int argc, char *argv[])
{
	
	//root_dir.filenames=realloc(root_dir.filenames,2);

	filecontent=(char*)malloc(6);
	filecontent[0]='O';
	filecontent[1]='K';
	filecontent[2]='\n';
	filecontent[3]='\r';
	filecontent[4]=0;
	filesize=6;
  return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
