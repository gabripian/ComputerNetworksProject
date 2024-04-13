#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <regex.h> //serve per il controllo del pattern

#define MESSAGE_LEN 20

#define BUFFER_SIZE 1024
#define CHECK_LEN 5
#define CHECK1_LEN 100
#define RESPONSE_LEN 1024 //risposta inviata dal server

int first_number(char *, int);
//trova la prima occorrenza di un numero in una stringa
//serve per i confronti delle date nel file e la data nel comando find
int first_number(char * str, int len){

    int j, cont =0;
    for(j = 0 ; j < len ; j++){
        
        //appena trovo un numero esce dal for, cont mi dà la posizione del primo numero
        //serve perchè il cognome inserito ha lunghezza variabile
        if(str[j] >= 49 && str[j] <= 56 ){
            
            break;
            
        }
        cont++;
                                
    }
    return cont;
}
struct date {
        int dd, mm, yy, hh,mn;
};
int date_cmp(struct date d1, struct date d2);
//confronta la data corrente con la data inserita dall'utente
//se quella inserita è minore restituisce 0, altrimenti 1, tiene conto anche dell'orario
int date_cmp(struct date d1, struct date d2){
        int valida=0;
        if(((d1.yy == d2.yy) && (d1.mm > d2.mm)) || ((d1.yy == d2.yy) &&  (d1.mm == d2.mm) &&  (d1.dd > d2.dd)) || ((d1.yy == d2.yy) &&  (d1.mm == d2.mm) &&  (d1.dd == d2.dd) && (d1.hh > d2.hh))){
                //si guarda anche l'ora, se sono le 10 non si può prenotare per le 10
                //nel caso che la data sia la stessa
                        valida=1;
        }
        return valida;
}
int main(){
        int ret, sd, len, i, new_len; 
        int cont = 0, data1; //cont per trovare il primo carattere numerico nella stringa data1 serve per il controllo del pattern
        regex_t r1; //per il controllo del pattern
        
        struct sockaddr_in server_addr;
        char buffer[BUFFER_SIZE];       //buffer d'inserimento
        char check[CHECK_LEN];  //primi 4 caratteri della stringa per il controllo del comando
        char check1[CHECK1_LEN]; //serve per contenere la sequenza dopo il cognome
        char find[BUFFER_SIZE];  //contiene la replica del comando find, che si invia con la book
        int book =0;    //serve per controllare che il book segua un comando find
        int tavoli[10]; //un elemento per ogni tavolo, utile per i tavoli liberi
        int device=1;   //distingue i vari tipi di client
        uint32_t device1;
        time_t t;       //struttura per la data corrente
        struct date d1; //data da confrontare 
        struct date d2; //data da confrontare 
        struct tm tm;    //struttura per la data corrente 
        int data_valida;  //valore a cui si assegna se la data è valida o no

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

        device1=htonl(device);
        ret = send(sd, (void*)&device1, sizeof(uint32_t), 0);

        if(ret == -1){

            perror("Error: ");
            exit(1);
        }
        //len = MESSAGE_LEN;
        printf("************ BENVENUTI *************\n");
        while(1){
                printf("\n");
                printf("Digita un comando:\n\n");
                printf("find --> ricerca disponibilità prenotazione\n");
                printf("book --> invia una prenotazione\n");
                printf("esc  --> termina il client\n\n");

		memset(&buffer, 0, sizeof(buffer));

                printf("Inserire comando: \n");
                //fgets per prendere gli spazi
                fgets(buffer, 1024, stdin);
                len = strlen((void*)buffer);
                cont = 0;

                //check per il controllo della validità del comando
                strncpy(check, buffer, 5);

               //si prende la data corrente
                t = time(NULL);
                tm = *localtime(&t);
                
                d2.yy=tm.tm_year+1900;
                d2.mm=tm.tm_mon+1;
                d2.dd=tm.tm_mday;
                d2.hh=tm.tm_hour;
                d2.mn=tm.tm_min;

                //controllo se il comando è find, book, esc
                //se non lo è non è un comando valido
                //inoltre il comando deve essere di almeno 22 caratteri
                //si suppone un cognome minimo di tre caratteri
                if(strcmp(check, "find ")==0 && len >21){
                        
                        printf("il comando è find\n");
                        //si copia il find nel buffer, se si fa find due volte di fila
                        //viene sovrascritto
                        memset(&find, 0, sizeof(find));
                        strncpy(find, buffer, len);
                       
                        //trovo la posizone del primo carattere numerico
                        //serve perchè nell'espressione del pattern non riesco a prenderlo
                        cont = first_number(buffer,len);

                        //copio la parte dopo il cognome del messaggio
                        strncpy(check1, buffer+cont, len-cont);

                        //controllo se il messaggio è valido
                
                        data1 = regcomp( &r1, "[1-8] [0-3][0-9][-][0-1][0-9][-][2][3] [0-2][0-9]", REG_EXTENDED);
                        data1 = regexec( &r1, check1, 0, NULL, 0);

                        if(data1 == 0){

                                printf("find valido\n");
                                //si assegna alla struttura data la data inserita
                                d1.dd=(check1[2]-'0')*10;
                                d1.dd+=check1[3]-'0';   
                                d1.mm=(check1[5]-'0')*10;
                                d1.mm+=check1[6]-'0';
                                d1.yy=(check1[8]-'0')*10;
                                d1.yy+=check1[9]-'0';
                                d1.yy+=2000;
                                d1.hh=(check1[11]-'0')*10;
                                d1.hh+=check1[12]-'0';
                                d1.mn+=00; 
                                data_valida=date_cmp(d1,d2);

                                if(data_valida==1){
                                        //se la data è valida si invia il comando al server
                                        ret = send(sd, (void*)buffer, len+1, 0);

                                        if(ret == -1){
                                                perror("errore\n");
                                                exit(1);
                                        }
                                        //ricevo il vettore dei tavoli liberi
                                        ret = recv(sd, (void*)tavoli, 40, 0);
                                        //mostro quali tavoli sono liberi
                                        printf("tavoli liberi:\n");
                                        for(i=0; i<10; i++){
                                                //0 significa che è libeo, 1 occupato
                                                if(tavoli[i]==0){
                                                        //i tavoli da 1 a 5 sono in SALA1, gli altri in SALA2
                                                        if(i<=4){                                                
                                                                
                                                                printf("T%d   SALA1\n", i+1);

                                                        }else if(i==9){
                                                                printf("T%d  SALA2\n", i+1);
                                                        }else{
                                                                printf("T%d   SALA2\n", i+1);
                                                        }
                                                }
                                        }
                                        //si controlla se i tavoli sono tutti prenotati o no
                                        for(i=0;i<10;i++){
                                                if(tavoli[i] == 0)
                                                        break;
                                        }
                                        if(i== 10){
                                                printf("Non sono presenti tavoli disponibili a quest'ora e in questa data\n");
                                        }

                                        //se ho fatto la find, posso fare la book
                                        book =1;
                                }else{
                                        printf("data non valida\n");
                                }

                        }else{

                                printf("find non valido\n");
                        }
                        //serve altrimenti se inserisco find con un cognome abbastanza lungo 
                        //senza altre informazioni, e data1 del controllo precedente se valeva 0, manteneva lo stato
                        //se data1 valeva 1 era ok
                        memset(&check1, 0, sizeof(check1));
                        
                        
                //supponiamo di avere 10 tavoli
                }else if((strcmp(check, "book ")==0) && (len == 8 || len == 7)){

                        //controllo se il numero è compreso tra 1 e 10, allora è valido, si controlla anche book =1
                        if((((len ==7) && (buffer[5] >=49) && (buffer[5] <=57)) || ((len ==8) && (buffer[5] ==49) && (buffer[6] == 48))) && (book == 1)){
                                
                                printf("il comando è book\n");
                                //si concatena la find al buffer
                                //si elimina il carattere invio
                                //si conatena la find con due slash
                                strncpy(buffer+len-1, "//", 2);
                                strncpy(buffer+len, find, strlen(find));
                                
                                //nuova lunghezza del buffer avendo concatenato la find
                                new_len=strlen(buffer);
                                if(len == 7){
                                       
                                        //se il tavolo è prenotabile, invia la prenotazione
                                        if(tavoli[buffer[5]-49]==0){
                                                ret = send(sd, (void*)buffer, new_len+1, 0);
                                        }else{
                                                printf("tavolo non disponibile\n");
                                                continue;
                                        }
                                }else if(len == 8){
                                        //se il tavolo è il 10, invia la prenotazione se disponibile
                                        if(tavoli[9]==0){
                                                ret = send(sd, (void*)buffer, new_len+1, 0);
                                        }else{
                                                printf("tavolo non disponibile\n");
                                                continue;
                                        }
                                }
                                memset(&buffer, 0, sizeof(buffer));
                                //si riceve dal server la prenotazione
                                ret = recv(sd, (void*)buffer, 5, 0);

                                if(strstr(buffer, "AAAAA")){
                                        printf("tavolo non più disponibile\n");
                                }else{
                                        printf("PRENOTAZIONE EFFETTUATA\n");
                                        printf("il tuo codice è: ");
                                        for(i=0;i<3;i++){
                                                printf("%c", buffer[i]);
                                        }
                                        printf("\n");
                                        if(len == 7){
                                                printf("il tuo tavolo è T%c", buffer[3]);
                                                if(buffer[3] <= 53)
                                                        printf(" si trova nella SALA1\n");
                                                else
                                                        printf(" si trova nella SALA2\n");
                                        }else if(len == 8){
                                                        printf("il tuo tavolo è T10 si trova nella SALA2\n");

                                        }
                                }
                        }else{
                                printf("book non valida\n");
                                if(book == 0){
                                        printf("usare prima il comando find\n");
                                }
                        }
                        //book si rimette a zero, non consentiamo di fare due book di fila, per fare book bisogna fare la find
                        book = 0;

                }else if((strcmp(check, "esc\n")==0) && (len == 4)){

                        printf("il comando è esc\n");
                        ret = send(sd, (void*)check, strlen((void*)check)+1, 0);
                        //esc chiude il client
                        printf("ARRIVEDERCI\n");
                        //close(sd);
                        exit(0);

                }else{

                        printf("comando non valido!\n");

                }
        }

        return 0;
}
