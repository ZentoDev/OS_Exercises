#include <iostream>
#include <string>
#include<string.h>  
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h> 
#include<pthread.h>
#include <signal.h>
#include <netinet/in.h>

#define MaxClient 5

using namespace std;

//tipo di dato che rappresenta il client nella chat
typedef struct {
    sockaddr_in indirizzo_client;
    int user_id, idsock;
    char nome[16];

} client_type;


//struttura dati che gestisce la coda di client che vogliono connettersi
client_type * multi_client[MaxClient]; 


//PROTOTIPI 
void aggiungiClient(client_type * );
void eliminaClient(int);
void *gestioneConnessioni(void *);
void inviaAtutti(char *, int);
bool controlloUsername(char * , int);


int main() {
 //VARIABILI LOCALI
 int clientSocket, serverSocket; //identificatore socket client
 int PortaS;
 char ipS[16]; 
 int userId = 1;
 struct sockaddr_in indirizzoServer, indirizzoClient; //strutture per socket address server e client
 

 //INPUT
 cout<<"Inserisci un indirizzo IP del Server:"<<endl;
 cout<<">";
 cin>>ipS;
 
 cout<<"Inserisci un numero di porta del Server:"<<endl;
 cout<<">";
 cin>>PortaS;
 
  //creo socket server + controllo
 serverSocket = socket(AF_INET, SOCK_STREAM, 0);
 if(serverSocket < 0) {
    cerr<<"[X] - ERRORE NELLA CREAZIONE SOCKET\n"<<endl;
    return 0;
 }
 cout <<"[i] - Socket server creata con successo!"<<endl;

 //assegna socket address + controllo
 indirizzoServer.sin_addr.s_addr = inet_addr(ipS);
 indirizzoServer.sin_family = AF_INET;
 indirizzoServer.sin_port = htons(PortaS);
 
 //bind + controllo
    if(bind(serverSocket, (struct sockaddr*)&indirizzoServer, sizeof(indirizzoServer)) < 0) {
        cerr<<"[X] - ASSEGNAZIONE INDIRIZZI FALLITA\n"<<endl;
        close(serverSocket);
        return 0;
    }
  
 cout<<"[i] - Indirizzo IP e Porta assegnati con successo!\n"<<endl;
 
 //set della socket server all'ascolto + controllo
 if(listen(serverSocket , MaxClient) < 0) {
        cerr<<"[X] - ATTIVAZIONE ASCOLTO SU SOCKET FALLITA\n"<<endl;
        close(serverSocket);
        return 0;
    }
 
 cout<<"/***********************CHATROOM CREATA***********************/\n"<<endl;
 pthread_t thread_id;
 //ACCETTAZIONE NUOVE CONNESSIONI
 do { 
 	  socklen_t clientLen; //ad ogni ciclo inizializziamo la variabile che indica la dimensione del client socket
      clientLen = sizeof(indirizzoClient);
      
	  clientSocket = accept(serverSocket, (struct sockaddr *)&indirizzoClient, &clientLen);
         
      if(clientSocket == -1) {
        cerr << "[X] - CONNESSIONE FALLITA\n"<<endl;
      } 
	  	   
      else {
        cout << "[i] - Connessione accettata!\n"<<endl;
        
        //creazione dell'oggetto "nuovo cliente connesso"
        client_type *client = (client_type*)malloc(sizeof(client_type)); //in c++ si dovrebbe usare la funzione new invece che malloc
        if(!(client)) { 
         cerr<<"[X] - ALLOCAZIONE FALLITA\n"<<endl;
         return 0;
		}
		
		//assegno i parametri al client
	    client->indirizzo_client = indirizzoClient;
	    client->idsock = clientSocket;
	    client->user_id = userId; 
	    userId++;

	    //aggiungo il client alla struttura multiclient
	    aggiungiClient(client);
	    
	    //creazione thread + controllo
	    if (pthread_create(&thread_id , NULL ,  gestioneConnessioni , (void*)client) < 0) { //la connessione di ogni client è gestita da un singolo thread
            cerr<<"[X] - IMPOSSIBILE CREARE UN THREAD\n"<<endl;
            close(client->idsock);
            eliminaClient(client->user_id);
            return 0;
        }    
    }
         
   } while(true);
 
return 0;
}


//aggiungo un client nel buffer multiclient, nel primo posto libero
void aggiungiClient(client_type *cli) {

 int successo = 0;
 for(int i = 0;(i < MaxClient && !successo); ++i) {
    if(!multi_client[i]) {
        multi_client[i] = cli;
        successo = 1;
    	}
    }
    
 if(!successo) {
 	cerr<<"[X] - CLIENT NON AGGIUNTO ALLA CHAT\n"<<endl;
    }

}


