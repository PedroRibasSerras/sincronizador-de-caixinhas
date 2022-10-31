#include "protocolo.c"
#include <pthread.h>

#define N_CAIXINHAS 10

typedef struct{
	int n;
	pthread_t reader;
	int socket;
	int ready;
}Caixinha;


void *listenInput(void* caixinhaP);
void enviaUltimoComandoDeMultimidia(int socket);

//VARIAVEIS GLOBAIS
Caixinha caixinhas[10];
struct sockaddr_in address;
int addrlen;
int server_fd;
pthread_t waiter;
char buffer[1024];
char ultimoComando[10];
int perguntouSeEstaPronto;

// FUNCAO QUE VERIFICA SE TODAS AS CAIXINHAS CONECTADAS RESPONDERAM QUE ESTAO PRONTAS
int verificaSeCaixinhasEstaoProntas(){
	for(int i = 0; i < N_CAIXINHAS; i ++){
		if(caixinhas[i].n != -1){
			if(caixinhas[i].ready == FALSE){
				printf("Nem todas as cixinhas prontas\n");
				return FALSE;
			}
		}
	}
	printf("Caixinhas prontas\n");
	return TRUE;
}

//FUNCAO QUE ENVIA O ULTIMO COMANDO DE MULTIMIDIA PARA TODAS AS CAIXINHAS
void enviaUltimoComandoDeMultimidiaParaTodasCaixinhas(){
	for(int i = 0; i < N_CAIXINHAS; i ++){
		if(caixinhas[i].n != -1){
			enviaUltimoComandoDeMultimidia(caixinhas[i].socket);
		}
	}
}

//FUNCAO FEITA PARA SER USADA POR UMA THREAD SEPARADA.
//CONNECTA COM UMA CAIXINHA NOVA
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

		//ADICIONA NA LIDAS DE CAIXINHAS
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

//ENVIA O ULTIMO COMANDO DE MUITIMIDIA QUE FOI DADO NO SERVER PARA UMA CAIXINHA
//REPESENTADA PELO SOCKET. COMUCINACAO FEITA COM TCP
void enviaUltimoComandoDeMultimidia(int socket){
	BYTE* packet = encode(CONTROLE_DE_MULTIMIDIA_COMANDO, ultimoComando);
	printPacket(packet);
	send(socket, packet, byteArraySize(packet), 0);
}

//ENVIA UMA MENSAGEM PARA UMA CAIXINHA PERGUNTADO SE ELA ESTA PRONTA PARA TOCAR A MUSICA ATUAL.
void perguntaSeEstaPronto(int socket){
	
	BYTE* packet = encode(VERIFICACAO_DE_CAIXA_COMANDO, READYQ_C_OPCAO);
	printPacket(packet);
	send(socket, packet, byteArraySize(packet), 0);
}

