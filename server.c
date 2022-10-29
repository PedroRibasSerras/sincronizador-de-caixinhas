#include "protocolo.c"
#include <pthread.h>
#include <sys/time.h>

#define N_CAIXINHAS 10
#define LIMIAR 1000

typedef struct{
	int n;
	pthread_t reader;
	int socket;
	int ready;
}Caixinha;

void *listenInput(void* caixinhaP);
void enviaUltimoComandoDeMultimidia(int socket);
void enviaUltimoComandoDeMultimidiaParaTodasCaixinhas();

struct sockaddr_in	 udpaddr;
int udpSock;
Caixinha caixinhas[N_CAIXINHAS];
struct sockaddr_in address;
int addrlen;
int server_fd;
pthread_t waiter,udpWaiter;
char buffer[1024];
char ultimoComando[10];
int perguntouSeEstaPronto;
int nUdpCaixinha;
struct timeval startBroadcast, broadcastResponse[N_CAIXINHAS];
int rn_caixinhas = 0;
int syncNum = 0;

long unsigned int calcBroadcastTempo(int n_caixinha){
	return (broadcastResponse[buffer[n_caixinha]].tv_sec - startBroadcast.tv_sec) * 1000000 + broadcastResponse[n_caixinha].tv_usec - startBroadcast.tv_usec;
}

int verificaErroSincronia(){
	syncNum--;
	int max = broadcastResponse[0].tv_sec - startBroadcast.tv_sec;
	int min =  broadcastResponse[0].tv_sec - startBroadcast.tv_sec;
	if(syncNum == 0){
		for(int i = 1; i < N_CAIXINHAS; i++){
			if(caixinhas[i].n == -1){
				long unsigned int tempoCaixinha  = calcBroadcastTempo(i);
				if(max < tempoCaixinha)
					max = tempoCaixinha;
				else if(min > tempoCaixinha)
					min = tempoCaixinha;
			}
		}
		if(max - min > LIMIAR)
			return TRUE;
	}
	return FALSE;
}


void *teste(void * nullP){
	char buffer[1024];

	int lenUdpaddr = sizeof(udpaddr);
	while(1){
	int n = recvfrom(udpSock, (char *)buffer, 1024,
					0, (struct sockaddr *) &udpaddr,
					&lenUdpaddr);

	gettimeofday(&broadcastResponse[buffer[0]], NULL);
	printf("%d - took %lu us\n",buffer[0], (broadcastResponse[buffer[0]].tv_sec - startBroadcast.tv_sec) * 1000000 + broadcastResponse[buffer[0]].tv_usec - startBroadcast.tv_usec);
		if(verificaErroSincronia()){
			strcpy(ultimoComando,RESET_C_OPCAO);
			enviaUltimoComandoDeMultimidiaParaTodasCaixinhas();
			strcpy(ultimoComando,PLAY_C_OPCAO);
		}
	}
}

int criaUdp(){
	int sockfd;
    int yes = 1;
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

    int ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes));
    if (ret == -1) {
        perror("setsockopt error");
        return 0;
    }
	
	memset(&udpaddr, 0, sizeof(udpaddr));
		
	// Filling server information
	udpaddr.sin_family = AF_INET; // IPv4
	udpaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	udpaddr.sin_port = htons(PORT+1);
		
	return sockfd;
}

int verificaSeCaixinhasEstaoProntas(){
	for(int i = 0; i < N_CAIXINHAS; i ++){
		if(caixinhas[i].n != -1){
			if(caixinhas[i].ready == FALSE){
				printf("nem todas as cixinhas prontas\n");
				return FALSE;
			}
		}
	}
	printf("Caixinhas prontas\n");
	return TRUE;
}

void enviaUltimoComandoDeMultimidiaParaTodasCaixinhas(){
	syncNum = rn_caixinhas;
	BYTE* packet = encode(CONTROLE_DE_MULTIMIDIA_COMANDO, ultimoComando);
	printPacket(packet);
	printf("Envia udp!");
	sendto(udpSock, packet, byteArraySize(packet),
		MSG_CONFIRM, (const struct sockaddr *) &udpaddr,
			sizeof(udpaddr));
}

void * esperaCaixinha(void * nullParam){
	while(1){
		int new_socket;
			if ((new_socket
				= accept(server_fd, (struct sockaddr*)&address,
						(socklen_t*)&addrlen))
				< 0) {
				perror("accept\n");
				exit(EXIT_FAILURE);
			}
		char n_caixa[2];
		n_caixa[0] = rn_caixinhas;
		n_caixa[1] = '\0'; 
		BYTE* packet = encode(IDENTIFICACAO_CAIXA_COMANDO, n_caixa);
		rn_caixinhas++;
		send(new_socket, packet, byteArraySize(packet), 0);
		for(int i = 0; i < N_CAIXINHAS; i++){
			if(caixinhas[i].n == -1){
				caixinhas[i].n = i;
				caixinhas[i].socket = new_socket;
				pthread_create(&(caixinhas[i].reader), NULL, listenInput, (void *) &caixinhas[i]);
				break;
			}
		}
	}
	
}

void enviaUltimoComandoDeMultimidia(int socket){
	BYTE* packet = encode(CONTROLE_DE_MULTIMIDIA_COMANDO, ultimoComando);
	printPacket(packet);
	send(socket, packet, byteArraySize(packet), 0);
}

void perguntaSeEstaPronto(int socket){
	
	BYTE* packet = encode(VERIFICACAO_DE_CAIXA_COMANDO, READYQ_C_OPCAO);
	printPacket(packet);
	send(socket, packet, byteArraySize(packet), 0);
}