//rimozione del client dalla struttura multiclient
void eliminaClient(int uid) {

 int successo = 0;
 
 for(int i = 0; (i < MaxClient && !successo); i++) {
    if(multi_client[i]->user_id == uid) {
        multi_client[i] = NULL;
        successo = 1;
        }
 }
 
 if(!successo) {
 	cerr<<"[X] - CLIENT NON ELIMINATO DALLA CHAT\n"<<endl;
 }
 
}


//funzione che invia un messaggio via broadcast
void inviaAtutti(char *mex, int id) {


 for(int i = 0; i < MaxClient; ++i) {
    if(multi_client[i]) {
        if(multi_client[i]->user_id != id) {
            send(multi_client[i]->idsock, mex, strlen(mex), 0); 
		}
    }
 }
}


//implementazione dell'applicazione chat
void * gestioneConnessioni(void *arg) {
 char nick[32];
 char buffer[1024]; //buffer utile all'invio broadcast dell'avviso di una nuova connessione da parte di un client (passato alla funzione inviaAtutti)
 //cast dell'argomento
 int controllo_nick;
 client_type *aux_cli= (client_type*)arg;
 
 int size_nome = recv(aux_cli->idsock, nick, 32, 0);
 strcpy(aux_cli->nome, nick);
 //Leggo dalla socket il nickname inserito dal client
 if (size_nome <= 0) {
		cerr << "[X] - ERRORE RICEZIONE NOME\n";
		close(aux_cli->idsock);
		eliminaClient(aux_cli->user_id);
		return NULL;
 }

 nick[size_nome] = '\0';
 //controllo che il nickname inserito non sia stato usato da qualche altro client
 if(controllo_nick = controlloUsername(nick, aux_cli->user_id)) { 
    char errore[] = "[i] - Mi dispiace, ma questo username e' gia stato utilizzato\n[i] - Prova a connetterti con un altro username\n";
    if (send(aux_cli->idsock, errore, sizeof(errore), 0) < 0) {
		cerr <<"[X] - ERRORE TRASMISSIONE\n"<<endl;
		close(aux_cli->idsock);
		eliminaClient(aux_cli->user_id);
		return NULL;
	}    
    close(aux_cli->idsock);
 }
    
 //invia a tutti i client la notizia che un nuovo client si è connesso
 else if (!controllo_nick) { 
 	char avviso[] = "/***********************BENVENUTO NELLA CHAT*********************/\n[i] - Per uscire dalla chat scrivi 'exit'\n";
 	if(send(aux_cli->idsock, avviso, sizeof(avviso), 0) < 0) {
 		cerr<<"[X] - ERRORE TRASMISSIONE\n"<<endl;
 		close(aux_cli->idsock);
 		eliminaClient(aux_cli->user_id);
 		return NULL;
	 }
	strcpy(buffer, "[+] - Il client ");
	strcat(buffer, nick);
	strcat(buffer, " si e' connesso\n");
	cout<< buffer<<endl;
	inviaAtutti(buffer, aux_cli->user_id);
 }
    
	
 while(true) {
	char buf[1024]; //utile a contenere il messaggio inviato dal client e verrà inviato a tutti gli altri
	char buf1[2048]; //utile a stampare gli errori
	
	//leggo il messaggio del client + controllo
	int esitoRecv = recv(aux_cli->idsock, buf, 1024, 0); 
	if(esitoRecv < 0) {
		cerr<<"[X] - ERRORE NELLA RICEZIONE DEL MESSAGGIO"<<endl;
		close(aux_cli->idsock);
		eliminaClient(aux_cli->user_id);
		break;
    }
    
    //se il client vuole uscire dalla chat
    else if(esitoRecv == 0 || !strcmp(buf, "exit")) {
    	strcpy(buf1, "\n[-] - Il client ");
		strcat(buf1, nick);
		strcat(buf1, " ha lasciato la chat\n");
		cout<<buf1 <<endl;
		inviaAtutti(buf1, aux_cli->user_id);
		close(aux_cli->idsock);
		eliminaClient(aux_cli->user_id);
		break;
	}
	//inoltro il messaggio a tutti i client presenti in chatroom
	if(strlen(buf) > 0) {
	buf[esitoRecv] = '\0';
	strcpy(buf1, nick);
	strcat(buf1, ": ");
	strcat(buf1, buf);
	cout<<buf1 <<endl;		
	inviaAtutti(buf1, aux_cli->user_id);
    }
 } 
return NULL;
}


//controllo che il nickname n venga utilizzato da un solo client (user_id != id)
bool controlloUsername(char * n , int id) {
	
    for(int i = 0; i < MaxClient; ++i) { //scandisco tutta la struttura multiclient
     if(multi_client[i]) { //verifico se la cella i è occupata
        if(multi_client[i]->user_id != id) { //se si allora confronto gli id, per confrontare due client diversi 
            if(!strcmp(multi_client[i]->nome, n)) { //se diversi allora confronto i nickname
                return true; //se i nickname sono uguali allora return true
            }
        }
     }
    }
return false; //altrimenti return false
}







