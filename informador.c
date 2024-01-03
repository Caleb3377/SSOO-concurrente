#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#define SEM_MODE 0640
#define SHM_MODE 0777
int semId;
int shmId;
key_t key;
int pid;
int vecesInformadas = 0;

void onCtrlC(){
	printf("\n-----------------------");
	printf("\nHE INFORMADO %d veces", vecesInformadas);
	printf("\n-----------------------");
	/*elimino pid*/
	kill(-pid, SIGTERM);
	/*elimino todos los semaforo*/
	if(semctl(semId, 3, IPC_RMID) == -1)	printf("Error\n");//Error(2);
	/*elimino la memoria compartida*/
    if(shmctl(shmId, IPC_RMID, NULL) == -1)	printf("Error\n");//Error(4);
    exit(0);
}

void unlock_lock_Sem3(int modo){
	struct sembuf semAction;
    semAction.sem_num = 3;
	semAction.sem_op = (modo == 1) ? -1 : 1;
    semAction.sem_flg = 0;
    semop(semId, &semAction, 1);
}

void obtenerSHM_SEM(){
	semId = semget(key, 4, SEM_MODE);
	shmId = shmget(key, 2 * sizeof(int), SHM_MODE);
}

void exitProgram(){
	signal(SIGINT, onCtrlC); //when Control+C is pressed we jump to handler
    printf("Press CTRL+C to exit \n");
}

void informar(){
	int* array = (int*) shmat(shmId, NULL,0);
	pid = fork();
	if(pid == 0){
		vecesInformadas+=1;
		printf("%d Informador [%d] estado de produccion : [Servidos: %d, Pendientes: %d]\n", vecesInformadas, getppid(), array[0], array[1]);
	}
	waitpid(pid, &wstatus, 0);
	
	shmdt(array);
}

int main(int argc, char** argv) {
    if (argc<=2){
        printf("FALTAN ARGUMENTOS => %s clave periodo\n", argv[0]);
        printf("Intentalo de nuevo\n");
		exit(-1);
    }
	key = ftok("/tmp", argv[1][0]);
    const int periodoI = atoi(argv[2]);
    obtenerSHM_SEM();
    exitProgram();
    int wstatus;
    while(1){
		unlock_lock_Sem3(1);
		informar();
		unlock_lock_Sem3(2);
		sleep(periodoI);
	}
    return (EXIT_SUCCESS);
}
