#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <assert.h>
#include <string.h>
#include "command.h"
#include <check.h>

struct scommand_s {
    GSList* cmd;
    char* in;
    char* out;
    // no hace falta length porque hay una propiedad de gslist para eso
};

struct pipeline_s{
    GSList * cmds;
    bool wait;
};

scommand scommand_new(void){
    scommand new = malloc(sizeof(struct scommand_s));
    assert(new != NULL);
    new->cmd=NULL;
    new->in=NULL;
    new->out=NULL;
    return new;
}

scommand scommand_destroy(scommand self){
    assert(self!=NULL);
    g_slist_free_full(self->cmd, g_free);
    free(self->in);
    free(self->out);
    free(self);
    return NULL;
}

void scommand_push_back(scommand self, char * argument){
    assert(self!=NULL && argument!=NULL);
    // Agrego argument al final
    self->cmd = g_slist_append(self->cmd, argument);
    assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
    assert(self != NULL && !scommand_is_empty(self));
    gpointer first = self->cmd->data;
    self->cmd = g_slist_remove(self->cmd, first);
    g_free(first);
}

void scommand_set_redir_in(scommand self, char * filename){
    assert(self!=NULL);
    // Hago un free al anterior input
    if (self->in != NULL) //para evitar doble free
	{
		free(self->in);
	}

    self->in = filename;
}

void scommand_set_redir_out(scommand self, char * filename){
    assert(self!=NULL);
    // Hago un free del anterior output
    if (self->out != NULL) //para evitar doble free
	{
		free(self->out);
	}

    self->out = filename;
}

bool scommand_is_empty(const scommand self){
    assert(self!=NULL);
	return self->cmd == NULL;
}

unsigned int scommand_length(const scommand self){
    assert(self!=NULL);
    return g_slist_length(self->cmd);
}

char * scommand_front(const scommand self){
    assert(self != NULL && !scommand_is_empty(self));
    return g_slist_nth_data(self->cmd,0);
}

char * scommand_get_redir_in(const scommand self){
    assert(self != NULL);
    return self->in;
}
char * scommand_get_redir_out(const scommand self){
    assert(self != NULL);
    return self->out;
}

char * scommand_to_string(const scommand self){
    assert(self != NULL);
    GString *str = g_string_new(NULL);
    GSList *current = self->cmd;

    while (current != NULL) {
        g_string_append(str, (char *)current->data);
        g_string_append(str, " ");
        current = current->next;
    }

    if (self->in != NULL) {
        g_string_append(str, "< ");
        g_string_append(str, self->in);
        g_string_append(str, " ");
    }

    if (self->out != NULL) {
        g_string_append(str, "> ");
        g_string_append(str, self->out);
    }

    return g_string_free(str, FALSE);
}

// PIPELIIIINES

pipeline pipeline_new(void){
    pipeline new = malloc(sizeof(struct pipeline_s));
    new->cmds = NULL;
    new->wait = true;
    assert(new!=NULL && pipeline_is_empty(new) && pipeline_get_wait(new));

    return new;
}

static void aux_destroy(void* self){
    scommand_destroy(self);
}

pipeline pipeline_destroy(pipeline self){
    assert(self != NULL);

    g_slist_free_full(self->cmds, aux_destroy);
    self->cmds=NULL;

    free(self);
    return NULL;
}

void pipeline_push_back(pipeline self, scommand sc){
    assert(self != NULL && sc != NULL);
    self->cmds = g_slist_append(self->cmds, sc);
    assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self){
    assert(self != NULL && !pipeline_is_empty(self));
    gpointer first = self->cmds->data;
    self->cmds = g_slist_remove(self->cmds, first);
    scommand_destroy((scommand)first);
}

void pipeline_set_wait(pipeline self, const bool w){
    assert(self != NULL);
    self->wait = w;
}

bool pipeline_is_empty(const pipeline self){
    assert(self!=NULL);
    return g_slist_length(self->cmds) == 0;//push no suma wait
}

unsigned int pipeline_length(const pipeline self){
    assert(self != NULL);
	return g_slist_length(self->cmds);
}

scommand pipeline_front(const pipeline self){
    assert(self != NULL && !pipeline_is_empty(self));
    return g_slist_nth_data(self->cmds, 0);
}

bool pipeline_get_wait(const pipeline self){
    assert(self != NULL);
    return self->wait;
}

char * pipeline_to_string(const pipeline self){
    assert(self != NULL);
    GString *result = g_string_new(NULL);
    GSList *current = self->cmds;

    while (current != NULL) {
        char *cmd_str = scommand_to_string((scommand)current->data);
        g_string_append(result, cmd_str);
        free(cmd_str);

        if (current->next != NULL) {
            g_string_append(result, " | ");
        }
        current = current->next;
    }

    if (!self->wait) {
        g_string_append(result, " &");
    }

    return g_string_free(result, FALSE);
}
