// Specifiche: implementazione servizio
//             singola connessione

//  1- socket: creazione socket
//     (costruisce l'endpoint lato host, ovvero
//      il computer su cui si eseguira' questo programma)

//  2- dichiarazione e riempimento struttura
//     con l'indirizzo IP e la porta con i quali si vuole 
//     implementare il servizio

//  3- bind: associazione servizio IP/PORTA all'endpoint 
//     creato in precedenza con l'istruzione socket

//  4- listen: imposta il socket in modalita' ascolto
//     (modalita' passiva), indicando il numero massimo
//     di richieste da accodare in attesa, se la coda
//     si riempie il client puo' ricevere il rigetto della 
//     richiesta di connessione

//  5- accept: si mette in attesa di una richiesta di 
//     connessione da parte di un client, ritorna il canale
//     di comunicazione sul quale comunicare (socket client)


#include <iostream>
using namespace std;

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

#define NREQUEST 6 // dimensione della coda delle richieste

int main(int na, char **va)
{
   // Richiesta IP e PORTA sui quali implementare il servizio
   
   char ipServizio[16];
   int  portaServizio;
   
   cout << "\nINSERISCI L'IP DI UNA TUA INTERFACCIA (0.0.0.0 = tutte)\n";
   cout << "(puoi ricavarlo mediante il comando /sbin/ifconfig)\n";
   cin >> ipServizio;
   
   cout << "\nINSERISCI IL NUMERO DI PORTA (valore intero)\n";
   cout << "(se non sei amministratore deve essere maggiore di 1023)\n";
   cin >> portaServizio;
   
   // Creiamo il socket per il servizio
   // - famiglia protocolli AF_INET => IPV4
   // - trasporto SOCK_STREAM => TCP, SOCK_DGRAM => UDP
   // - protocollo 0 => IP AUTO

   int serverSocket;
   serverSocket = socket(AF_INET,SOCK_STREAM,0);
   
   if(serverSocket == -1)
   {
      cerr << "CREAZIONE SOCKET FALLITA\n";
      exit(1);
   }   
   
   // Indichiamo l'interfaccia e la porta di ascolto
   sockaddr_in indirizzoApplicazione; 
   indirizzoApplicazione.sin_family = AF_INET;
   indirizzoApplicazione.sin_port = htons(portaServizio);
   indirizzoApplicazione.sin_addr.s_addr = inet_addr(ipServizio);
   
   // Chiedo al sistema operativo di installare il servizio
   int esitoBind = 
        bind(serverSocket,(sockaddr *)&indirizzoApplicazione,
        sizeof(indirizzoApplicazione));
   
   if(esitoBind == -1)
   {
      cerr << "INSTALLAZIONE SERVIZIO FALLITA (bind)\n";
      exit(2);
   }   
   
   // Chiedo al sistema operativo di attivare l'ascolto 
   // sul socket specificato nella bind()
   // gestendo una coda di massimo 6 richieste in attesa
   int esitoListen = listen(serverSocket, NREQUEST);
   
   if(esitoListen == -1)
   {
      cerr << "ATTIVAZIONE ASCOLTO SU SOCKET FALLITA (listen)\n";
      exit(3);
   }   
   
   // Creo le variabili per accettare la richiesta
   sockaddr_in indirizzoClient;
   socklen_t lenIndirizzoClient;
   memset(&indirizzoClient,0,sizeof(sockaddr_in));
   lenIndirizzoClient = sizeof(sockaddr_in);
   
   // Mi metto in attesa di connessione
   // (questa operazione ? bloccante)
   int clientSocket;

   cout << "\nSONO IN ATTESA CONNESSIONE (accept)\n";
   cout << "(prova a connetterti con 'telnet oppure nc')\n";

   clientSocket = accept(serverSocket, 
                    (sockaddr *)&indirizzoClient, &lenIndirizzoClient);
         
   if(clientSocket != -1)
   {
      // Scriviamo sullo schermo i dati dei client
      cout << "\nCONNESSIONE INSTAURATA CON IL SEGUENTE CLIENT\n";
      cout << inet_ntoa(indirizzoClient.sin_addr) << ":";
      cout << ntohs(indirizzoClient.sin_port) << "\n";
         
      // Chiudo la connessione
      cout << "CHIUDO LA CONNESSIONE CON IL CLIENT\n";
      close(clientSocket);
   }
   
   // Chiudo il servizio
   cout << "\nCHIUDO IL SERVIZIO\n";
   close(serverSocket);
}