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
#define numero_semaforos_contadores 3
#define MSG_SIZE	sizeof(ordenProduccion)-sizeof(long)

typedef struct{
	long tipoMensaje;
	int A;
	int B;
	int C;
}ordenProduccion;

int materiales;
key_t key;
int semId;
FILE* input;

void onCtrlD(){
	/*elimino pid*/
	kill(-materiales, SIGTERM);
	/*elimino todos los semaforo*/
	for(int i= 0; i < numero_semaforos_contadores+1; i++){
		if(semctl(semId, i, IPC_RMID) == -1){	
			printf("\nhrbihrjsnfjinrji ====> %d \n", i);
			//Error(2);
		}
	}
    fclose(input);
    exit(0);
}

void Error(int modo){
	if(modo == 1){
		printf("Ha ocurrido un error con  los argumentos de entrada\n");
	}
	onCtrlD();
}

void obtenerSEM_(){
	semId = semget(key, 4, SEM_MODE);
}

void accesoFileDescriptor(int argc, char**argv){
	input = (argc == 3) ? fopen(argv[1], "r") : stdin;
}

void actualizarGrupoSemaforico(ordenProduccion material){
	struct sembuf semAction; 
	semAction.sem_num = 0;
	semAction.sem_op +=material.A;
	semAction.sem_flg = 0;
	semop(semId, &semAction, 1);
	
	semAction.sem_num = 1;
	semAction.sem_op += material.B;
	semop(semId, &semAction, 1);
	
	semAction.sem_num = 2;
	semAction.sem_op += material.C;
	semop(semId, &semAction, 1);
}

void acopiarMateriales(){
	ordenProduccion produccion;
	produccion.tipoMensaje = 0;
	while(fscanf(input, "%d %d %d[^\n]", &produccion.A, &produccion.B, &produccion.C) != EOF){
		actualizarGrupoSemaforico(produccion);
	}	
}

int main(int argc, char **argv){
	if(argc == 2 || argc == 3){
		accesoFileDescriptor(argc, argv);
		key = ftok("/tmp", argv[1][0]);
	}
	else Error(1);
	
	obtenerSEM_();
	signal(SIGQUIT, onCtrlD);
	materiales = fork();
	if(materiales > 0){
		acopiarMateriales();
		waitpid(materiales,NULL,0);
		exit(0);
	}
}
