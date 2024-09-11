#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "parsing.h"
#include "parser.h"
#include "command.h"

static scommand parse_scommand(Parser p) { // Creo el scommand donde se guarda el comando
    scommand cmd = scommand_new();

    arg_kind_t type;
    parser_skip_blanks(p);
    char *arg = parser_next_argument(p, &type); 

    // Recorro el comando y tomo los datos necesarios (arg es el dato y type el tipo de dato)
    while(arg!=NULL){
        if (type == ARG_NORMAL){ 
            scommand_push_back(cmd, arg);       // Si es normal, lo agrego
        } else if (type == ARG_INPUT){
            scommand_set_redir_in(cmd, arg);    // Si es de entrada, la seteo
        } else if (type == ARG_OUTPUT){
            scommand_set_redir_out(cmd, arg);   // Si es de salida, la seteo
        }

        parser_skip_blanks(p);
        arg = parser_next_argument(p, &type);
    }

    if (scommand_length(cmd) == 0)
    {
        scommand_destroy(cmd);
        cmd = NULL;
    }
    return cmd;
}

pipeline parse_pipeline(Parser p) {

    assert(p!=NULL && !parser_at_eof(p));

    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool error = false, another_pipe=true;

    cmd = parse_scommand(p);
    error = (cmd==NULL); /* Comando invalido al empezar */

    while (another_pipe && !error) {

        pipeline_push_back(result, cmd);
        parser_op_pipe(p, &another_pipe);

        if (another_pipe)
        {
            cmd = parse_scommand(p);
            error = (cmd == NULL);
        }
    }
    // Busco un & 
    if(!parser_at_eof(p)){
        bool is_background;

        parser_op_background(p, &is_background);

        // Le digo si tiene que esperar
        pipeline_set_wait(result, !is_background);

        // Borro lo que sobra pasado el &
        parser_skip_blanks(p);
        parser_garbage(p, &error);
    }
    
    if(error || pipeline_is_empty(result)){
        result = pipeline_destroy(result);
        result = NULL;
    }

    return result; 
}
