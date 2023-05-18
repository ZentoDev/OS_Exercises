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
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <cstdlib>

using namespace std;


int main()
{

    // VARIABILI LOCALI
    int sockServer, portaServer;
    sockaddr_in indirizzoClient;
    char ipServer[16], input[4], bufferComunication[62];

    // creazione socket server alla quale si connetterà il client
    // La funzione socket ha tre parametri
    // - famiglia protocolli AF_INET => IPV4
    // - trasporto SOCK_STREAM => TCP, SOCK_DGRAM => UDP
    // - protocollo 0 => IP AUTO
    sockServer = socket(AF_INET, SOCK_STREAM, 0);
    char nome[24], cognome[24];

    if (sockServer < 0)
    {
        cerr << "[X] - ERRORE NELLA CREAZIONE DELLA SOCKET\n"
             << endl;
        return -1;
    }

    // Prendo in input indirizzo ip e porta della socket server, inserendo 'd' si ottiene un indirizzo di default (ip '127.0.0.1, porta '2005')
    cout << "Inserisci un indirizzo IP del Server (inserisci 'd' per ottenere indirizzo di default '127.0.0.1'):\n";
    cout << ">";
    cin >> ipServer;
    if (ipServer[0] == 'd')
        strcpy(ipServer, "127.0.0.1");

    cout << "Inserisci un numero di porta del Server (inserisci 'd' per ottenere la porta di default '2005'):\n";
    cout << ">";
    cin >> input;
    if (input[0] == 'd')
        portaServer = 2005;
    else
        portaServer = atoi(input);

    // assegno i valori presi in input alla socket server
    // Indirizzo socket servizio server
    // - famiglia protocolli AF_INET => IPV4
    // - numero porta (convertito in network order)
    // - indirizzo ip (convertito in network order)
    indirizzoClient.sin_addr.s_addr = inet_addr(ipServer);
    indirizzoClient.sin_port = htons(portaServer);
    indirizzoClient.sin_family = AF_INET;

    // Connessione al servizio
    // - endpoint creato con la funzione socket
    // - puntatore alla struttura (attento al casting)
    // - dimensione della struttura
    if (connect(sockServer, (struct sockaddr *)&indirizzoClient, sizeof(indirizzoClient)) < 0)
    {
        cerr << "[X] - ERRORE NELLA CONNESSIONE CON IL SERVER\n"
             << endl;
        close(sockServer);
        return -1;
    }

    cout << "Inserisci il tuo nome:\n";
    cout << "> ";
    cin >> nome;
    if (send(sockServer, nome, sizeof(nome), 0) < 0)
    {
        cerr << "[X] - ERRORE NELL'INSERIMENTO DEL NOME" << endl;
    }

    int size_comunicazione = recv(sockServer, bufferComunication, sizeof(bufferComunication), 0);
    if (size_comunicazione <= 0)
    {
        cerr << "[X] - ERRORE RICEZIONE NOME\n";
        close(sockServer);
        return -1;
    }
    bufferComunication[size_comunicazione] = '\0';
    cout << bufferComunication;

    cout << "Inserisci il tuo cognome:\n";
    cout << "> ";
    cin >> cognome;
    if (send(sockServer, cognome, sizeof(cognome), 0) < 0)
    {
        cerr << "[X] - ERRORE NELL'INSERIMENTO DEL COGNOME" << endl;
    }
    
    size_comunicazione = recv(sockServer, bufferComunication, sizeof(bufferComunication), 0);
    if (size_comunicazione <= 0)
    {
        cerr << "[X] - ERRORE RICEZIONE COGNOME\n";
        close(sockServer);
        return -1;
    }
    bufferComunication[size_comunicazione] = '\0';
    cout << bufferComunication;

    return 0;
}
