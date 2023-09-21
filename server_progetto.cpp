/*Applicativo client-server 

Scrivere un applicativo client-server dove il server salva su un file i dati degli utenti che si collegano seguendo questo protocollo:

    Il client si collega al server mandando un messaggio di saluto es "Buongiorno sono Pippo"
    Il server risponderà " buongiorno  Pippo sono collegato con altri x utenti inviami il tuo nome,cognome".
    Il client invia i dati
    il server li memorizza su file
    Finita la memorizzazione il sever saluta il client.
*/

#include <iostream>

#include <string>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <fstream>
#include <pthread.h>
#include <atomic>
#include <semaphore.h>

using namespace std;
#define MaxClient 2

// DICHIARAZIONE FUNZIONI
void *connect(void *);

// VARIABILI GLOBALI
atomic_int countClient = 0; // contiene il numero di client collegati
sem_t semaphore;            // Dichiarazione del semaforo
int socketArray[MaxClient]; // contiene i descrittori delle socket dei client

int main()
{

   // VARIABILI LOCALI
   int clientSocket, serverSocket; // identificatori socket
   int PortaS;
   char ipServizio[16], input[4];
   int i;
   struct sockaddr_in indirizzoServer, indirizzoClient; // strutture per socket address server e client
   sem_init(&semaphore, 0, 1);                          // Inizializzazione del semaforo

   // decido di rappresentare l'assenza di una socket con il valore '-10'
   for (i = 0; i < MaxClient; i++)
      socketArray[i] = -10;

   // Richiesta IP e PORTA sui quali implementare il servizio, inserendo 'd' si ottiene un indirizzo di default (ip '127.0.0.1, porta '2005')
   cout << "Inserisci un indirizzo IP del Server (inserisci 'd' per ottenere indirizzo di default '127.0.0.1'):\n";
   cout << ">";
   cin >> ipServizio;
   if (ipServizio[0] == 'd')
      strcpy(ipServizio, "127.0.0.1");

   cout << "Inserisci un numero di porta del Server (inserisci 'd' per ottenere la porta di default '2005'):\n";
   cout << ">";
   cin >> input;
   if (input[0] == 'd')
      PortaS = 2005;
   else
      PortaS = atoi(input);  // atoi() converte una stringa in un intero

   // Creiamo il socket per il servizio
   // - famiglia protocolli AF_INET => IPV4
   // - trasporto SOCK_STREAM => TCP, SOCK_DGRAM => UDP
   // - protocollo 0 => IP AUTO

   // socket server + controllo
   serverSocket = socket(AF_INET, SOCK_STREAM, 0);
   if (serverSocket < 0)
   {
      cerr << "[X] - ERRORE NELLA CREAZIONE SOCKET\n\n";
      return -1;
   }
   cout << "[i] - Socket server creata con successo!\n";

   // Indichiamo l'interfaccia e la porta di ascolto
   // assegna socket address + controllo
   indirizzoServer.sin_addr.s_addr = inet_addr(ipServizio);  // inet_addr() coverte un numero in notazione puntata in un numero a 32 bit
   indirizzoServer.sin_family = AF_INET;
   indirizzoServer.sin_port = htons(PortaS);  // htons() converte un numero al formato Big-Endian

   // Chiedo al sistema operativo di installare il servizio
   // bind + controllo
   if (bind(serverSocket, (struct sockaddr *)&indirizzoServer, sizeof(indirizzoServer)) < 0)
   {
      cerr << "[X] - ASSEGNAZIONE INDIRIZZI FALLITA\n\n";
      close(serverSocket);
      return -1;
   }

   cout << "[i] - Indirizzo IP e Porta assegnati con successo!\n\n";

   // set della socket server all'ascolto + controllo
   if (listen(serverSocket, MaxClient) < 0)
   {
      cerr << "[X] - ATTIVAZIONE ASCOLTO SU SOCKET FALLITA\n\n";
      close(serverSocket);
      return -1;
   }

   cout << "Server creato, ascolto in corso...\n\n";

   pthread_t thread_id;
   // ACCETTAZIONE NUOVE CONNESSIONI
   do
   {
      socklen_t clientLen = sizeof(indirizzoClient); // ad ogni ciclo inizializziamo la variabile che indica la dimensione del client socket

      clientSocket = accept(serverSocket, (struct sockaddr *)&indirizzoClient, &clientLen);

      if (clientSocket == -1)
         cerr << "[X] - CONNESSIONE FALLITA\n\n";

      else
      {
         cout << "[i] - Connessione accettata!\n\n";

         if (++countClient <= MaxClient)
         { // verifico che non si sia raggiunto il limite di connessioni

            // cerco un elemento libero nel vettore di socket collegati
            while (socketArray[i] != -10)
               i = (i + 1) % MaxClient;

            // una volta trovato, salvo il valore del descrittore del socket che si è appena collegato e lo passo al thread
            socketArray[i] = clientSocket;
            // creazione thread + controllo
            if (pthread_create(&thread_id, NULL, connect, (void *)&socketArray[i]) < 0) // Si crea un thread per ogni connessione con un client
            { 
               cerr << "[X] - IMPOSSIBILE CREARE UN THREAD\n\n";
               close(clientSocket);
               countClient--;
               return -1;
            }
         }
         else
         {
            cerr << "[X] - E' STATO RAGGIUNTO IL LIMITE DI THREAD ATTIVI CONTEMPORANEAMENTE (" << MaxClient << ")\n\n";
            close(clientSocket);
            countClient--;
         }
      }

   } while (true);

   sem_destroy(&semaphore); // Distruzione del semaforo
   return 0;
}

