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
#include<string.h>

extern int errno;
static int processCount=15;

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
void getPhysicalMemoryInfo();
void getVirtualMemoryInfo();
void getHelp();
void getProcessInformation(int);
char* getUserNameById(int);

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
    		case 'n':
     			tty_mode(1);
     			fcntl(STDIN_FILENO, F_SETFL, flags);
    			printf("Maximum tasks = %d, change to (0 is unlimited) ", processCount );
    			scanf("%d",&processCount);	
    			fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    			set_terminal_raw();    			
    			continue;	
    		case 'h':
    			getHelp();
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
	getPhysicalMemoryInfo();
	getVirtualMemoryInfo();
	printf("\n");
	getProcessInformation(processCount);
	printf("\n");
}

void getProcessInformation(int displayCount){
	printf("PID\tUSER\tPR\tNI\tVIRT\tRES\tSHR\tS\t%cCPU\t%cMEM\tTIME%c\tCOMMAND\n",'%','%','+');
   	DIR* dp = opendir("/proc/");
   	
   	char name[100];
  	char state;  	
  	long uid;
  	long pid,priority,nice;

  	int value, value2;
  	int userId, virtualMem, residentMem, sharedMem;
	char field[50];

   	char statDirName[100];
   	char loginDirName[100];
   	char line[500];
   	errno = 0;
   	int totalProcesses=0;
   	struct dirent* entry;
   	FILE * fp = NULL;
   	FILE * fp2 = NULL;  
   	while(1){
    	entry = readdir(dp);
    	if(entry == NULL && errno != 0){
        	perror("readdir");
        	exit(errno);
      	}
      	if(entry == NULL && errno == 0){
         	break;         
      	}
      	if(totalProcesses==displayCount)
      		break;
      	long lpid = atol(entry -> d_name);
	    if (entry -> d_type == DT_DIR) {
    	    if (isdigit(entry -> d_name[0])) {
        	  	totalProcesses++;
        	  	snprintf(statDirName, sizeof(statDirName), "/proc/%ld/stat", lpid);          
          		fp = fopen(statDirName, "r");
				if (fp) {
            		fscanf(fp, "%ld %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %ld %ld", &pid, name, &state, &priority, &nice);
    			}
    			fclose(fp);

    			snprintf(loginDirName, sizeof(loginDirName), "/proc/%ld/status", lpid);          
          		fp2 = fopen(loginDirName, "r");
          		while (fgets(line, 500, fp2)){          			
			  		sscanf(line, "%s %d %d", field, &value, &value2);  		
			  		if (!strcmp(field, "Uid:"))			        	
			        	userId = value2;			  		
			  		else if (!strcmp(field, "VmSize:"))
        				virtualMem = value;
      				else if (!strcmp(field, "VmRSS:"))
	        			residentMem = value;
    	    		else if (!strcmp(field, "RssAnon:")){
        				sharedMem = value;
        				break;
        			}
			  	}
    			fclose(fp2);
    			char* username = getUserNameById(userId);
    			printf("%s\t%s\t%ld\t%ld\t%d\t%d\t%d\t%c\t\n",entry->d_name,username,priority,nice,virtualMem,residentMem,residentMem-sharedMem,state);
    			userId = virtualMem = residentMem = sharedMem = 0;
    		}
   		}
	}
   	closedir(dp);
   	return;
}

char* getUserNameById(int uid){
	errno = 0;
	struct passwd * pwd = getpwuid(uid);
	if (pwd == NULL){
      	if (errno == 0)
        	return "";
      	else
        	perror("getpwuid failed");
    return "";
   }
   return pwd->pw_name;
}

void getTimeSinceBoot(){
	int hour, min, upTimeInSec;
	struct sysinfo s_info;
    int error = sysinfo(&s_info);
    if(error != 0)
    {
        printf("code error = %d\n", error);
        return;
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

void getPhysicalMemoryInfo(){
	double totalMem, freeMem, bufMem, usedMem, cachedMem,bufCacheMem, value;
	struct sysinfo s_info;
	char line[50], field[50];
    int error = sysinfo(&s_info);
    if(error != 0)
    {
        printf("code error = %d\n", error);
        return;
    }
    FILE * fp = NULL; 
  	fp = fopen("/proc/meminfo", "r");
  	if (fp == NULL)
  		return;
  	while (fgets(line, 50, fp)){
  		sscanf(line, "%s %lf", field, &value);  		
  		if (!strcmp(field, "Cached:")){
        	cachedMem = value;
        	break;
  		}
  	}
    totalMem = s_info.totalram;
    freeMem = s_info.freeram;
    bufMem = s_info.bufferram;
    usedMem = (totalMem/1024/1024) - (freeMem/1024/1024) - (bufMem/1024/1024) - (cachedMem/1024);
    bufCacheMem = (bufMem/1024/1024) + (cachedMem/1024);
    printf("Mib Mem:   %0.1f total,   %0.1f free,   %0.1f used,   %0.1f buff/cache \n", totalMem/1024/1024, freeMem/1024/1024, usedMem, bufCacheMem);
}

void getVirtualMemoryInfo(){
	double totalSwapMem, freeSwapMem, usedMem, availMem, value;
	struct sysinfo s_info;
	char line[50], field[50];
    int error = sysinfo(&s_info);
    if(error != 0)
    {
        printf("code error = %d\n", error);
        return;
    }
    FILE * fp = NULL; 
  	fp = fopen("/proc/meminfo", "r");
  	if (fp == NULL)
  		return;
  	while (fgets(line, 50, fp)){
  		sscanf(line, "%s %lf", field, &value);  		
  		if (!strcmp(field, "MemAvailable:")){
        	availMem = value;
        	break;
  		}
  	}
    totalSwapMem = s_info.totalswap;
    freeSwapMem = s_info.freeswap;
    usedMem = (totalSwapMem/1024/1024) - (freeSwapMem/1024/1024);
    printf("Mib Mem:   %0.1f   total, %0.1f   free, %0.1f   used, %0.1f   avail Mem \n", totalSwapMem/1024/1024, freeSwapMem/1024/1024, usedMem, availMem/1024);
}

void getHelp(){
	system("clear");
	FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen("help.txt", "r");
    if (fp == NULL)
        return;
    while ((read = getline(&line, &len, fp)) != -1) {
        printf("%s", line);
    }
    fclose(fp);
    if (line)
        free(line);
    char exit;
    do{
		exit = getchar();
	}while(exit != 'q');
	return;
}