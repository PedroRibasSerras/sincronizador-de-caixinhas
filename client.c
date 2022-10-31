#include "protocolo.c"
#include <pthread.h>
#include <string.h>
#include "lista/lista.c"


#define VIDEOS_PATH "./videos"

//VARIAVEIS GLOBAIS - FEITO ASSIM PARA PODER USAR ENTRE AS THREADS SEM TER QUE FICAR PASSANDO POR PARAMETRO
int sock;
pthread_t reader, playListner, downloader;
libvlc_instance_t *vlc;
int playerControllerState[1];
pthread_mutex_t downloadsEmAndamentoMutex;
int nDownloadsEmAndamento, readyRequest;
Lista filaDeMusicas;
int indexMusicaAtual;

//FOI NECESSARIO CRIAR UM PROTOTYPE DE ALGUMAS FUNÇÕES QUE SE CHAMAM ENTRE SI
void nextMusica(int);
void prevMusic();
void playMusica(MusicaPlayer *);
void pauseMusica(MusicaPlayer *);
void handleReadyQuestion();
void readyResponse();

//FUNÇÃO QUE E FEITA PARA RODAR DENTRO DE UMA THREAD EM SEPARADO E FICA VERIFICANDO SE A MUSICA ESTASENDO TOCADA
//QUANDO A MUSICA ACABAR, O SERVIDOR E AVISADO
void *esperaAcabar(void* mp){
    sleep(1);
    while(libvlc_media_player_is_playing(mp)){
        sleep(1);
    }
    BYTE* packet;
    if(*playerControllerState == PLAY){
        packet = encode(CONTROLE_DE_MULTIMIDIA_COMANDO, ENDED_C_OPCAO);
        nextMusica(1);
        printf("Musica acabou!\n");
    }
    else if(*playerControllerState == PAUSE){
        packet = encode(CONTROLE_DE_MULTIMIDIA_COMANDO, PAUSE_OK_C_OPCAO);
        printf("Musica foi pausada!\n");
    }
    else if(*playerControllerState == NEXT){
        packet = encode(CONTROLE_DE_MULTIMIDIA_COMANDO, ENDED_C_OPCAO);
        printf("Pulou!\n");
    }

    send(sock, packet, byteArraySize(packet), 0);
    
    return NULL;
    
}

//DA PLAY NA MUSICA
void playMusica(MusicaPlayer * player){
    if(player == NULL)
        return;
    
    if(player->mp == NULL)
        return;

    printf("Play:%s\n",player->nome);
    *playerControllerState = PLAY;
    libvlc_media_player_play(player->mp);
    pthread_create(&playListner, NULL, esperaAcabar, player->mp);
}

//DA PAUSE NA MUSICA
void pauseMusica(MusicaPlayer * player){
    if(player == NULL)
        return;

    if(player->mp == NULL)
        return;
    if(libvlc_media_player_is_playing(player->mp)){
        printf("Pause:%s\n",player->nome);
        *playerControllerState = PAUSE;
        libvlc_media_player_pause(player->mp);
        pthread_join(playListner,NULL);
    }
}

//VAI PARA A PROXIMA MUSICA
void nextMusica(int withPlay){
        MusicaPlayer *atual = getIndexLista(filaDeMusicas,indexMusicaAtual);
        *playerControllerState = NEXT;
        if(atual != NULL){
            libvlc_media_player_stop(atual->mp);
            pthread_join(playListner,NULL);
            if(indexMusicaAtual<tamanhoLista(filaDeMusicas)){
                printf("TESte1");
                indexMusicaAtual++;
            }
            handleReadyQuestion(); 
        }
        else{
            if(indexMusicaAtual<tamanhoLista(filaDeMusicas)){
                printf("teste2\n");
                indexMusicaAtual++;
                handleReadyQuestion(); 
            }
        }
}

//VOLTA UMA MUSICA MUSICA
void prevMusic(){
    MusicaPlayer *atual = getIndexLista(filaDeMusicas,indexMusicaAtual);
    if(atual == NULL){
        indexMusicaAtual--;
        handleReadyQuestion(); 
        return;
    }

    printf("%ld\n", libvlc_media_player_get_time(atual->mp));
    if(libvlc_media_player_get_time(atual->mp) < 5000){
        pauseMusica(atual);
        libvlc_media_player_set_time(atual->mp,0);
        indexMusicaAtual--;
        handleReadyQuestion(); 
        playMusica(getIndexLista(filaDeMusicas,indexMusicaAtual));
        printf("Nao passou dos 5 seg\n");
    }
    else{
        libvlc_media_player_set_time(atual->mp,0);
        printf("Passou dos 5 seg\n");
    }
}

