#include <vlc/vlc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VIDEOS_PATH "./videos"

libvlc_instance_t *vlc;

typedef struct
{
    libvlc_media_player_t *mp;
    char * nome;
    libvlc_time_t duracao;
}MusicaPlayer;

int strNewEnd(char* str, char endStrMarker){
    for(int i = 0; i<strlen(str); i++){
        if(str[i] == endStrMarker){
            str[i] = '\0';
            return 0;
        }
    }
    return -1;
}

MusicaPlayer* downloadMusica(char * link){
    MusicaPlayer *player = (MusicaPlayer *) malloc(sizeof(MusicaPlayer));

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
    player->nome = title;
    
    char path[201];
    snprintf(path,200,"%s/%s%s", VIDEOS_PATH, vCopy,".webm"); 
    libvlc_media_t *m = libvlc_media_new_path(vlc, path);
    player->mp = libvlc_media_player_new_from_media(m);
    
    libvlc_media_release(m);

    return player;
   
}

int main(){
    vlc = libvlc_new (0, NULL);
    printf("init");
    MusicaPlayer *p = downloadMusica("https://www.youtube.com/watch?v=2A2XBoxtcUA&list=RDMM9z8MdRRxMLY&index=15");
    if(p){
        libvlc_media_player_play(p->mp);
        printf("%s\n",p->nome);
        sleep(1);

        p->duracao = libvlc_media_player_get_length (p->mp);
    
        usleep((p->duracao-2000)*1000);
        while(libvlc_media_player_is_playing(p->mp)){
                
        }
    }
    return 0;
}