void *listenInput(void* caixinhaP){
	char buffer[1024];
	Caixinha *caixinha = ((Caixinha*) caixinhaP);
    while(1){
		read((*caixinha).socket, buffer, 1024);
		printPacket((BYTE*)buffer);
		Linha_de_comando* lc =  decode((BYTE*)buffer);
		switch(lc->command){
			case VERIFICACAO_DE_CAIXA_COMANDO:{
				if(strcmp(lc->option,READYR_C_OPCAO) == 0){
					(*caixinha).ready = TRUE;
					if(verificaSeCaixinhasEstaoProntas()){
						gettimeofday(&startBroadcast, NULL);
						perguntouSeEstaPronto = FALSE;
						enviaUltimoComandoDeMultimidiaParaTodasCaixinhas();
					}
				}
			}break;
			case CONTROLE_DE_MULTIMIDIA_COMANDO:{
				if(strcmp(lc->option,ENDED_C_OPCAO) == 0){
					perguntouSeEstaPronto = TRUE;
						(*caixinha).ready = 0;
					perguntaSeEstaPronto((*caixinha).socket);
				}
				else if(strcmp(lc->option,PAUSE_C_OPCAO) == 0){
					// MARCAR MUSICA COMO PAUSADA
				}
			}break;
		}
	}
	return NULL;
}

void initControlMenu(Musica * musicaAtual){

	int option;
	while(1){
		//system("clear");
		printf("----- Musica atual: %s -----\n", musicaAtual->name);
		printf("1- Play\n");
		printf("2- Pause\n");
		printf("3- Anterior\n");
		printf("4- Proxima\n");
		printf("0- Menu\n");
		printf("Escolha a opcao:\n");
		scanf("%d", &option);
		switch (option)
		{
			case PLAY:{
				musicaAtual->estado = PLAY;
				strcpy(ultimoComando,PLAY_C_OPCAO);
				
				if(perguntouSeEstaPronto == FALSE){
					for(int i = 0; i < N_CAIXINHAS; i ++){
						if(caixinhas[i].n != -1)
							perguntaSeEstaPronto(caixinhas[i].socket);
					}
				}
				
			}break;
			case PAUSE:{
				musicaAtual->estado = PAUSE;
				strcpy(ultimoComando,PAUSE_C_OPCAO);
				
				if(perguntouSeEstaPronto == FALSE){
					for(int i = 0; i < N_CAIXINHAS; i ++){
						if(caixinhas[i].n != -1)
							perguntaSeEstaPronto(caixinhas[i].socket);
					}
				}
				
			}break;
			case NEXT:{
				musicaAtual->estado = PLAY;
				strcpy(ultimoComando,NEXT_C_OPCAO);
				for(int i = 0; i < N_CAIXINHAS; i ++){
					if(caixinhas[i].n != -1)
						enviaUltimoComandoDeMultimidia(caixinhas[i].socket);
				}
				strcpy(ultimoComando,PLAY_C_OPCAO);

			}break;
			case PREV:{
				musicaAtual->estado = PLAY;
				strcpy(ultimoComando,PREV_C_OPCAO);
				for(int i = 0; i < N_CAIXINHAS; i ++){
					if(caixinhas[i].n != -1)
						enviaUltimoComandoDeMultimidia(caixinhas[i].socket);
				}
				strcpy(ultimoComando,PLAY_C_OPCAO);

			}break;
			case STOP:{
				musicaAtual->estado = STOP;
				return;
			}break;
		
		default:
			printf("d\n");
			break;
		}
	}

}

int main(int argc, char const* argv[])
{
	for(int i = 0; i< N_CAIXINHAS; i++){
		caixinhas[i].n = -1;
		caixinhas[i].ready = 0;
	}
	int opt = 1;
	addrlen = sizeof(address);
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";
	perguntouSeEstaPronto = FALSE;

	printf("Criando socket...");
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	printf("Configurando socket...");
	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	printf("Ligando socket...\n");
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("bind failed\n");
		exit(EXIT_FAILURE);
	}

	printf("Escutando socket...\n");
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	
	pthread_create(&waiter, NULL, esperaCaixinha, NULL);
	pthread_create(&udpWaiter, NULL, teste, NULL);

	udpSock = criaUdp();
	

	int estaLigado = TRUE;
	Musica *musicaAtual;
	if(!(musicaAtual = ((Musica *) malloc(sizeof(Musica *)))))
					exit(1);
	while(estaLigado){
		int option;
		printf("1- Escolher musica\n");
		printf("0- Desligar server\n");
		printf("Escolha uma das opcoes:");
		fflush(stdout);
		fflush(stdin);
		scanf("%d",&option);
		switch (option)
		{
			case ESCOLHER_MUSICA:{
				iniciaMusica(musicaAtual);
				
				//valread = read(new_socket, buffer, 1024);
				//printf("%s\n", buffer);
				BYTE* pakect = encode(DOWNLOAD_MULTIMIDIA_COMANDO,musicaAtual->name);
				for(int i = 0; i < N_CAIXINHAS; i ++){
					if(caixinhas[i].n != -1){
						fflush(NULL);
						send(caixinhas[i].socket, pakect, byteArraySize(pakect), 0);
					}
				}
    			
				initControlMenu(musicaAtual);
			} break;
			case DESLIGAR_SERVER:{
				estaLigado = FALSE;
			} break;
			default:{
				printf("Nao e uma opcao!!");
			} break;
		}
	
		
		
		
	}

	for(int i = 0; i< N_CAIXINHAS; i++){
		if(caixinhas[i].n != -1){
			close(caixinhas[i].socket);
		}
	}
	// closing the connected socket
	

	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}
