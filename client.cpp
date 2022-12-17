
// Specifiche: connessione client-server TCP

//  1- creazione socket con analoga funzione
//     (costruisce l'endpoint lato host, ovvero
//      il computer su cui eseguirai questo programma)

//  2- dichiarazione e riempimento struttura
//     dell'altro host (il server in attesa di connessioni)
//     ti servira' l'IP e il numero di PORTA

//  3- connessione all'altro host, verifica e chiusura


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
using namespace std;

int main(int na, char **va)
{
   // Richiesta dati del server
   
   char ipServer[16];
   int  portaServer;
   
   cout << "\nINSERISCI UN IP VERSIONE 4 (dotted decimal)\n";
   cout << "(puoi ricavarlo tramite 'nslookup nomedns')\n";
   cin >> ipServer;
   
   cout << "\nINSERISCI IL NUMERO DI PORTA (valore intero)\n";
   cout << "(se non ricordi i valori guarda '/etc/services')\n";
   cin >> portaServer;
   
   // Il socket e' un handle intero
   // assimilabile a quello di un file
   int socketMio;
   
   // La funzione socket ha tre parametri
   // - famiglia protocolli AF_INET => IPV4
   // - trasporto SOCK_STREAM => TCP, SOCK_DGRAM => UDP
   // - protocollo 0 => IP AUTO
   socketMio = socket(AF_INET, SOCK_STREAM, 0);

   if( socketMio == -1 )
   {
      cerr << "\nCreazione socket fallita\n";
      exit(1);
   }
   
   // Indirizzo socket servizio server  
   // - famiglia protocolli AF_INET => IPV4
   // - numero porta (convertito in network order)
   // - indirizzo ip (convertito in network order)
   sockaddr_in indirizzoServer;
   indirizzoServer.sin_family = AF_INET;
   indirizzoServer.sin_port   = htons(portaServer);
   indirizzoServer.sin_addr.s_addr = inet_addr(ipServer);
   
   // Connessione al servizio
   // - endpoint creato con la funzione socket
   // - puntatore alla struttura (attento al casting)
   // - dimensione della struttura
   
   int esitoConnect = 
      connect(socketMio, (sockaddr *)&indirizzoServer, sizeof(sockaddr_in));
   
   if( esitoConnect == -1 )
   {
      cerr << "\nConnessione al server fallita\n";
      exit(2);
   }
   
   // Connessione avvenuta
   cout << "\nLa connessione e' avvenuta, ";
   cout << "verificalo mediante 'netstat -nt'\n";
   cout << "(hai 20 secondi per farlo)\n";
   sleep(20);
   
   // chiusura del canale di comunicazione
   cout << "\nAdesso chiudo la connessione,\n";
   cout << "verificalo mediante 'netstat -nt'\n";
   close(socketMio);
}