// DEFINIZIONE FUNZIONI
void *connect(void *clientSoc)
{
   char nome[24], cognome[24], comunicazione[62];
   int *client = (int *)clientSoc;

   // ricevo il nome del client e verifico che sia stato ricevuto correttamente
   int size_nome = recv(*client, nome, sizeof(nome), 0);
   if (size_nome <= 0)
   {
      cerr << "[X] - ERRORE RICEZIONE NOME\n";
      close(*client);
      *client = -10;
      countClient--;
      return NULL;
   }
   nome[size_nome] = '\0';
   cout << nome << " si è collegato\n";

   sprintf(comunicazione, "Benvenuto %s, ci sono %d client collegati\n", nome, (int)countClient);

   if (send(*client, comunicazione, sizeof(comunicazione), 0) < 0)
   {
      cerr << "[X] - ERRORE TRASMISSIONE\n"
           << endl;
      close(*client);
      *client = -10;
      countClient--;
      return NULL;
   }

   // ricevo il cognome del client e verifico che sia stato ricevuto correttamente
   int size_cognome = recv(*client, cognome, sizeof(cognome), 0);
   if (size_cognome <= 0)
   {
      cerr << "[X] - ERRORE RICEZIONE COGNOME\n";
      close(*client);
      *client = -10;
      countClient--;
      return NULL;
   }
   cognome[size_cognome] = '\0';

   cout << "cognome ricevuto: " << cognome << "\n";

   // SALVATAGGIO FILE
   sem_wait(&semaphore); // acquisizione del semaforo

   ofstream file("output.txt", ios::app); // Apriamo il file in modalità di append
   if (file.is_open())
   {
      // Aggiungiamo la stringa al file
      file << nome << " " << cognome << endl;

      // Chiudiamo il file
      file.close();
      cout << "Dati aggiunti al file." << endl;
   }
   else
   {
      cout << "Impossibile aprire il file." << endl;
   }

   sem_post(&semaphore); // rilascio del semaforo

   strcpy(comunicazione, "Le tue informazioni sono state salvate correttamente");
   if (send(*client, comunicazione, strlen(comunicazione), 0) < 0)
   {
      cerr << "[X] - ERRORE TRASMISSIONE\n"
           << endl;
      close(*client);
      *client = -10;
      countClient--;
      return NULL;
   }

   cout << "Le informazioni del client di " << nome << " " << cognome << " sono state salvate\nclient scollegato\n";

   close(*client);
   *client = -10;
   countClient--;
   return NULL;
}