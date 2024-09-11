#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <assert.h>
#include <string.h>
#include "command.h"
#include <unistd.h>
#include "builtin.h"
#include "tests/syscall_mock.h"

static int estrue(scommand cmd){
    int i=0;
    //Lista del nombre de los comandos donde siempre el ultimo es NULL asi termina
    const char* lista_funciones[] = {"help","cd","exit",NULL};
    // Si no le mando nada devuelvo 0,que es no es builtin
    if (cmd==NULL){
    return 0;}
    //Voy fijandome si lo cmd==a una de las palabras de la lista
    while(lista_funciones[i]!=NULL){
    if (strcmp(scommand_front(cmd),lista_funciones[i])==0){
    return i+1;
    //si es igual returneo i+1, que es siempre distinto de 0
    }
    i++;
    }
    //si termina el while sin retornar retorno 0, ya que no está
    return 0;
}

bool builtin_alone(pipeline p){
    if (p==NULL){
        return false;
        //Me fijo que su tamaño sea 1 y que sea de builtin
    }else{ return ((pipeline_length(p) == 1) && builtin_is_internal(pipeline_front(p)));
}
}

//transforma un int de estrue() en bool, aunque podriamos haber trabajado tranquilamente con estrue
bool builtin_is_internal(scommand cmd){
    return (estrue(cmd)!=0);
}

static void func_help(scommand cmd){
    //Funcion help
            printf("    Shell name:\n"
            "- Mybash \n\n" 
            "   Nombres de los integrantes:\n"
            "- José Castro, Thomas Bunzli, Gabriel Franicevich,  Lucas Sabino.\n\n"
            "   Lista de comandos internos: \n"
            "(1) - cd:\n "
            "   Se utiliza para cambiar el directorio de trabajo actual. Ejecute 'cd /' para redirigirse a HOME. \n"
            "(2) - help:\n "
            "   breve descripcion de la consola y comandos.\n"
            "(3) - exit:\n" 
            "   Sale de la shell\n\n");

            scommand_pop_front(cmd);
        }
//funcion cd
static void func_cd(scommand cmd){
            scommand_pop_front(cmd); //elimino cd
            if (cmd==NULL){//me fijo si es del tipo (cd   )
                chdir(getenv("HOME"));
            }else{
                int a=chdir(scommand_front(cmd));//tomo lo que le sigue a cd y voy a esa carpeta
            if (a!=0){//me fijo si se produjo un error
                perror(scommand_front(cmd));
            }}
        }

//funcion exit
static void func_exit(scommand cmd){
            exit(EXIT_SUCCESS);
        }



void builtin_run(scommand cmd){
    //lista de llamado de funciones
    void (*func[])(scommand) = {NULL,func_help, func_cd, func_exit}; 
    //Llamo a las funciones
    func[estrue(cmd)](cmd);
    }
/*Es facil de agregarle mas funciones, ya que solamente hay que crear un static void func,
 y luego agregar el nombre del comando a listafunciones[] y su funcion a func[]
  */      
