#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>

extern int errno;
void read_dir(char * );
void getCurrentTime();
void getTimeSinceBoot();

int main(int argc, char *argv[]){
	//read_dir("/proc/");
	printf("top - ");
	getCurrentTime();
	getTimeSinceBoot();
	printf("\n");
	return 0;
}

void getCurrentTime(){
	time_t currentTime = time(0);
  	struct tm localTime = *localtime(&currentTime);
   	printf("%02d:%02d:%02d",localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
}

void getTimeSinceBoot(){
	int hour, min, upTimeInSec;
	struct sysinfo s_info;
    int error = sysinfo(&s_info);
    if(error != 0)
    {
        printf("code error = %d\n", error);
    }
    upTimeInSec = s_info.uptime;
    hour = (upTimeInSec / 3600);
  	min = (upTimeInSec - (3600 * hour)) / 60;
    printf(" up %d:%d",hour,min);
}

void read_dir(char *dir){	
   DIR* dp = opendir(dir);
   errno = 0;
   struct dirent* entry;
   while(1){
      entry = readdir(dp);
      if(entry == NULL && errno != 0){
         perror("readdir");
         exit(errno);
      }
      if(entry == NULL && errno == 0){
         printf("\nEnd of directory\n");
         break;         
      }
      printf("%s   ",entry->d_name);
   }
   closedir(dp);
   return;
}