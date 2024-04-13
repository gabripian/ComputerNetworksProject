#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <regex.h> //serve per il controllo del pattern

#define MESSAGE_LEN 20

#define BUFFER_SIZE 1024
#define CHECK_LEN 5
#define CHECK1_LEN 8
#define CODE_LEN 20
#define RESPONSE_LEN 1024 //risposta inviata dal server

int main(){
        //ret è per la connect, sd per la socket, len e ret1 per la recv

        fd_set master;
        fd_set read_fds;
        int fdmax;
        int ret, sd, len; 
        int data1; //data1 serve per il controllo del pattern
        regex_t r1; //per il controllo del pattern

        struct sockaddr_in server_addr;
        int i, new_sd=0;

        char buffer[BUFFER_SIZE];   //inserisco i comandi nel buffer
        char ricevo[BUFFER_SIZE];   //contiene ciò che ricevo dals erver
        char check[CHECK_LEN]; //primi 4 caratteri della stringa per il controllo
        char check1[CHECK1_LEN]; //primi 7 caratteri della stringa per il controllo di comanda
        char code[CODE_LEN];              // codice inserito dall'utente, si suppone alfanumerico
        char code1[CODE_LEN];               //codice che si recupera dalla comande
        int valid = 0;       //serve per controllare se il codice è valido
        char support[BUFFER_SIZE];    //serve come supporto per inviare il codice
        char str[100];              //serve per leggere i file
        char req_conto[100];        //contiene la parte finale del file comande per verificare se il conto si può mandare
        uint32_t valid1;           
        uint32_t conto1;
        FILE* fd;
        int conto=0;            //contiene il conto 
        int numero_comanda=0;   //numero di comanda progressivo
        char numero;            //numero effettivo inserito
        int device=2;           //identifica il tipo di client
        uint32_t device1;
    

        //creazione socket
        sd = socket(AF_INET, SOCK_STREAM, 0);

        if(sd == -1){

                perror("Error: ");
                exit(1);
        }

        //creazione indirizzo del server
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;

	    server_addr.sin_port = htons(4242);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        ret = connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr));

        if(ret == -1){

            perror("Error: ");
            exit(1);
        }
        //si invia al server il tipo di device
        device=2;
        device1=htonl(device);
        ret = send(sd, (void*)&device1, sizeof(uint32_t), 0);

        if(ret == -1){

            perror("Error: ");
            exit(1);
        }

        FD_ZERO(&master);
        FD_ZERO(&master);

        FD_SET(sd, &master);
        FD_SET(0, &master);     //si inserisce anche il descrittore dello stdin
        fdmax = sd;

        printf("************ BENVENUTO *************\n\n");
        printf("Inserisci il codice:\n\n");
        for(;;){
            read_fds=master;
            ret=select(fdmax+1, &read_fds, NULL, NULL, NULL);
            for(i=0; i <=fdmax; i++){
                if(FD_ISSET(i, &read_fds)){
                     
                        if(i==sd){
                            printf("invio dal server rilevato!\n");
                            memset(&ricevo, 0, sizeof(ricevo)); 
                            FD_SET(new_sd, &master);
                            if(new_sd > fdmax){fdmax = new_sd;}
                            //si riceve dal server la comanda nello stato aggiornato
                            ret=recv(i, (void*)ricevo, sizeof(ricevo),0);
                            printf("ricevo=%s", ricevo);
                            //se ricevo quit, esco
                            if(strcmp(ricevo, "quit")==0){
                                fflush(stdout);
                                FD_CLR(i, &master);
                                    exit(0);
                                }
                                memset(&ricevo, 0, sizeof(ricevo));
                                printf("\n");
                                printf("Digita un comando:\n\n");
                                    
                        }else if(i==0){

                            //standard input
                            memset(&buffer, 0, sizeof(buffer));
                            //si cicla se il codice digitato non è vallido
                            while(valid == 0){
                                //altrimenti concatena i codici
                                memset(&support, 0, sizeof(support));
                                memset(&code, 0, sizeof(code));
                                printf("Inserisci il codice:\n\n");
                                fgets(code, 20, stdin);
                                len = strlen((void *)code);

                                //controllo correttezza lunghezza codice
                                if(len != 4){
                                    printf("lunghezza codice non corretta\n");
                                    continue;
                                }
                                //si mette una h così il server sa che deve controllare il codice
                                support[0]='h';
                                strcat(support, code);
                                    
                                //invio codice al server
                                ret = send(sd, (void*)support, sizeof(support), 0);
                                if(ret == -1){
                                    perror("Error: ");
                                    exit(1);
                                }
                                //si riceve se il codice è valido o no            
                                ret = recv(sd, (void*)&valid1, sizeof(uint32_t), 0);
                                if(ret == -1){
                                    perror("Error: ");
                                    exit(1);
                                }
                                valid=ntohl(valid1);
                                    //se il server ci restituisce 1, il codice è valido
                                    
                                    if(valid == 1){
                                        printf("codice valido\n");
                                    }else{
                                        //se il codice non è valido ne chiedo un altro
                                        printf("codice non valido\n");
                                    }
                                }
                            
                                printf("\n");
                                printf("Digita un comando:\n\n");
                                printf("help --> mostra i dettagli dei comandi\n");
                                printf("menu --> mostra il menu dei piatti\n");
                                printf("comanda  --> invia una comanda\n");
                                printf("conto  --> chiede il conto\n\n");
                    
                                memset(&buffer, 0, sizeof(buffer));
                                printf("Inserire comando: \n");
                                fgets(buffer, 1024, stdin);
                                len = strlen((void*)buffer);
                            
                                strncpy(check, buffer, 5);
                                strncpy(check1, buffer, 8);

                                if(strcmp(check, "help\n")==0){
                                        
                                    printf("il comando è help\n\n");

                                    printf("Digitando il comando 'menu' verrà mostrato il menu giornaliero\n");
                                    printf("Digitando il comando 'comanda' verranno inviati i piatti selezionati\n");
                                    printf("il comando deve avere la seguente forma: 'comanda piatto1-quantità1....piatton-quantitàn'\n");
                                    printf("Digitando il comando 'conto' si richiede il conto\n\n");
                    
                                }else if(strcmp(check, "menu\n")==0){
                                                
                                    printf("il comando è menu\n\n");
                                    printf("A1 - ANTIPASTO DI TERRA             7\n");
                                    printf("A2 - ANTIPASTO DI MARE              8\n");
                                    printf("P1 - SPAGHETTI ALLO SCOGLIO         12\n");
                                    printf("P2 - PENNE CACIO E PEPE             7\n");
                                    printf("S1 - FRITTURA DI CALAMARI           20\n");
                                    printf("S2 - ARROSTO E PATATE               15\n");
                                    printf("D1 - TIRAMISU                       5\n");
                                    printf("D2 - CHEESCAKE                      5\n\n");

                                }else if(strcmp(check1, "comanda ")==0){

                                    printf("il comando è comanda\n");
                                    //bisogna azzerare support
                                    memset(&support, 0, sizeof(support));
                                    data1 = regcomp( &r1, "comanda [A,P,S,D][1-2][-][1-8]", REG_EXTENDED);
                                    data1 = regexec( &r1, buffer, 0, NULL, 0);

                                    printf("DATA1 =%d\n", data1);
                                    if(data1 == 0){

                                            printf("comanda valida\n");
                                            //si invia anche il codice separato da uno spazio, si memorizza anche il codice,
                                            //per risalire al tavolo
                                            //non copio l'invio (\n)
                                            //e si mette in attesa
                                            numero_comanda++;
                                            numero = numero_comanda + '0';
                                            strncpy(support, buffer, len-1);
                                            strcat(support, " ");
                                            strncpy(code1, code, strlen(code)-1);
                                            strcat(support, code1);
                                            strcat(support, " ");
                                            support[strlen(support)] = numero;
                                            strcat(support, " IN ATTESA\n");
                                            printf("support=%s\n", support);
                                            //si suppone di avere al massimo 9 comande
                                            //per la gestione del numero di comanda nel file
                                            if(numero_comanda<=9){
                                                //invio comanda
                                                ret = send(sd, (void*)support, sizeof(support), 0);
                                                if(ret == -1){
                                                    perror("Error: ");
                                                    exit(1);
                                                }
                                                //si riceve la comanda                               
                                                ret = recv(sd, (void*)buffer, RESPONSE_LEN, 0);
                                                if(ret == -1){
                                                    perror("Error: ");
                                                    exit(1);
                                                }
                                                printf("COMANDA RICEVUTA\n");
                                                //si stampano le comande in attesa
                                                printf("comande in attesa\n");
                                                printf("%s\n", buffer);
                                                
                                            }else
                                            {
                                                printf("numero di comande esaurite\n");
                                            }
                                            
                                    }else{
                                            printf("comanda non valida\n");
                                    }
                                    
                                }else if((strcmp(check1, "conto\n")==0) && (len == 6)){

                                    printf("il comando è conto\n");
                                    conto=0;
                                    fd = fopen("comande", "r");
                                                
                                    if (NULL == fd) {

                                        printf("file can't be opened \n");
                                    }
                                        //si cerca nel file delle comande, se ci sono comande in servizio
                                    while(fgets(str, 100, fd)!= NULL){
                                        
                                        strcpy(req_conto, str);
                                        //si cerca se c'è il codice in una comanda
                                        if(strstr(str, code1)){
                                            //se la comanda è in servizio si incrementa il contatore
                                            //non si può richiedere il conto
                                            if(strstr(str, "IN SERVIZIO")){
                                                conto++;
                                            }else{
                                                //appena trova una comanda dell'utente con uno stato diverso non richiede il conto
                                                conto=0;
                                                break;
                                            }
                                        }
                                    }
                                    fclose(fd);
                                    
                                    if(conto==0){
                                        //bisogna attendere prima di richiedere il conto
                                        printf("attendere il completamento del pasto prima di richiedere il conto\n");
                                    }else{
                                        //gestione conto
                                        //con il conto si invia il codice per calcolarlo
                                        strncpy(buffer, "conto ", 6);
                                        strcat(buffer, code);
                                        //si invia la richiesta del conto assieme al codice
                                        ret = send(sd, (void*)buffer, sizeof(code), 0);
                                        if(ret == -1){
                                            perror("Error: ");
                                            exit(1);
                                        }
                                        //si riceve il conto
                                        ret = recv(sd, (void*)&conto1, sizeof(uint32_t), 0);
                                        if(ret == -1){
                                            perror("Error: ");
                                            exit(1);
                                        }
                                        conto=ntohl(conto1);
                                        printf("IL conto è: %d\n\n", conto);
                                        //si riparte dal codice
                                        numero_comanda=0;
                                        valid=0;
                                        printf("************ BENVENUTO *************\n\n"); 
                                    }

                                }else{
                                        printf("comando non valido\n"); 
                                }
                                memset(&buffer, 0, sizeof(buffer));
                                
                                printf("\n");
                                printf("Digita un comando:\n\n");
                                printf("help --> mostra i dettagli dei comandi\n");
                                printf("menu --> mostra il menu dei piatti\n");
                                printf("comanda  --> invia una comanda\n");
                                printf("conto  --> chiede il conto\n\n");
                    
                                printf("Inserire comando: \n");
                                

                    }else{

                        if(ret==0){
                            fflush(stdout);
                            // Tolgo il descrittore del socket connesso dal
                            // set dei monitorati
                            FD_CLR(i, &master);
                        }else if(ret <0){
                            fflush(stdout);
                            // Tolgo il descrittore del socket connesso dal
                            // set dei monitorati
                            FD_CLR(i, &master);
                        }

                    }
                
            }
        }
    }
    return 0;
}
