#include <stdlib.h>
#include <stdio.h>
#include <vlc/vlc.h>

typedef struct
{
    libvlc_media_player_t *mp;
    char * nome;
    libvlc_time_t duracao;
}MusicaPlayer;

typedef MusicaPlayer tipo;


typedef struct no{
    tipo dado;
    struct no* proximo;
}No;

typedef No** Lista;


Lista criaLista(){
    Lista lista;
    lista = (No**) malloc(sizeof(No*));
    *lista = NULL;
    return lista;
}

int appendLista(Lista lista, tipo dado){
    No* atual = *lista;
    No* novo = (No*) malloc(sizeof(No));
    novo->dado = dado;
    novo->proximo = NULL;
    if(!atual){
        *lista = novo;
        return 0;
    }

    while(atual->proximo){
        atual = atual->proximo;
    }
    atual->proximo = novo;

    return 0;
    
}

int tamanhoLista(Lista lista){
    No* atual = *lista;
    int c = 0;
    if(!atual)
        return c;
    c++;
    while(atual->proximo){
        atual = atual->proximo;
        c++;
    }
    return c;


    return 0;
    
}

tipo* getIndexLista(Lista lista, int index){
    No* atual = *lista;
    if(index < 0)
        return NULL;

    int c = 0;
    while(atual){
        if(c == index)
            return &(atual->dado);
        atual = atual->proximo;
        c++;
    }
    return NULL;
}

tipo* getFirstLista(Lista lista){
    if(*lista){
        return &((*lista)->dado);
    }
    return NULL;
}

tipo* getLastLista(Lista lista){
    No* atual = *lista;
    if(!atual){
        return NULL;
    }

    while(atual->proximo){
        atual = atual->proximo;
    }
    return &(atual->dado);
    
}


tipo* takeFirstLista(Lista lista){
    if(*lista){
        No* first = *lista;
        *lista = (*lista)->proximo;
        return &(first->dado);
    }
    return NULL;
}

void printListaMP(Lista lista){
    
    No* atual = *lista;
    int c = 0;
    if(!atual)
        printf("Fila vazia!");

    while(atual){
        printf("%d - %s\n", c, atual->dado.nome);
        atual = atual->proximo;
        c++;
    }
    
}