//FUNCAO FEITA PARA SER RODADA EM UM THREAD SEPARADA, SENDO 1 THREAD POR CAIXINHA
//FICA ESPERANDO A CAIXINHA ENVIAR ALGUMA MENSAGEM
void *listenInput(void* caixinhaP){
	char buffer[1024];
	Caixinha *caixinha = ((Caixinha*) caixinhaP);
	int res = 0; 
    while(1){
		res = read((*caixinha).socket, buffer, 1024);
		if(res <= 0){
            printf("Caixinha %d desconectou!\n", (*caixinha).n);
            (*caixinha).n = -1; 
			return NULL;
        }
		printPacket((BYTE*)buffer);
		Linha_de_comando* lc =  decode((BYTE*)buffer);
		switch(lc->command){
			case VERIFICACAO_DE_CAIXA_COMANDO:{
				if(strcmp(lc->option,READYR_C_OPCAO) == 0){
					(*caixinha).ready = TRUE;
					if(verificaSeCaixinhasEstaoProntas()){
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

//PARTE "GRAFICA" DO CONTROLE DE MIDIA FEITA NO TERMINAL
void initControlMenu(){

	int option;
	while(1){
		system("clear");
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
				strcpy(ultimoComando,PLAY_C_OPCAO);
				
				if(perguntouSeEstaPronto == FALSE){
					for(int i = 0; i < N_CAIXINHAS; i ++){
						if(caixinhas[i].n != -1)
							perguntaSeEstaPronto(caixinhas[i].socket);
					}
				}
				
			}break;
			case PAUSE:{
				strcpy(ultimoComando,PAUSE_C_OPCAO);
				
				if(perguntouSeEstaPronto == FALSE){
					for(int i = 0; i < N_CAIXINHAS; i ++){
						if(caixinhas[i].n != -1)
							perguntaSeEstaPronto(caixinhas[i].socket);
					}
				}
				
			}break;
			case NEXT:{
				strcpy(ultimoComando,NEXT_C_OPCAO);
				for(int i = 0; i < N_CAIXINHAS; i ++){
					if(caixinhas[i].n != -1)
						enviaUltimoComandoDeMultimidia(caixinhas[i].socket);
				}
				strcpy(ultimoComando,PLAY_C_OPCAO);

			}break;
			case PREV:{
				strcpy(ultimoComando,PREV_C_OPCAO);
				for(int i = 0; i < N_CAIXINHAS; i ++){
					if(caixinhas[i].n != -1)
						enviaUltimoComandoDeMultimidia(caixinhas[i].socket);
				}
				strcpy(ultimoComando,PLAY_C_OPCAO);

			}break;
			case STOP:{
				return;
			}break;
		
		default:
			printf("d\n");
			break;
		}
	}

}

//PERGUNTA PARA O USUARIO O LINK DA MUSICA
void iniciaMusica(Musica* musica){
	printf("Link do youbube da musica: ");
	fflush(stdout);
	scanf("%s", musica->name);
	musica->estado = PAUSE;
}

//FUNCAO PRINCIPAL
int main(int argc, char const* argv[])
{
	//INICIA TODAS AS CAIXINHAS
	for(int i = 0; i< N_CAIXINHAS; i++){
		caixinhas[i].n = -1;
		caixinhas[i].ready = 0;
	}

	int opt = 1;
	addrlen = sizeof(address);

	//INICIA BUFFER VAZIO
	char buffer[1024] = { 0 };


	perguntouSeEstaPronto = FALSE;

	//CRIA O DESCRITOR DO SCOKET
	printf("Criando socket...\n");
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Erro na criacao do socket\n");
		exit(-1);
	}

	//CONFIGURA O SOCKET
	printf("Configurando socket...\n");
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		printf("Erro ao configurar o socket!\n");
		exit(-1);
	}

	//VALORES DO ENDERECO
	address.sin_family = AF_INET;//FALA QUE E IPV4
	address.sin_addr.s_addr = INADDR_ANY;//QUALQUER IP PERMITIDO
	address.sin_port = htons(PORT);//PORTA

	//FAZ O BIND DO SOCKET PARA REALMENTE CRIAR O SOCKETS(FAZRE A LIGACAO DA PORTA COM O SOCKET)
	printf("Ligando socket...\n");
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("Erro no bind \n");
		exit(-1);
	}

	//ESCUTA ENVIOS PARA O SOCKET
	printf("Escutando socket...\n");
	if (listen(server_fd, 3) < 0) {
		printf("Erro ao iniciar o listen!\n");
		exit(-1);
	}

	//CRIACAO DA THREAD PARA ESPERA DE NOVAS CONEXOES
	pthread_create(&waiter, NULL, esperaCaixinha, NULL);

	

	int estaLigado = TRUE;
	Musica *musicaAtual;


	musicaAtual = (Musica *) malloc(sizeof(Musica *));
	//LOOP PRINCIPAL
	while(estaLigado){
		int option;
		printf("1- Escolher musica\n");
		printf("2- Ir para controlador\n");
		printf("0- Desligar server\n");
		printf("Escolha uma das opcoes:");
		fflush(NULL);
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
    			
				initControlMenu();
			} break;
			case 2:{
				initControlMenu();
			}break;
			case DESLIGAR_SERVER:{
				estaLigado = FALSE;
			} break;
			default:{
				printf("Nao e uma opcao!!");
				printf("Algo deu errado na escolha(Possivelmente não foi colocado um inteiro)!");
				estaLigado = FALSE;
			} break;
		}
	
		
		
		
	}

	//FECHANDO OS SOCKETS DAS CAIXINHAS
	for(int i = 0; i< N_CAIXINHAS; i++){
		if(caixinhas[i].n != -1){
			close(caixinhas[i].socket);
		}
	}
	

	//PARANDO DE ESCUTAR OS SOCKETS
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}
