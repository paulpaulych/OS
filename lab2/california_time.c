#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TIME_STRINg_LEN 50

int main(int argc, char ** argv){
	
	putenv("TZ=PST8PDT");
   	
   	time_t cur_time;
    time(&cur_time);
    
    struct tm *time_struct;
    time_struct = localtime(&cur_time);
    
    printf("%d/%d/%02d %d:%02d \n",
        time_struct->tm_mday,
        time_struct->tm_mon + 1,
        time_struct->tm_year + 1900,
        time_struct->tm_hour,
        time_struct->tm_min);	
}
