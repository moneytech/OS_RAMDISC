 //based on http://engineering.facile.it/blog/eng/write-filesystem-fuse/
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <utime.h>
#include <time.h>
void statcpy(struct stat* to,struct stat* from)
{

	if (from==NULL || to==NULL) return;
	to->st_dev=from->st_dev;
	to->st_ino=from->st_ino;
	to->st_mode=from->st_mode;
	to->st_nlink=from->st_nlink;
	to->st_uid=from->st_uid;
	to->st_gid=from->st_gid;
	to->st_rdev=from->st_rdev;
	to->st_size=from->st_size;
	to->st_blksize=from->st_blksize;
	to->st_blocks=from->st_blocks;
	to->st_atime=from->st_atime;
	to->st_mtime=from->st_mtime;
	to->st_ctime=from->st_ctime;
	return;
}

struct file
{
	char *name;
	char *data;
	struct stat *stat; 
};


int dir_size;
struct file** files;



int add_file(const char* path, struct stat* stat_in)
{
	printf("ADD FILE: %s\n",path);
	printf("DIR SIZE: %d\n",dir_size);
	dir_size++;
	files=(struct file**) realloc(files,sizeof(struct file*)*dir_size);
	if (files==NULL)
	{
		printf("I/O ERROR files==NULL\n");	
		return -EIO;
	}
	files[dir_size-1]=(struct file*) malloc(sizeof(struct file));
	files[dir_size-1]->stat= (struct stat*) malloc( sizeof(struct stat) );
	files[dir_size-1]->name=(char*) malloc(sizeof(char)* (strlen(path)+1)); 
	files[dir_size-1]->data=(char*) malloc(sizeof(char));
	strcpy(files[dir_size-1]->name,path);
	statcpy(files[dir_size-1]->stat,stat_in);
	return 0;
}
struct file* get_file (const char* path) // NULL mean no file found
{
	printf("GET FILE BY NAME: %s\n",path);
	int i;
	if (strcmp(path,"/") == 0) // if we call . as /
	{
		return get_file("/.");
	}
	for (i=0;i<dir_size;i++)
	{
		if(strcmp(files[i]->name,path+1)==0)
		{
			return files[i];

		}
	}
	return NULL;
}
struct file* get_file_by_id(int id)
{
	if (id>=0 && id<dir_size)
	 return files[id];
	else return NULL;
}


int del_file(const char* path)
{
	printf("DELETE FILE %s",path);
	if (strcmp(path,"/")==0)
	{
		return -EACCES;
	}
	int i;
	for (i=0;i<dir_size;i++)
	{
		if (strcmp(files[i]->name,path+1)==0)
		{
			break;
		}
	}
	printf("FILE ID IS: %d\n",i); 
	free(files[i]->data);
	printf("DATA REMOVED\n");
	free(files[i]->name);
	printf("NAME REMOVED\n");
	free(files[i]->stat);
	printf("STAT REMOVED\n");
	free(files[i]);
	printf("FILE REMOVED\n");
	files[i]=files[dir_size-1];
	printf("CHAIN CHANGED\n");
	dir_size--;
	files=(struct file**) realloc(files,sizeof(struct file*)*dir_size);
	printf("DIR REALOC SIZE %d\n",dir_size);

	return 0;	
}

int init()
{
	dir_size=0;
	files=NULL;
	struct stat f;
	struct stat d;
	struct fuse_context *con;
	con=fuse_get_context();
	d.st_uid=con->uid;
	d.st_gid=con->gid;
	f.st_uid=con->uid;
	f.st_gid=con->gid;
	f.st_size=0;
	d.st_size=sizeof(files);
	d.st_nlink=1;
	f.st_nlink=1;
	d.st_mode=S_IFDIR | 0777;
	f.st_mode=S_IFREG | 0777;
	add_file(".",&d);
	add_file("..",NULL);
	add_file("autor.txt",&f);
	
	//1. add . and .. dir
	//2. add autor.txt file 
	//3. add "Jakub Staniszewski 149559" into file 
	
	return 0;
}


static int getattr_callback(const char *path, struct stat *stbuf) 
{
	memset(stbuf, 0, sizeof(struct stat));
	struct file* curr_file;
	curr_file=get_file(path);
	if(curr_file==NULL)
	{
		return -ENOENT;
	}
	statcpy(stbuf,curr_file->stat);
	return 0;
}

static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi) 
{
	(void) offset;
	(void) fi;
	int i=0;
	struct file* curr_file;
	for(i=0;i<dir_size;i++)
	{
		curr_file=get_file_by_id(i);

		if (curr_file == NULL ) return -EIO; 

		printf("READ DIR-> FILE %s\n",curr_file->name);
		filler( buf, /* one function in multiple lines*/
			curr_file->name,
			curr_file->stat,
			0);	
	}
	return 0;
}

