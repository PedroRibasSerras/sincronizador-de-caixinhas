// Client side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

//-----BOOL-----
#define FALSE 			0
#define TRUE 			1

//COMANDOS										<Comando>	Categoria
#define	VERIFICACAO_DE_CAIXA_COMANDO			0x03		//Verificação de caixa
#define	CONTROLE_DE_MULTIMIDIA_COMANDO			0x07		//Multimidia control
#define	DOWNLOAD_MULTIMIDIA_COMANDO			    0x08		//Multimidia DOWNLOAD

//OPCOES												<Comando>	Categoria

#define	READYQ_C_OPCAO			"READYQ"				//0x03		Verificação de caixa
#define	READYR_C_OPCAO			"READYR"				//0x03		Verificação de caixa
#define	DOWNLOAD_C_OPCAO		"FILE"					//0x03		Verificação de caixa
#define	DOWNLOAD_OK_C_OPCAO		"FILE OK"				//0x03		Verificação de caixa 	
#define	END_C_OPCAO				"END"					//0x05		Controle de envio de arquivo
#define	CANCELED_C_OPCAO		"CANCELED"				//0x05		Controle de envio de arquivo
#define	PLAY_C_OPCAO			"PLAY"					//0x07		Multimidia control
#define	PAUSE_C_OPCAO			"PAUSE"					//0x07		Multimidia control
#define	NEXT_C_OPCAO			"NEXT"					//0x07		Multimidia control
#define	PREV_C_OPCAO			"PREV"					//0x07		Multimidia control
#define	ENDED_C_OPCAO			"ENDED"					//0x07		Multimidia control
#define	PAUSE_OK_C_OPCAO		"PAUSE OK"				//0x07		Multimidia control


//-----OPCOES-----
#define DESLIGAR_SERVER 0
#define ESCOLHER_MUSICA 1

#define PLAY			1
#define PAUSE			2
#define PREV			3
#define NEXT			4
#define STOP			0

typedef struct
{

	char name[150];
	int estado;

}Musica;

typedef unsigned char BYTE;

typedef struct
{
	BYTE  command;
	char* option;

}Linha_de_comando;

//CALCULA O TAMANHO DO BYTE ARRAY
int byteArraySize(BYTE* byteArray){
	int count = 0;
	while(byteArray[count++] != '\0'){}
	return count;
}

//PRINTA UM BYTE ARRAY
void printByteArray(BYTE* byteArray){
    for(int i =0; i<byteArraySize(byteArray); i++){
        printf("%X", byteArray[i]);
    }
    printf("\n");
}

//CODIFICA UM COMANDO E SUA OPCAO PARA O PACOTE QUE DEVE SER ENVIADO
BYTE* encode(BYTE command, char* option){
	BYTE* packet;
	packet = (BYTE*) malloc(( 1 + strlen(option) + 1 ) * sizeof(BYTE));

	packet[0] = command;
	for(int i = 0; i < strlen(option); i++)
		packet[i+1] = option[i];
	packet[strlen(option) + 1] = '\0';
	return packet;
}

//CODIFICA A PARTIR DA ESTRUTURA LINHA DE COMANDO
BYTE* encodeLc(Linha_de_comando* lc){
	BYTE* packet;
	packet = (BYTE*) malloc(( 1 + strlen(lc->option) ) * sizeof(BYTE));
	
	packet[0] = lc->command;
	for(int i = 0; i < strlen(lc->option); i++)
		packet[i+1] = lc->option[i];
	packet[strlen(lc->option) + 1] = '\0';

	return packet;
}

//DECODIFICA O PACOTE PARA O FORMATO DE LINHA DE COMANDO
Linha_de_comando* decode(BYTE* packet){
	Linha_de_comando* lc;

	lc = (Linha_de_comando*) malloc(1 * sizeof(Linha_de_comando));
	lc->command = packet[0];
	
	int optionSize = (byteArraySize(packet) - 1);
	lc->option = (char*) malloc( (optionSize)* sizeof(char));
	for(int i = 0; i < optionSize; i++)
		lc->option[i] = packet[i+1];
	lc->option[optionSize] = '\0';

	return lc;
	
}

//PRINTA UMA LINHA DE COMANDO
void printLc(Linha_de_comando* lc){
	printf("Command: %d\n",lc->command);
	printf("Option: %s\n",lc->option);
}

//PRINTA UMA PACOTE
void printPacket(BYTE* packet){
	printLc(decode(packet));
}
