#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>
#include <unistd.h>
#define MSG_SIZE	sizeof(ordenProduccion)-sizeof(long)
#define pathname "/tmp"
#define numero_semaforos_contadores	3
#define SHM_MODE	(IPC_CREAT | 0777)
#define SEM_MODE	(0640 | IPC_CREAT)
#define QUEUE_MODE	(0660 | IPC_CREAT)

typedef struct{
	long tipoMensaje;
	int A;
	int B;
	int C;
}ordenProduccion;

key_t keyall;
int queueId;
int shmId;
int semId;

void Error(int modo){
	printf("\n---------------------------------------");
	printf("\n----------------+ALERTA+---------------");
	printf("\n---------------------------------------\n\n");
	if(modo == 1) printf("\nNO HAY MATERIALES PARA DESENCOLAR, ESPERANDO MATERIALES...\n");
	if(modo == 2) printf("\nNO SE HA ELIMINADO EL SEMAFORO\n");
	if(modo == 3) printf("\nNO SE HA ELIMINADO LA COLA DE MENSAJES\n");
	if(modo == 4) printf("\nNO SE HA ELIMINADO LA MEMORIA COMPARTIDA\n");
	msgctl(queueId, IPC_RMID, 0);
    shmctl(shmId, IPC_RMID, NULL); //destroy shared memory
    semctl(semId, 0, IPC_RMID); //destroy semaphore
	exit(-1);
}

void onCtrlC(){
	/*elimino todos los semaforo*/
	for(int i= 0; i < numero_semaforos_contadores+1; i++){
		if(semctl(semId, i, IPC_RMID) == -1){	
			printf("\nhrbihrjsnfjinrji ====> %d \n", i);
			Error(2);
		}
	}
	/*elimino la cola*/
	if(msgctl(queueId, IPC_RMID, 0) == -1)	Error(3);
	/*elimino la memoria compartida*/
    if(shmctl(shmId, IPC_RMID, NULL) == -1)	Error(4);
    
    exit(0);
}

void inicializarSemaforos(){
	//Inicializacion de semaforos no principales a "Rojo"
	for(int i = 0; i < numero_semaforos_contadores; i++){
		semctl(semId, i, SETVAL, 0);
	}
	//Inicializacion de semaforo principal a "Verde"
	semctl(semId, 3, SETVAL, 1);
}

void printEstado(int modo){
	printf("\nAhora: %s de la produccion\n", (modo ==1) ? "INICIO" : "FIN");
}

void actualizarGrupoSemaforico(ordenProduccion material){
	struct sembuf semAction; 
	semAction.sem_num = 0;
	semAction.sem_op -= material.A;
	semAction.sem_flg = 0;
	semop(semId, &semAction, 1);
	
	semAction.sem_num = 1;
	semAction.sem_op -= material.B;
	semop(semId, &semAction, 1);
	
	semAction.sem_num = 2;
	semAction.sem_op -= material.C;
	semop(semId, &semAction, 1);
}

void actualizarEstadosPedidos(const key_t key){
	const int shmId = shmget(key, 0, 0777); //get shm
    int* array = (int*) shmat(shmId, NULL, 0); //attach
    array[0] = array[0] + 1; //servidos
    array[1] = array[1] - 1; //pendientes
    shmdt(array);//detach
}

void crearSHM_SEM_QUEUE(int proj_id){
	keyall = ftok(pathname, proj_id);
    shmId = shmget(keyall, 2 * sizeof(int), SHM_MODE);
    semId = semget(keyall, 4, SEM_MODE);
    queueId = msgget(keyall, QUEUE_MODE);
}

void exitProgram(){
	signal(SIGINT, onCtrlC); //when Control+C is pressed we jump to handler
    printf("Press CTRL+C to exit \n");
}

int main(int argc, char **argv){
	if(argc <= 1){
		printf("FALTAN ARGUMENTOS => %s clave\n", argv[0]);
		printf("Intentalo de nuevo\n");
		exit(-1);
	}
    crearSHM_SEM_QUEUE(argv[1][0]);
    inicializarSemaforos();
    exitProgram();
    ordenProduccion material;
    for(;;){
		if(msgrcv(queueId, &material, MSG_SIZE, -9999, 0) != -1){
			actualizarGrupoSemaforico(material);
			printEstado(1);
			sleep(2);
			printEstado(2);
			semctl(semId, 3, SETVAL, 0);
			actualizarEstadosPedidos(keyall);
			semctl(semId, 3, SETVAL, 1);
		}
	}
	return (EXIT_SUCCESS);
}