//AVALIA SE A MUSICA ATUAL ESTA PRONTA PARA SER TOCADA, CAS0 ESTEIA AVISA O SERVIDOR.
//CASO NAO ESTEJA, LEVANTA UMA FLAG PARA LEMBRAR DE AVISAR O SERVIDOR QUANDO ELA ESTIVER PRONTA
void handleReadyQuestion(){
    MusicaPlayer *p = getIndexLista(filaDeMusicas,indexMusicaAtual);
    if(p != NULL){
        printf("MusicaPlaye nao nulo\n");
        if(p->mp != NULL){
            printf("mp nao nulo\n");
            readyResponse();
        }else{
            printf("mp nulo\n");
            readyRequest = TRUE;
        }
    }
    else{
        printf("Primeiro da lista nulo\n");
        readyRequest = TRUE;
    }
}

//FALA PRO SERVIDOR QUE A MUSICA ATUAL ESTA PRONTA
void readyResponse(){
    printf("Ready response enviado!");
    BYTE* packet;
    packet = encode(VERIFICACAO_DE_CAIXA_COMANDO, READYR_C_OPCAO);
    send(sock, packet, byteArraySize(packet), 0);
    readyRequest = FALSE;
}

//MUDA O FIM DE UM STRING
int strNewEnd(char* str, char endStrMarker){
    for(int i = 0; i<strlen(str); i++){
        if(str[i] == endStrMarker){
            str[i] = '\0';
            return 0;
        }
    }
    return -1;
}


//MUDA O INICIO DE UM STRING
int strNewBegin(char** strP, char newBeginMarker){
    for(int i = 0; i<strlen(*strP); i++){
        if((*strP)[i] == newBeginMarker){
            *strP +=i; 
            return 0;
        }
    }
    return -1;
}

//RODA EM UMA THREAD SEPARADA.
//FAZ O DOWNLOAD DA MUSICA USANDO O YOUTUBE-DL
//PREPARA O PLAYER DA MUSICA E ADICIONA NA LISTA
void * downloadMusica(void * linkParam){

    char* link = (char *)linkParam;
    MusicaPlayer *player = getLastLista(filaDeMusicas);
    int posicao = tamanhoLista(filaDeMusicas) - 1;
    char buffer[1024];
    FILE *terminal;
    char comando[1024];
    char *vBegin = strstr(link,"v=");
    char vCopy[strlen(vBegin)];

    strcpy(vCopy, vBegin);
    if(strlen(vCopy) == 0){
        printf("Link quebrado! faloutou o parametro v= na url do youtube!");
        return NULL;
    }
    
    strNewEnd(vCopy, '&');

    snprintf(comando,1023,"youtube-dl -x -f bestaudio --audio-format mp3 --write-info-json  -o '%s/%s.%(ext)s' %s",VIDEOS_PATH, vCopy, link);
    printf("Cria o comando!");
    terminal = popen(comando,"r");

	if(terminal == NULL){
		fputs("POPEN: Failed to execute command. \n", stderr);
		return NULL;
	}
    while(fgets(buffer, 1023, terminal) != NULL){
        
        printf("%s", buffer);
        if(strstr(buffer, "100%") != NULL){
            printf("Deu certo\n");
            break;
        }
        
    }	

	pclose(terminal);

    
    FILE *info;
    char *title;
    char pathInfo[201];
    snprintf(pathInfo,200,"%s/%s%s", VIDEOS_PATH, vCopy,".info.json");

    
    info = fopen(pathInfo,"r");
    if(!info){
        printf("PROBLEMA NA ABERTURA DAS INFORMACOES");
        return NULL;
    }
    
    fseek(info, -700, SEEK_END);
    while(fgets(buffer, 1024, info) != NULL){
			
        title = strstr(buffer, "fulltitle");
        if(title != NULL){
            title = title + strlen("fulltitle\": \"");
            strNewEnd(title,'"');
            break;
        }
        
    }

    player->nome = malloc((strlen(title) + 1)* sizeof(char));
    strcpy(player->nome,title);

    snprintf(comando,200,"ls %s/%s%s", VIDEOS_PATH, vCopy,".*");
    terminal = popen(comando,"r");

	if(terminal == NULL){
		fputs("POPEN: Failed to execute command. \n", stderr);
		return NULL;
	}
    while(fgets(buffer, 1023, terminal) != NULL){
        
        if(strstr(buffer, ".info.json") == NULL){
            break;
        }
        
    }	

	pclose(terminal);

    char **exts = (char **) malloc (sizeof(char *));
    *exts = (char *) malloc (strlen(vCopy + 1) * sizeof(char ));
    *exts = strstr(buffer,vCopy);
    strNewBegin(exts,'.');
    if((*exts)[strlen(*exts) - 1] == '\n')
        (*exts)[strlen(*exts) - 1] = '\0';

    char path[201];
    snprintf(path,200,"%s/%s%s", VIDEOS_PATH, vCopy,*exts);
    libvlc_media_t *m = libvlc_media_new_path(vlc, path);
    player->mp = libvlc_media_player_new_from_media(m);
    
    libvlc_media_release(m);

    if(readyRequest && posicao == indexMusicaAtual){
        readyResponse();
    }

    return NULL;
   
}


