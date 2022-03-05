#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"

typedef struct {
    int process;
    int th_nb;
    pthread_mutex_t *lock;
    pthread_cond_t *start_cond;
    pthread_cond_t *end_cond;
} TH_STRUCT_P4;

typedef struct {
    int process;
    int th_nb;
    sem_t *logSem;
} TH_STRUCT_P6;

bool start_t1 = false;
bool end_t2 = false;
int th_running = 0;
int th_started = 0;

void* th_func_p7(void* param){
	TH_STRUCT_P4 *s = (TH_STRUCT_P4*)param;
	
	info(BEGIN, s->process, s->th_nb);
	info(END, s->process, s->th_nb);

	return NULL;
}

void* th_func_p6(void* param){
	TH_STRUCT_P6 *s = (TH_STRUCT_P6*)param;
	sem_t *logSem = s->logSem;
	int nb;
	sem_getvalue(logSem, &nb);
	//printf("Val before is %d\n", nb);
	sem_wait(logSem);
	/*
	if(s->th_nb == 11){
		if(nb == 0){
			info(BEGIN, s->process, s->th_nb);
			info(END, s->process, s->th_nb);
		}
		
	}
	else{
		
	}
	*/
	info(BEGIN, s->process, s->th_nb);
	info(END, s->process, s->th_nb);
	
	sem_post(logSem);

	return NULL;
}

void* th_func_p4(void* param){
	TH_STRUCT_P4 *s = (TH_STRUCT_P4*)param;
	
	pthread_mutex_lock(s->lock);
	if(s->process == 4 && s->th_nb == 2){
		while(!start_t1){
			pthread_cond_wait(s->start_cond, s->lock);
			
		}
		info(BEGIN, s->process, s->th_nb);
	}
	else{
		info(BEGIN, s->process, s->th_nb);
		if(s->process == 4 && s->th_nb == 1){
			start_t1 = true;
			pthread_cond_broadcast(s->start_cond);
		}
	}
	
	if(s->process == 4 && s->th_nb == 1){
		while(!end_t2){
			pthread_cond_wait(s->end_cond, s->lock);
		}
		info(END, s->process, s->th_nb);
	}
	else{
		info(END, s->process, s->th_nb);
		if(s->process == 4 && s->th_nb == 2){
			end_t2 = true;
			pthread_cond_broadcast(s->end_cond);
		}
	}
	
	pthread_mutex_unlock(s->lock);
	return NULL;
}

int main(){
    init();
    info(BEGIN, 1, 0);
    
    pid_t p2, p3, p4, p5, p6, p7;
    
    //create p2
    p2 = fork();
    if(p2 == 0){
    	//we are in p2
    	info(BEGIN, 2, 0);
    	
    	p3 = fork();
    	if(p3 == 0){
    		//we are in p3
    		info(BEGIN, 3, 0);
    		
    		
    		info(END, 3, 0);
    		exit(0);
    	}
    	
    	p4 = fork();
    	if(p4 == 0){
    		//we are in p4
    		info(BEGIN, 4, 0);
    		
    		pthread_t tid[5];
    		TH_STRUCT_P4 params[5];
    		pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    		pthread_cond_t start_cond = PTHREAD_COND_INITIALIZER;
    		pthread_cond_t end_cond = PTHREAD_COND_INITIALIZER;
    		
    		for(int i = 0; i < 5; i++){
    			params[i].process = 4;
    			params[i].th_nb = i+1;
    			params[i].lock = &lock;
    			params[i].start_cond = &start_cond;
    			params[i].end_cond = &end_cond;
    			pthread_create(&tid[i], NULL, th_func_p4, &params[i]);
    		}
    		
    		for(int i = 0; i < 5; i++){
    			pthread_join(tid[i], NULL);
    		}
    		
    		info(END, 4, 0);
    		exit(0);
    	}
    	
    	p5 = fork();
    	if(p5 == 0){
    		//we are in p5
    		info(BEGIN, 5, 0);
    		
    		
    		info(END, 5, 0);
    		exit(0);
    	}
    	
    	p7 = fork();
    	if(p7 == 0){
    		//we are in p7
    		info(BEGIN, 7, 0);
    		
    		pthread_t tid[5];
    		TH_STRUCT_P4 params[5];
    		
    		for(int i = 0; i < 5; i++){
    			params[i].process = 7;
    			params[i].th_nb = i+1;
    			pthread_create(&tid[i], NULL, th_func_p7, &params[i]);
    		}
    		
    		for(int i = 0; i < 5; i++){
    			pthread_join(tid[i], NULL);
    		}
    		
    		info(END, 7, 0);
    		exit(0);
    	}
    	
    	waitpid(p3, NULL, 0);
    	waitpid(p4, NULL, 0);
    	waitpid(p5, NULL, 0);
    	waitpid(p7, NULL, 0);
    	info(END, 2, 0);
    	exit(0);
	}
	
	//create p6
	p6 = fork();
	if(p6 == 0){
		//we are in p6	
		info(BEGIN, 6, 0);
		
		pthread_t tid[50];
    	TH_STRUCT_P6 params[50];
    	sem_t logSem;
    	if(sem_init(&logSem, 0, 5) != 0){
    		perror("Could not init the semaphore");
			return -1;
    	}
    	
    		
    	for(int i = 0; i < 50; i++){
    		params[i].process = 6;
    		params[i].th_nb = i+1;
    		params[i].logSem = &logSem;
    		pthread_create(&tid[i], NULL, th_func_p6, &params[i]);
    	}
    		
    	for(int i = 0; i < 50; i++){
    		pthread_join(tid[i], NULL);
    	}
    	
    	sem_destroy(&logSem);
		
		info(END, 6, 0);
		exit(0);
	}
	
	waitpid(p2, NULL, 0);
    waitpid(p6, NULL, 0);
    info(END, 1, 0);
    return 0;
}
