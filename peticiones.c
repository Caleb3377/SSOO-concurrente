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
#define QUEUE_MODE 0660
#define numero_semaforos_contadores 3
#define MSG_SIZE	sizeof(ordenProduccion)-sizeof(long)

typedef struct{
	long tipoMensaje;
	int A;
	int B;
	int C;
}ordenProduccion;

int peticiones;
key_t key;
int queueId;
int shmId;
int semId;
FILE* input;

void onCtrlD(){
	/*elimino pid*/
	kill(-peticiones, SIGTERM);
	/*elimino todos los semaforo*/
	for(int i= 0; i < numero_semaforos_contadores+1; i++){
		if(semctl(semId, i, IPC_RMID) == -1){	
			printf("\nhrbihrjsnfjinrji ====> %d \n", i);
			//Error(2);
		}
	}
	/*elimino la cola*/
	if(msgctl(queueId, IPC_RMID, 0) == -1)	printf("Eeror\n");
	/*elimino la memoria compartida*/
    if(shmctl(shmId, IPC_RMID, NULL) == -1)	printf("Eeror\n");
    fclose(input);
    exit(0);
}

void Error(int modo){
	if(modo == 1){
		printf("Ha ocurrido un error con  los argumentos de entrada\n");
	}
	onCtrlD();
}

void obtenerSHM_SEM_QUEUE(){
	semId = semget(key, 4, SEM_MODE);
	shmId = shmget(key, 2 * sizeof(int), SHM_MODE);
	queueId = msgget(key, QUEUE_MODE);
}

void unlock_lock_Sem3(int modo){
	struct sembuf semAction;
    semAction.sem_num = 3;
	semAction.sem_op = (modo == 1) ? -1 : 1;
    semAction.sem_flg = 0;
    semop(semId, &semAction, 1);
}

void actualizarEstadoPedidoPendiente(){
	int* array = (int*) shmat(shmId, NULL, 0); //attach
    array[1] = array[1]+1; //write
    shmdt(array); //detach
}

void accesoFileDescriptor(int argc, char**argv){
	input = (argc == 3) ? fopen(argv[1], "r") : stdin;
}

ordenProduccion crearMensaje(ordenProduccion produccion){
	ordenProduccion mensaje={0, 0, 0};
	return mensaje;
}

void atenderPeticiones(){
	ordenProduccion produccion;
	produccion.tipoMensaje = 0;
	while(fscanf(input, "%ld %ld %ld[^\n]", &produccion.A, &produccion.B, &produccion.C) != EOF){
		unlock_lock_Sem3(1);
		actualizarEstadoPedidoPendiente();
		unlock_lock_Sem3(2);
		/*esto*/
		//ordenProduccion mensaje = crearMensaje(produccion);
		/*o esto*/
		produccion.tipoMensaje += 1;
		ordenProduccion mensaje = produccion;
		msgsnd(queueId, &mensaje, MSG_SIZE, 0);
	}	
}

int main(int argc, char **argv){
	if(argc == 2 || argc == 3){
		accesoFileDescriptor(argc, argv);
		key = ftok("/tmp", argv[1][0]);
	}
	else Error(1);
	
	obtenerSHM_SEM_QUEUE();
	signal(SIGQUIT, onCtrlD);
	peticiones = fork();
	if(peticiones > 0){
		atenderPeticiones();
		waitpid(peticiones,NULL,0);
		exit(0);
	}
}
