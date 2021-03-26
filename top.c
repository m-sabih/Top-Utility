#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <utmp.h>
#include <ctype.h>
#include <pwd.h>
#include <termios.h>
#include <fcntl.h>

extern int errno;
void top();
void read_dir(char *);
void getCurrentTime();
void getTimeSinceBoot();
void getUserCount();
void getLoadAverage();
void loadavg();
void getProcessesCount();
void getCpuUsage();
void tty_mode(int);
void set_terminal_raw();
void printStats();

int main(int argc, char *argv[]){
	tty_mode(0);                /* save current terminal mode */
    set_terminal_raw();         /* set -icanon, -echo   */
    top();                 /* interact with user */
    tty_mode(1); 
	return 0;
}

void top(){
	char ch = '\0';
	double timer = 3.00;
	time_t end = time(0) + timer;
	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
	while(1){		
		fflush(stdout);
		system("clear");
    	printStats();
    	ch = '\0';
    	//sleep(3);
    	end = time(0) + timer;
    	while(time(0) < end){
    		ch = getchar();
    		if(isalpha(ch) || isdigit(ch))
        		break;
    	}    	
    	switch(ch){
    		case 'q':
    			return;
     		case 's':
     			tty_mode(1);
     			fcntl(STDIN_FILENO, F_SETFL, flags);
    			printf("Change delay from %0.3lf to ", timer );
    			scanf("%lf",&timer);	
    			fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    			set_terminal_raw();    			
    			continue;	
    		default:
    			continue;
    	}        	
	}
}


void getCurrentTime(){
	time_t currentTime = time(0);
  	struct tm localTime = *localtime(&currentTime);
   	printf("%02d:%02d:%02d",localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
}

/* put file descriptor 0 into chr-by-chr mode and noecho mode */
void set_terminal_raw(){
    struct  termios ttystate;
    tcgetattr( 0, &ttystate);               /* read current setting */
    ttystate.c_lflag          &= ~ICANON;   /* no buffering     */
    ttystate.c_lflag          &= ~ECHO;     /* no echo either   */
    ttystate.c_cc[VMIN]        =  1;        /* get 1 char at a time */
    tcsetattr( 0 , TCSANOW, &ttystate);     /* install settings */
}

/* 0 => save current mode  1 => restore mode */
void tty_mode( int operation ){
    static struct termios original_mode;
    if ( operation == 0 )
        tcgetattr( 0, &original_mode );
    else
        tcsetattr( 0, TCSANOW, &original_mode ); 
}

void printStats(){
	printf("top - ");
	getCurrentTime();
	getTimeSinceBoot();
	getUserCount();
	loadavg();
	getProcessesCount();
	getCpuUsage();

	printf("\n");
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

void getCpuUsage() {	
	double user, nice, system, idle, ioWait, hardInt, softInt, vmSteal, total;
	
  	FILE * fp = NULL; 
  	fp = fopen("/proc/stat", "r");  	
  	fseek(fp, 0, SEEK_SET);
    if (fp) {
    	fscanf(fp, "%*s %lf %lf %lf %lf %lf %lf %lf %lf", &user, &nice, &system, &idle, &ioWait, &hardInt, &softInt, &vmSteal);      
    	total = user + nice + system + idle + ioWait + hardInt + softInt + vmSteal;
    	printf("%cCpu(s):  %0.1f us,  %0.1f sy, %0.1f ni, %0.1f id, %0.1f wa, %0.1f hi, %0.1f si, %0.1f st \n", '%',
    	 (user * 100) / total, (system * 100) / total, (nice * 100) / total, (idle * 100) / total, (ioWait * 100) / total, (hardInt * 100) / total, (softInt * 100) / total, (vmSteal * 100) / total);    	
	}	
	fflush(fp);
  	fclose(fp);
}

