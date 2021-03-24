#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <utmp.h>
#include <ctype.h>

extern int errno;
void read_dir(char *);
void getCurrentTime();
void getTimeSinceBoot();
void getUserCount();
void getLoadAverage();
void loadavg();
void getProcessesCount();

int main(int argc, char *argv[]){
	printf("top - ");
	getCurrentTime();
	getTimeSinceBoot();
	getUserCount();
	loadavg();
	getProcessesCount();

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
    printf(" up %d:%d,  ",hour,min);
}

void getUserCount() {
  	struct utmp * loggedUsers;
  	setutent();
  	loggedUsers = getutent();
  	int userCount = 0;
  	while (loggedUsers) {
	    if (loggedUsers -> ut_type == USER_PROCESS) {
    	  userCount++;
    	}
    loggedUsers = getutent();
  	}
  	printf("%d user,  ", userCount);
}

void loadavg(void){
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}
	printf("load average: %.2f, %.2f, %.2f \n", avgs[0], avgs[1], avgs[2]);	
}

void getProcessesCount() {
  int totalProcesses = 0, running = 0, sleeping = 0, stopped = 0, zombie = 0;
  char dirName[100];
  char name[100];
  char state;
  
  long pid;
  FILE * fp = NULL;  
  struct dirent * entry;

   DIR* dp = opendir("/proc");
   errno = 0;
   while(1){
      entry = readdir(dp);
      if(entry == NULL && errno != 0){
         perror("error while reading proc directory");
         exit(errno);
      }
      if(entry == NULL && errno == 0){
         break;         
      }
      long lpid = atol(entry -> d_name);
      if (entry -> d_type == DT_DIR) {
        if (isdigit(entry -> d_name[0])) {
          totalProcesses++;
          snprintf(dirName, sizeof(dirName), "/proc/%ld/stat", lpid);          
          fp = fopen(dirName, "r");

          if (fp) {
            fscanf(fp, "%ld %s %c", &pid, name, &state);
            switch (state) {
            	case 'R':
              		running++;
              		break;
            	case 'S':
            	case 'D':
            	case 'I':
              		sleeping++;
              		break;
            	case 'T':
              		stopped++;
              		break;
            	case 'Z':
              		zombie++;
              		break;
            	default:
              		break;
            }
            fclose(fp);
          }
        }
      }
   }
  closedir(dp);
  printf("Tasks: %d total,  %d running,  %d sleeping,  %d stopped,  %d zombie\n", totalProcesses, running, sleeping, stopped, zombie);
}