# Trabalho 1 - Redes de computadores

## Participantes do grupo
Pedro Ribas Serras - 11234328

## Link apresentacao

https://docs.google.com/presentation/d/1Vrm48ENDmsNgo3ACVTBc8W4mwh7hpFbjkU_eNXZRjy8/edit?usp=sharing

## Sistema operacional

PRETTY_NAME="Ubuntu 22.04.1 LTS"
NAME="Ubuntu"
VERSION_ID="22.04"
VERSION="22.04.1 LTS (Jammy Jellyfish)"
VERSION_CODENAME=jammy
ID=ubuntu
ID_LIKE=debian
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
UBUNTU_CODENAME=jammy

## Compilador

gcc (Ubuntu 11.2.0-19ubuntu1) 11.2.0
Copyright (C) 2021 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


## Requesitos

Como pedido para esse projeto, nenhuma biblioteca de SOCKET externa ao C foi utilizada, mas foi necessario utilizar de coisas externas para cuidar da parte da música. Para baixar os videos do yutube foi utilizada o youtube-dl e para rodar as música foi utilizado o vlc junto da libvlc, sendo que para compilar com a libvlc teve que ser utilizado o PKG-config. Todos os requisitos podem ser instalados utilizando o script de instalação deixado. Por fim também foi utilizado o pthreads para manipulação de threads.

Para rodar o script de instalação use o comando:

```
sudo ./install.sh
```

Criar a pasta videos que vai receber os videos(ela deve ser criada na mesma pasta do objeto final do cliente e o nome precisa ser videos):

```
mkdir videos
```

Para compilar o programa utilize o comando:

```
make
```

Para rodar o servidor utilize:
```
./server
```

Para rodar o cliente utilize:
```
./client
```

Importante dizer que o cliente só pode ser iniciado depois do servidor, caso contrário o servidor clinete é desligado. Ultima coisa é que deve-se rodar o cliente onde esta o arquivo e a pasta videos.

## Objetivos

O objetivo do projeto era fazer um conjunto de raspberry pi sincronizar a música utilizando o socket. No projeto foi utilizado o socket, a sincronização não teve um bom resultado. Na verdade essa não foi a ultima vesão do projeto, já que a ultima verção tentou utilizar um broadcast UPD para a sincronização, o que funcionou em parter, mas acabou deixando o código mais instával e por isso essa foi a versão enviada. Mesmo não completando o seu objetivo principal com sucesso, é possível usar esse projeto para tocar musica em outros computadores, sendo apenas um de servidor e outro de caixinha, ou até vários como caixinhas não perfeitamente sincronizadas (isso pode ser utilizado entre amigos para ouvirem a mesma música de longe, com apenas uma deles controlando as caixas).

## Observacoes
- Mesmo essa sendo a versão sendo a mais estável, ela ainda tem alguns problemas de uso. Não sei citar os bugs. Acredito que alguns desse bugs tenham sido concertados em versões futuras, mas o controle com o git não foi muito bom e essas versões foram perdidas (O mané que fez o trabalho fez um squash de varios commits e perdeu algumas coisas).
- Esse projeto foi testado em um RASP 3 também e funciona parcialmente, dando problema de memória.
- O caminho escolhido para armazenar as musicas foram instancias do vlc, o que tornor o programa bem pesado. Caso mate o servidor não esqueça de matar o cliente, pois se ele bugar, em poucos minutos pode lotar toda memória forçar o deligamento do pc na mão (Aconteceu duas vezes ;-;). 
- Como em C não existem estruturas de dados prontas, foi criada uma lista encadeada para podermos utilizar como "Fila".
- Para definir o protocolo que deveria ser seguido tanto pelo servidor quanto pelo cliente, foi criado o arquivo protocolo.c que cria DEFINEs de constantes que devem ser usadas para a comunicação e também cria funções e estruturas que ajudam nassa comunicação.

## Para alcançar o objetivo final...

Para isso faltou um sincronização melhor que poderia ser feito ao colocar threads para o envio mais paralelizado dos TCPs para as caixinhas e utilizar uma sincronização de horários(ou por meio da demora de resposta) sincronizar o inicio da musica nas caixinhas. isso até começou a ser feito em outra versão (Usando o broadcast em udp), mas por conta do tempo não foi terminado e ficou bem instável. De qualquerforma, deixo o git caso alguem queira ver(é o que esta na master): https://github.com/PedroRibasSerras/sincronizador-de-caixinhas

A verdade é que quis fazer um trabalho ambicioso e ainda fiz sozinho, mas acabou não dando certo e gastou mais tempo do que eu realmente tinha para desprender, mas a parte de socket pedido, acredito que tenha sido contemplada.

## Justificativa principal para a utilização do TCP

Ele foi utilizado pela estabilidade em si já que o ideal era que nenhuma caixinha fosse perdida. Quando tentei utilizar o UDP, várias caixinhas eram perdidas por que a mensagem não chegava, ou coisas do genero.