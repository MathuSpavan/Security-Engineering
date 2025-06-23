#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>

#define N_DATA 30         // Anzahl der zu verarbeitenden Daten
#define N_SHARED 5        // Größe des Shared Memory Puffers

// Semaphore-Nummern 
#define SEM_CAN_READ  0   // Semaphore für Leser (P2)
#define SEM_CAN_WRITE 1   // Semaphore für Schreiber (P1)


// P-Operation (Warten)
void semaphore_wait(int sem_id, int sem_num) {
    struct sembuf sembuf;
    sembuf.sem_num = sem_num;
    sembuf.sem_op = -1;
    semop(sem_id, &sembuf, 1);
}

// V-Operation (Signalisieren)
void semaphore_signal(int sem_id, int sem_num) {
    struct sembuf sembuf;
    sembuf.sem_num = sem_num;
    sembuf.sem_op = 1;
    semop(sem_id, &sembuf, 1);
}

// Speicher aufräumen
void cleanup(int shm_id, int sem_id) {
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, 0, IPC_RMID) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int shm_id, sem_id;
    int *shared_memory;     // Zeiger auf den Shared Memory, der ein Array von Ganzzahlen repräsentiert
    pid_t pid;              // pid_t ist ein Datentyp, der die Prozess-ID repräsentiert
    key_t key = IPC_PRIVATE; // IPC_PRIVATE erzeugt einen eindeutigen Schlüssel für Shared Memory und Semaphoren

    // Shared Memory anlegen
    shm_id = shmget(key, N_DATA * sizeof(int), 0666 | IPC_CREAT);
    if (shm_id < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Shared Memory anhängen
    shared_memory = (int *)shmat(shm_id, NULL, 0); // 0 bedeutet, dass der Prozess den Shared Memory im Standardmodus anhängt
    if (shared_memory == (int *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Semaphoren anlegen
    sem_id = semget(key, 2, 0666 | IPC_CREAT);
    if (sem_id < 0) {
        perror("semget");
        shmdt(shared_memory);
        exit(EXIT_FAILURE);
    }

    // Semaphoren initialisieren
    //union semunion sem_union;

    
   if (semctl(sem_id, SEM_CAN_READ, SETVAL, 0) < 0) {
    // Initialisiert SEM_CAN_READ auf 0:
    // P2 (Leser) muss warten – Lesen ist anfangs **nicht erlaubt**,
    // weil P1 (Schreiber) noch nichts geschrieben hat.
    perror("Fehler beim Initialisieren von SEM_CAN_READ");
    exit(EXIT_FAILURE);
}

if (semctl(sem_id, SEM_CAN_WRITE, SETVAL, 1) < 0) {
    // Initialisiert SEM_CAN_WRITE auf 1:
    // P1 (Schreiber) darf sofort starten – Schreiben ist **sofort erlaubt**,
    // da P2 noch nichts gelesen hat.
    perror("Fehler beim Initialisieren von SEM_CAN_WRITE");
    exit(EXIT_FAILURE);
}


    // Prozess erzeugen
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Kindprozess (P2 – Verbraucher)
        for (int i = 0; i < N_DATA; i += N_SHARED) { 
            semaphore_wait(sem_id, SEM_CAN_READ); // geht nicht weiter, weil 0 steht, warten bis 1 steht
            for (int j = 0; j < N_SHARED; ++j) {
                if (i + j < N_DATA) {
                    printf("P2 liest auf Position [%d]: %d\n", i + j + 1 , shared_memory[j]); // +1 für 1-basierte Ausgabe 
                }
            }
            semaphore_signal(sem_id, SEM_CAN_WRITE);
        }
        shmdt(shared_memory);
        exit(EXIT_SUCCESS);
    } else {
        // Elternprozess (P1 – Erzeuger)
        srand48(time(NULL)); // Initialisiert den Zufallszahlengenerator mit der aktuellen Zeit
        for (int i = 0; i < N_DATA; i += N_SHARED) {
            semaphore_wait(sem_id, SEM_CAN_WRITE);
            for (int j = 0; j < N_SHARED; ++j) {
                if (i + j < N_DATA) {
                    shared_memory[j] = lrand48();
                    printf("P1 schreibt auf Position [%d]: %d\n", i + j + 1, shared_memory[j]);
                }
            }
            semaphore_signal(sem_id, SEM_CAN_READ);
        }
        wait(NULL);
        shmdt(shared_memory);
        cleanup(shm_id, sem_id);
    }

    exit(EXIT_SUCCESS);
}

