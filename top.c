#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <pwd.h>

#include<sys/utsname.h>
#include<utmp.h>


extern int errno;
void read_dir(char *);
void getCurrentTime();
void getTimeSinceBoot();
void getUserCount();
void getLoadAverage();
void loadavg();

int main(int argc, char *argv[]){
	printf("top - ");
	getCurrentTime();
	getTimeSinceBoot();
	getUserCount();
	loadavg();

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