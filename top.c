#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include<utmp.h>

extern int errno;

char* getCurrentTime();
char* getTimeSinceBoot();
char* getUserCount();
char* getLoadAverage();
char* loadavg();

int main(int argc, char *argv[]){
	//read_dir("/proc/");
	printf("top - ");
	char* currentTime = getCurrentTime();
	getTimeSinceBoot();
	getUserCount();
	char* load = loadavg();
	printf("%s", load);

	printf("\n");
	return 0;
}

char* getCurrentTime(){
	char *curTime = malloc(10);
	time_t currentTime = time(0);
  	struct tm localTime = *localtime(&currentTime);
   	sprintf(curTime,"%02d:%02d:%02d",localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
	return curTime;
}

char* getTimeSinceBoot(){
	char *upTime = malloc(5);
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
    sprintf(upTime,"%d:%d",hour,min);
	return upTime;
}

char* getUserCount() {
	char *upTime = malloc(5);
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
  	printf(",  %d user", userCount);
}

char* getLoadAvg(void){
	double avgs[3];
	char *str = malloc(15);

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}
	sprintf(str,"%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
	return str;
}