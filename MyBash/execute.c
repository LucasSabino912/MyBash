#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <fcntl.h>      //  O_RDWR, O_CREAT
#include <unistd.h>     //  STDOUT_FILENO
#include <sys/stat.h>   //  S_IRUSR 
#include <sys/wait.h>   //  waitpid

#include "execute.h"
#include "builtin.h"
#include "tests/syscall_mock.h"

//el número de variables para asignar en la memoria y el tamaño en bytes de una sola variable
//Descomponemos cmd en un array arg[] que contiene todos los argumentos del comando
static char **argumentos(scommand cmd){
    int tam=scommand_length(cmd);
    char **arg=calloc(tam+1,sizeof(char*));
    
    for(int i=0;i<tam;i++){
        arg[i] = strdup(scommand_front(cmd));
        scommand_pop_front(cmd);
    }
    arg[tam]=NULL;//Para que llene arg
    return(arg);
}

//ejecutador de los comandos
static void ejecuto_scommand(scommand cmd) {


    //vemos los REDIR_IN 
    if (scommand_get_redir_in(cmd) != NULL) {
        char* scommand_in = scommand_get_redir_in(cmd);

        //Abro el archivo con el nombre dado con derechos de lector solamente
        int open_in = open(scommand_in , O_RDONLY , S_IRUSR); 
        if (open_in < 0){
            //Si no lo pude abrir marco error
            perror("Error al abrir el scommand_in");
            exit(EXIT_FAILURE);
        }

        //Dupeo
        int dup_in = dup2(open_in , STDIN_FILENO);
        if (dup_in < 0) {//Si algo fallo cierro
            perror("Error al abrir el dup_IN");
            close(open_in);  // Cierro el archivo y exiteo
            exit(EXIT_FAILURE);
        }  
        close(open_in);//cierro el archivo; 
    } 

    //vemos REDIR_OUT
    if (scommand_get_redir_out(cmd) != NULL) {

        char* scommand_out = scommand_get_redir_out(cmd);
        int open_out = open (scommand_out, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR); // solo los permisos necesarios

        if (open_out<0){//Si no lo pude abrir marco error
            perror("Error al abrir el scommand_out");
            exit(EXIT_FAILURE);//Salgo con fallo
        }

        int dup_out = dup2(open_out , STDOUT_FILENO);

        if (dup_out<0){//Si algo falla, marco error
            perror("Error al abrir el dup_out");
            close(open_out);  // Cierro el archivo y exiteo
            exit(EXIT_FAILURE);
        }
        close(open_out);//cierro el archivo 
    }

    //Ejecuto el scommand

    //Transformo cmd en una lista de su comando, seguido de sus argumentos 
    char **arg=argumentos(cmd);

    //Ejecuto
    if (arg[0]!=NULL){
        int ex=execvp(arg[0], arg);
                
        if (ex<0){
            perror("No se ejecuto el execvp correctamente");
            exit(EXIT_FAILURE);
        }
    }
}


//  si pipe es de tamaño uno
static void ejecuto_pipe_1(pipeline apipe) {

    scommand cmd = pipeline_front(apipe);
    bool pipe_wait = pipeline_get_wait(apipe);

    //  me fijo si es internal
    if (builtin_is_internal(cmd)) {
        builtin_run(cmd);
        return; //  si lo es termino el programa
    }

    pid_t pid = fork();

    if (pid<0){
        perror("Error al hacer fork");
        exit(EXIT_FAILURE);
    }else if (pid==0){
        //si salio bien ejecuto
        ejecuto_scommand(cmd);
    }else  if (pipe_wait){ //  me fijo si tiene que esperar
        waitpid(pid, NULL,0);
    }
}

//Ejecuto una pipe de tamaño 2
static void ejecuto_pipe_2(pipeline apipe) {
    scommand com_1 = pipeline_front(apipe);
    // Chequeo si ejecuta en bg
    bool wait = pipeline_get_wait(apipe);

    int fd[2];

    pipe(fd);

    int id1 = fork();

    if (id1 < 0) {
        fprintf(stderr, "Error en fork\n");
    } else if (id1 == 0){
        int chequeo1 = dup2(fd[1], STDOUT_FILENO);
        if (chequeo1 == -1){
            fprintf(stderr, "descriptor de archivo duplicado\n");
        } else {
            close(fd[0]);
            close(fd[1]);

            ejecuto_scommand(com_1);
        }

    } else {
        pipeline_pop_front(apipe);
        scommand com_2 = pipeline_front(apipe);
        int id2 = fork();

        if (id2 < 0){
            fprintf(stderr, "Error en fork\n");
        } else if (id2 == 0) {    
            int chequeo2 = dup2(fd[0],STDIN_FILENO);
            if (chequeo2 == -1) {
                fprintf(stderr, "descriptor de archivo duplicado\n");
            } else {
                close(fd[0]);
                close(fd[1]);

                ejecuto_scommand(com_2);
            }
        } else {
            close(fd[0]);
            close(fd[1]);
        }

        if (wait){
            waitpid(id1, NULL, 0);
            waitpid(id2, NULL, 0);
        }
    }
}

void execute_pipeline(pipeline apipe){
    unsigned int tam = pipeline_length(apipe);

    if (builtin_alone(apipe)){ // Si lo unico que hay es un builtin
        builtin_run(pipeline_front(apipe));
    } else if(tam == 1){ //Si lo unico que hay es un comando no interno 
        ejecuto_pipe_1(apipe);
    } else if(tam == 2){ //Si hay dos comandos
        ejecuto_pipe_2(apipe);
    } else if(tam > 2) {
        fprintf(stderr, "Error: solo se permite una pipe\n");
    }
}