//FUNCAO PRINCIPAL
int main(int argc, char const* argv[])
{
    //INICIA AS VARIAVEIS GLOBAIS E OUTRAS VARIAVEIS
    vlc = libvlc_new(0, NULL);
    sock = 0;
    indexMusicaAtual = 0;
    int valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    filaDeMusicas = criaLista();
    nDownloadsEmAndamento = 0;
    readyRequest = FALSE;

    //PREPARA O SOCKET PARA UTILIZAR O TCP
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Erro na criacao do socket\n");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
  
    //CONVERT O IPV4 PARA BINARIO
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "Erro na conversao do endereco\n");
        return -1;
    }
    

    //CONECTA CLIENT E SERVIDOR USANDO O SOCKET CRIADO
    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("Conexao falhou!\n");
        return -1;
    }

    int sair = FALSE;
    int res = 0;
    //LOOP PRINCIPAL
    while (!sair)
    {
        printListaMP(filaDeMusicas);
        printf("\nMusica atula:%d\n\n",indexMusicaAtual);
        res = read(sock, buffer, 1024);
        if(res <= 0){
            printf("Algo deu errado (possivelmente o servidor desconectou)!");
            exit(0);
        }
        printf("res: %d\n", res);
        printPacket((BYTE*)buffer);
        Linha_de_comando* lc =  decode((BYTE*)buffer);
        
        switch(lc->command){
            case VERIFICACAO_DE_CAIXA_COMANDO:{
                if(strcmp(lc->option,READYQ_C_OPCAO) == 0){
                    handleReadyQuestion();
                }
            }break;
            case DOWNLOAD_MULTIMIDIA_COMANDO:{
                MusicaPlayer *p = (MusicaPlayer *) malloc(sizeof(MusicaPlayer));
                p->mp=NULL;
                appendLista(filaDeMusicas,*p);
                pthread_create(&downloader, NULL, downloadMusica, lc->option);
            }break;
            case CONTROLE_DE_MULTIMIDIA_COMANDO:{
                if(strcmp(lc->option,PLAY_C_OPCAO)==0){
                    MusicaPlayer *p = (getIndexLista(filaDeMusicas,indexMusicaAtual));
                    playMusica(p);
                }else
                if(strcmp(lc->option,PAUSE_C_OPCAO)==0){
                    MusicaPlayer *p = (getIndexLista(filaDeMusicas,indexMusicaAtual));
                    pauseMusica(p);
                }else
                if(strcmp(lc->option,NEXT_C_OPCAO)==0){
                    printf("testeoutnext\n");
                    nextMusica(TRUE);
                }else
                if(strcmp(lc->option,PREV_C_OPCAO)==0){
                    prevMusic();
                }

            }break;
            default:{
                sleep(1);
                printf("Estado de espera");
            }break;
        }
    
    }
    
  
    //FECHA O SOCKET DO CLIENT
    close(client_fd);
    return 0;
}