static int open_callback(const char *path, struct fuse_file_info *fi) 
{
	
	return 0;
}
static int truncate_callback(const char *path,off_t size) // change file size 
{
	
	printf("TRUCACE FILE: %s \n",path);
	struct file* curr_file;
	curr_file=get_file(path);
	if (curr_file==NULL)
	{
		return -ENOENT;
	}
	if (size==0)
	{

		printf("FILE TRUNKATED TO:%ld bytes ",size);
		printf("Just emualte trucate\n");
		free(curr_file->data);
		curr_file->data=(char*)malloc(1);
		curr_file->stat->st_size=0;
		return 0;

	}
	curr_file->data=(char*)realloc((void*)curr_file->data,size);
	curr_file->stat->st_size=size;
	printf("FILE TRUNKATED TO:%ld bytes",size);
	return 0;
}
static int read_callback(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi) 
{
	
	printf("READ FROM:%s \n",path);
	struct file *curr_file;
	curr_file=get_file(path);
	if (curr_file !=NULL ) 
	{
		size_t len = curr_file->stat->st_size;
		if (offset >= len) 
		{
			return 0;
		}
		if (offset + size > len) 
		{
			memcpy(buf, curr_file->data+offset, len - offset);
			return len - offset;
		}
	memcpy(buf, curr_file->data+offset, size);
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
	printf("WRITE TO %s\n",path);
	/* here we shoud test premission ??*/
	struct file *curr_file;
	curr_file=get_file(path);
	if(curr_file != NULL )
	{
		if(curr_file->stat->st_size  < offset+data_size )
		{
			truncate_callback(path,offset+data_size);
		} 
		memcpy((char*)(curr_file->data+offset),data,data_size);
		return data_size;
	
	}

	return -ENOENT;
}
static int mknode_callback(const char* path, mode_t mode,dev_t dev)
{
	//if mode is not file return not implemented
	struct stat tmp;
	tmp.st_dev=dev;
	tmp.st_mode=mode;
	tmp.st_size=0;
	struct fuse_context *con;
	con=fuse_get_context();
	tmp.st_uid=con->uid;
	tmp.st_gid=con->gid;
	tmp.st_nlink=1;
	
	add_file(path+1,&tmp); // +1 for skip /
	return 0; 		
}
static int chmod_callback(const char* path, mode_t mode)
{
	struct file* curr_file;
	curr_file=get_file(path);
	if (curr_file == NULL) 
	{
		return -ENOENT;
	}
	curr_file->stat->st_mode=mode;
	return 0;

}
static int chown_callback(const char* path, uid_t uid,gid_t gid)
{
	struct file* curr_file;
	curr_file=get_file(path);
	if (curr_file == NULL) 
	{
		return -ENOENT;
	}
	curr_file->stat->st_uid=uid;
	curr_file->stat->st_gid=gid;
	return 0;
}

static int utime_callback(const char* path, struct utimbuf* time)
{
	struct file* curr_file;
	curr_file=get_file(path);
	if (curr_file == NULL) 
	{
		return -ENOENT;
	}
	curr_file->stat->st_mtim.tv_sec=time->modtime;
	curr_file->stat->st_atim.tv_sec=time->actime;
	return 0;

}

static int ulink_callback(const char* path)
{
	return del_file(path);
}


static int rename_callback(const char* from, const char* to)
{

	printf ("RENAME %s TO %s \n",from,to);
	struct file* curr_file;
	curr_file=get_file(from);
	if (curr_file == NULL) 
	{
		return -ENOENT;
	}
	free(curr_file->name);
	curr_file->name=(char*) malloc (sizeof(char) * ( strlen(to) +1));
	strcpy(curr_file->name,to+1);  // +1 for skip `/`

	return 0;
}
static struct fuse_operations fuse_example_operations = 
{
	.getattr = getattr_callback, //[OK] 
	.open = open_callback, //not implented	
	.read = read_callback, // [OK]
	.write = write_callback, // [OK]
	.readdir = readdir_callback, // [OK]
	.truncate = truncate_callback, // [OK]
	.flush = flush_callback,   // not use 
	.rename = rename_callback, // [OK]
	.mknod = mknode_callback, //[OK]
	.chmod = chmod_callback, // [OK]
	.chown = chown_callback, //[OK]
	.utime = utime_callback, //[OK]
	.unlink = ulink_callback // Remove File


};

int main(int argc, char *argv[])
{

	init();

	return fuse_main(argc, argv, &fuse_example_operations, NULL);
}
