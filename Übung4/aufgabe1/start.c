#include <stdlib.h>         // exit() und EXIT_FAILURE/SUCCESS
#include <stdio.h>          // printf() und fprintf()
#include <unistd.h>         // fork() und execvp()
#include <sys/types.h>      // Datentyp pid_t 
#include <sys/resource.h>   // setpriority()
#include <sys/wait.h>       // waitpid() und Makros zur Statusüberprüfung
#include <string.h>         // strsignal()


/**
 * Hauptfunktion, die einen Kindprozess erstellt und ein angegebenes Programm mit Argumenten ausführt.
 * @param argc die Anzahl der Befehlszeilenargumente beispiel (sleep 2 wäre die anzhal 2) 
 * @param argv ein Array von Zeichenketten, das die Befehlszeilenargumente enthält (Array 1 = sleep, array 2 = 34 )
 * @return EXIT_SUCCESS bei erfolgreicher Ausführung, andernfalls EXIT_FAILURE
 */
int main(int argc, char **argv) { //argv ist ein Array von Zeichenketten, das die Befehlszeilenargumente enthält und **argv ist ein doppelter Zeiger auf Zeichenketten, der auf das erste Element des Arrays zeigt

    if (argc < 3) {
    fprintf(stderr, "Usage: %s <Programm> [Argumente...]\n", argv[0]);
    exit(EXIT_FAILURE);
}

    // Starten eines neuen Kind Prozesses mittels fork()
    pid_t pid = fork();                      // pid_t = Prozess-ID, die durch fork() zurückgegeben wird und verwendet wird, um den Prozess zu identifizieren

    //printf("Prio: Prozess %d wurde gestartet\n", PRIO_PROCESS);
      //  if (setpriority(PRIO_PROCESS, pid, 19) == -1) {     // Priorität des Kindprozesses auf 19 setzen -> niedrige Prioritä
       // perror("setpriority");                           // Fehlermeldung ausgeben, falls setpriority fehlschlägt -1
      // exit(EXIT_FAILURE);                              // Es gibt PRIO_PROCESS, PRIO_PGRP und PRIO_USER, die jeweils für Prozesse, Prozessgruppen und Benutzer verwendet werden können.
    //}
    if (pid == 0) {                 
      
if (setpriority(PRIO_PROCESS, pid, 19) == -1) {     // Priorität des Kindprozesses auf 19 setzen -> niedrige Prioritä
        perror("setpriority");                           // Fehlermeldung ausgeben, falls setpriority fehlschlägt -1
       exit(EXIT_FAILURE);   
}

        // Das Programm mit den angegebenen Argumenten ausführen int execvp(const char* command, char* argv[]);
        execvp(argv[1], argv+1);        //starte ein Nuues programm aus, mit dem Den Programm was in der ersten stelle im array steht und argv +1  ist ein Zeiger auf das Sub-Array
        perror("execvp");               // execvp gibt nur zurück, wenn ein Fehler auftritt
        exit(EXIT_FAILURE);

    } else {    // Ansonsten handelt es sich um den Elternprozess

        int status;
        // Variable zum Speichern des Status des Kindprozesses
        printf(" Kindprozess %d wurde gestartet\n", pid);
        

        printf("Priorität des Kindprozesses: %d\n", getpriority(PRIO_PROCESS, pid));
        // Die PID des gestarteten Kindprozesses ausgeben
        if (waitpid(pid, &status, WUNTRACED | WCONTINUED) < 0) {  
            // waitpid() sorgt dafür, dass der Vaterprozess auf das Ende des Kindprozesses wartet
            // Wird statt wait() genutzt, da waitpid() auf spezifische Kindprozesse warten kann, wait() wartet auf beliebige Kindprozesse
            // pid: i.ein kindprozess, beendigungsstatus wird in status gespeichert
            // WUNTRACED: wartet auf Kindprozesse, die gestoppt wurden
            // WCONTINUED: wartet auf Kindprozesse, die fortgesetzt wurden
            // WUNTRACED und WCONTINUED sind Optionen, die es dem Elternprozess ermöglichen, Informationen über den Kindprozess zu erhalten, auch wenn dieser gestoppt oder fortgesetzt wurde
            // Stellt sicher, dass Elternprozess Informationen über die Beendigung des Kindprozesses erhalten kann
            perror("waitpid");
            // Fehlermeldung ausgeben, falls waitpid fehlschlägt
            exit(EXIT_FAILURE);
            // Programm beenden mit Fehlercode
        }

        if (WIFEXITED(status)) {
            // WIFEXITED(status) prüft, ob der Kindprozess normal beendet wurde
            // WEXITSTATUS(status) gibt den Exitcode des Kindprozesses aus, falls dieser normal beendet wurde
            printf("Kindprozess Exit-Code: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            // Durch WIFSIGNALLED(status) prüfen, ob der Kindprozess durch ein Signal beendet wurde, 3. ausgabe eines evtl. ...
            int signal = WTERMSIG(status);
            // WTERMSIG(status) gibt das Signal aus, das den Kindprozess beendet hat
            printf("Kindprozess durch Signal %d beendet: %s\n", signal, strsignal(signal));
            // strsignal(signal) gibt eine verständliche Bezeichnung des Signals aus, das den Kindprozess beendet hat
        } else if (WIFSTOPPED(status)) {
            // Durch WIFSTOPPED(status) prüfen, ob der Kindprozess gestoppt wurde
            // Ausgabe des Signals, das den Kindprozess gestoppt hat durch WSTOPSIG(status)
            printf("Kindprozess wurde durch Signal %d gestoppt\n", WSTOPSIG(status));
        }
    }
    exit(EXIT_SUCCESS);
    // Programm erfolgreich beenden
}










/** 
*
*| Makro                 | Bedeutung                                                                 |
*| --------------------- | ------------------------------------------------------------------------- |
*| `WIFEXITED(status)`   | ✅ Kindprozess wurde normal beendet (z. B. mit `exit()` oder `return`)    |
*| `WEXITSTATUS(status)` | → Gibt den Exit-Code zurück (z. B. `0`, `1`, `42` usw.)                   |
*| `WIFSIGNALED(status)` | ❌ Prozess wurde durch ein Signal beendet (z. B. `KILL -9 ...`)            |
*| `WTERMSIG(status)`    | → Welches Signal hat ihn beendet                                           |
*| `WIFSTOPPED(status)`  | 🔁 Kindprozess wurde gestoppt(nicht beendet)                               |
*| `WSTOPSIG(status)`    | → Welches Signal hat den Prozess gestoppt                                 |
*/