#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <regex.h> //serve per il controllo del pattern
//struttura per identificare i client
struct MySocket {
    int sd;
    char code[100];
    
};

#define MESSAGE_LEN 20

#define BUF_LEN 1024
#define REQUEST_LEN 100
#define CHECK_LEN 5
int first_number(char *, int);
int* check_booking(char *, int);
int calcola_conto(char *);
void stampa(char *);
//controlla se il tavolo è prenotabile
int* check_booking(char *buffer, int len){

    int cont = 0, j;
    FILE* fd;
    char str[100];              //legge nel file
    static int occupato[10];    //ogni posizione un tavolo, 0 libero, 1 occupato
    char data[100];             //data letta dal file
    char tavolo;
    char data_find[BUF_LEN];    //contiene la data arrivata con la find

    //appena trovo un numero esce dal for, cont mi dà la posizione del primo numero
    //serve perchè il cognome inserito ha lunghezza variabile
    //con il numero controlliamo i tavoli con almeno n posti
    cont = first_number(buffer, len);
    //il comando find ricevuto si salva, perchè quandi si fa la book il buffer viene sovrascritto                  
    //si prende la data arrivata con la find
                    
    for(j=0; j<11; j++){
        data_find[j]=buffer[cont+2+j];
    }
    printf("cont = %d\n", cont);
    printf("data_find = %s\n", data_find);

    printf("numero = %c\n", buffer[cont]);

    fd = fopen("sale", "r");
    if (NULL == fd) {

        printf("il file non può essere aperto\n");
    }
    j = 0;
    //ad ogni find si azzera il vettore dei tavoli con un numero adeguato
    while(fgets(str, 500, fd)!= NULL){
        //se nella riga c'è scritto SALA si va avanti                        
        if((strcmp(str, "SALA1:\n")==0) || (strcmp(str, "SALA2:\n")==0))
            continue;

        if(strlen((void*)str) == 5){

            //se il tavolo ha un numero di posti maggiore o uguale alla prenotazione
            //se ne tiene conto
            if(str[3] >= buffer[cont]){

                occupato[j]=0;
                j++;
 
            }else{
                occupato[j]=1;
                j++;
            }
        //se è il tavolo 10, la stringa ha un carattere in più
        }else if(strlen((void*)str) == 6){

            if(str[4] >= buffer[cont]){

                occupato[j]=0;
            }else{
                occupato[j]=1;
            }
        }
    }
    
    fclose(fd);
    fd = fopen("prenotazioni", "r");
    if (NULL == fd) {
        printf("errore apertura file\n");
    }
                      
    while(fgets(str, 500, fd)!= NULL){

        //ad ogni iterazione si inizia a contare da 0 per ogni riga del file
        memset(&data, 0, sizeof(data));
        cont = 0;
        len = strlen((void*)str);   
        //si trova l'indice del primo numero
        cont=first_number(str,len);

        //si somma 2 a cont e si trova l'indice del primo carattere della data
       // printf("numero =%c\n", str[cont+2]);
        strncpy(data,str+cont+2,11);
        printf("data=%s\n",data);
        //si ricava il numero del tavolo dalla riga di prenotazione
        tavolo=str[len-6];
        //se il carattere è lo '0', è il tavolo 10
        if(tavolo=='0'){
            printf("tavolo = 10\n");
        }else{
            printf("tavolo = %c\n",tavolo);
        }
        

        //se la data è uguale, il tavolo occupati[tavolo] diventa ad 1 se non lo è già
        //se la data è diversa non c'è nessuna prenotazione per quel tavolo
        if(strcmp(data, data_find)==0){
                                
            printf("data uguale\n");
                                
            if(tavolo == '0'){
                occupato[9] = 1;
            }else{
                occupato[tavolo-49]=1;
            }
        }
        printf("libero = %d\n",occupato[tavolo-49]);
        printf("vettore dei tavoli occupati: ");
        for(j=0; j<10; j++){
            printf("%d ", occupato[j]);
        }
        printf("\n");

    }

    fclose(fd);

    return occupato;

}
//trova la prima occorrenza di un numero in una stringa
//serve per i confronti delle date nel file e la data nel comando find
int first_number(char * str, int len){

    int j, cont =0;
    for(j = 0 ; j < len ; j++){
        
        if(str[j] >= 49 && str[j] <= 56 ){
            
            break;
            
        }
        cont++;
                                
    }
    return cont;
}
//calcola il conto
int calcola_conto(char *code1){
    FILE *fd;
    char str[100];          //legge nel file
    char piatto[100];       //contiene il codice del piatto
    int i,j,k;
    int numero;
    int val=0;
    int totale =0;
    char code[100];         //contiene l'argomento passato
    strncpy(code, code1, 3);
    
    fd = fopen("comande", "r");
                                
    if (NULL == fd) {

        printf("il file non può essere aperto\n");
    }
    //si cerca nel file delle comande, se ci sono comande in servizio
    printf("\nCALCOLO CONTO\n");
    while(fgets(str, 100, fd)!= NULL){
        
        if(strstr(str, code)){
            i=0, j=1, k=3, val=0;
            memset(&piatto, 0, sizeof(piatto));
                while(val == 0){            
                    strncpy(piatto, str+i, 2);       
                    numero=str[k] - '0';
                    if(strcmp(piatto, "A1")==0){

                        totale += 7*numero;

                    }else if(strcmp(piatto, "A2")==0){
                        totale += 8*numero;
                    }else if(strcmp(piatto, "P1")==0){
                        totale += 12*numero;
                    }else if(strcmp(piatto, "P2")==0){
                        totale += 7*numero;
                    }else if(strcmp(piatto, "S1")==0){
                        totale += 20*numero;
                    }else if(strcmp(piatto, "S2")==0){
                        totale += 15*numero;
                    }else if(strcmp(piatto, "D1")==0){
                        totale += 5*numero;
                    }else if(strcmp(piatto, "D2")==0){
                        totale += 5*numero;
                    }
                    //si avanza nella stringa dei piatti, se ce ne sono altri
                    i+=5;
                    j+=5;
                    k+=5;
                    //se la stringa non è un codice esce mette val=1 ed esce
                    if(str[i]!='A' && str[i]!='P' && str[i]!='S' && str[i]!='D'){
                        val=1;
                    }
                }         
            }
        }
    fclose(fd);
    printf("TOTALE %d\n", totale);
    return totale;
}
//stampa gli ordini col comando stat
void stampa(char *ordini){

    int len=strlen(ordini);
    int i,b; char a;
    b=1;    //serve perchè dopo due caratteri c'è il - che non si riesce a confrontare
            //con un if, se b=3 allora c'è il trattino e si stampa lo spazio
    char numero[2];    //contiene il numero
    char code[100];      //contiene il codice
    char tavolo[100];    //contiene il tavolo
    FILE* fd;
    char str[100];      //stessa dimensione altrimenti dà problemi
    char ordini1[100];  //stampa i piatti ordinati
    //altrimenti stampa altri caratteri
    memset(&ordini1, 0, sizeof(ordini1));
    memset(&numero, 0, sizeof(numero));
    //se nella stringa siamo in attesa
    if(strstr(ordini, "ATTESA")){
        strncpy(numero, ordini+(len-12), 1);
        strncpy(code, ordini+(len-16), 3);
        strncpy(ordini1, ordini, len-17);

    }else if(strstr(ordini, "SERVIZIO")){
        strncpy(numero, ordini+(len-14), 1);
        strncpy(code, ordini+(len-18), 3);
        strncpy(ordini1, ordini, len-19);

    }else if(strstr(ordini, "PREPARAZIONE")){
        strncpy(numero, ordini+(len-18), 1);
        strncpy(code, ordini+(len-22), 3);
        strncpy(ordini1, ordini, len-23);

    }
    //altrimenti stampa anche dei caratteri diversi
    memset(&tavolo, 0, sizeof(tavolo));
    //si prende il tavolo
    fd = fopen("prenotazioni", "r");
    if (NULL == fd) {
        printf("il file non può essere aperto\n");
    }
    
        while(fgets(str, 100, fd)!= NULL){
            if(strstr(str, code)){
                //se la stringa corrisponde al codice si prende il tavolo
                strncpy(tavolo, str+(strlen(str)-7), 3);
                break;
            }
        }
        fclose(fd);

    printf("com%s", numero);
    printf("  ");
    if(strstr(tavolo, "10")){
        printf("T10  ");
    }else{
        printf("%s  ", tavolo);
    }
     if(strstr(ordini, "ATTESA")){
        printf("  <in attesa>\n");

    }else if(strstr(ordini, "SERVIZIO")){
        printf("  <in servizio>\n");
    }else if(strstr(ordini, "PREPARAZIONE")){
        printf("  <in preparazione>\n");

    }
    
    for(i=0; i<len; i++){
        a=ordini1[i];

       if(a == ' '){
        printf("\n");
        b=0;
       }else if( b == 3)
            printf(" ");
        else
            printf("%c", ordini1[i]);
        b++;
    }
    printf("\n");
    
}

int main(int argc, char* argv[]){

    fd_set master;
    fd_set read_fds;
	int fdmax;
    FILE *fd; //descrittore del file

    struct MySocket sv[20];

    //ret è per la connect, sd per la socket, len e ret1 per la recv
    int ret, sd, new_sd, len, ret1;
	int i, j;
    socklen_t addrlen;              //di questo tipo per lo warning
    char ch;                        //codice h così controlla il codice inserito
    char star[100];                 //asterischi da inviare ai kitchen
    
    struct sockaddr_in my_addr, client_addr;
    char buffer[BUF_LEN];       //buffer di ricezione
    char check[CHECK_LEN];      //serve per controllare i comandi
    char buffer_in[BUF_LEN];    //buffer per l'input da tastiera
    char check_in[CHECK_LEN];           //controllo l'input
    int data1; //data1 serve per il controllo del pattern
    regex_t r1;          //serve nel controllo pattern
    int stat1=0;        //se non è >=1 non stampa stat
    char tavolo_in[100];    //tavolo comando stat
    char code_in[100];      //codice comando stat

    //--------------------variabili utili interazione col client----------------------------------
    
    char find[100];      //serve per salvare il comando find nel file
    char numero[2];     //serve per contenere il numero del tavolo
    char response[5];   //serve per il messaggio di risposta al client
    char response1[BUF_LEN];    //risposta da inviare al table device interessato
    char str[500];              //legge nel file
    char code[4];  //calcolo il codice di prenotazione
    char tavolo;    //serve per cercare il tavolo nel file delle prenotazioni
    int occupato[10]; //vettore, un elemento per tavolo, se è prenotabile 0, altrimenti 1, mantiene i tavoli con numero di posti >= alla richiesta
    char support [100]; //mantiene il find ricevuto nella book
    int* punt;          //puntatore per ottenere il valore di ritorno
    char a;             //controllo di / nella book
    char number;        //numero del tavolo prenotato nel comando book
    int no_booking=0;   //se vale 0 il tavolo è prenotabile, se vale 1 il tavolo non è prenotabile
    //---------------------------------table device-----------------------------------------------
    char check_code[3]; //controlla il codice inserito
    uint32_t val1;        //serve per il controllo del codice, 0 codice errato, 1 codice corretto
    int val;
    char check1[8];         //controllo per la comanda
    char comanda[BUF_LEN];  //serve per scrivere nel file delle comande
    char code_conto[4];     //contiene il codice del conto
    int conto=0;            //contiene il conto da inviare al table
    uint32_t conto1;
    FILE *temp;
    int device;             //indica il tipo di device che si collega
    uint32_t device1;
    char code_comanda[4];   //si prende il codice dalla comanda
    //si inizializza il vettore di strutture
    for(j=0;j<20 ; j++){
        sv[j].sd=0;
        memset(&sv[j].code, 0, sizeof(sv[j].code));
    }

    //creazione socket
    sd = socket(AF_INET, SOCK_STREAM, 0);

    if(sd == -1){
            
        perror("Error: ");
        exit(1);
    }

    //creazione indirizzo del server
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(4242); //scelta a caso
    //inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr);
    my_addr.sin_addr.s_addr = INADDR_ANY;
	
	ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr));

    if(ret == -1){
        
        perror("Error: ");
        exit(1);
    }
    
    ret = listen(sd, 10);

    if(ret == -1){

        perror("Error: ");
        exit(1);
    }

    //si azzerano i set
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    //aggiungo il listener sd al set
    FD_SET(sd, &master);
    FD_SET(0, &master);

    //tengo traccia del maggiore (ora è il listener)
    fdmax = sd;
    
    for(;;){

        read_fds = master;
        
        ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
    
        if(ret < 0){
            
            perror("Errore:");
            exit(1);
        }

        for(i=0; i<=fdmax; i++){

            if(FD_ISSET(i, &read_fds)){

                if(i == sd){

                    printf("Nuovo client rilevato!\n");

                    addrlen = sizeof(client_addr);

                    new_sd = accept(sd, (struct sockaddr*)&client_addr, &addrlen);
                    //si riceve il tipo di dispositivo
                    ret = recv(new_sd, (void*)&device1, sizeof(uint32_t), 0);
                    if(ret==-1){
                        perror("Errore\n");
                        exit(1);
                    }
                    device=ntohl(device1);
                    printf("Il device è: %d\n\n", device);
                    //si assegna ils ocket descriptor al primo elemento libero
                    //e si assegna un codice in base al tipo di dispositivo
                    for(j=0; j<20;j++){
                       if(sv[j].sd==0){
                        //non si tiene traccia dei client, non si invia al client
                        //quando ci si disconnette
                            if(device!=1){
                                sv[j].sd=new_sd;
                                if(device==2)
                                    strncpy(sv[j].code, "BBB", 3);
                                else if(device==3)
                                    strncpy(sv[j].code, "CCC", 3);
                                break;
                            }
                       }
                    }
                    
                    printf("---------------------------\n");
                    //si stampano i descrittori collegati
                    printf("descrittori collegati:\n");
                    for(j=0; j<=fdmax+1;j++){
                        //si stampano solo quelli assegnati
                        if(sv[j].sd!=0){
                            printf("SV[%d].sd=%d\n", j,  sv[j].sd);
                            printf("SV[%d].code=%s\n", j,  sv[j].code);
                        }
                    }

                    if(new_sd == -1){

                        perror("Error: ");
                        exit(1);
                    }

                    FD_SET(new_sd, &master);

                    if(new_sd > fdmax){fdmax = new_sd;}
                //entriamo nello stdin
                }else if(i==0){
                    
                    printf("comandi disponibili: \n");
                    printf("stat----> senza parametri fornisce lo stato di ogni comanda\n");
                    printf("stat----> con tavolo fornisce lo stato di ogni comanda del tavolo\n");
                    printf("stat----> con 'a'/'p'/'s' fornisce lo stato di ogni comanda in attesa/preparazione/servizio\n");
                    printf("stop----> arresta il server\n");
                    memset(&buffer_in, 0, sizeof(buffer_in));
                    memset(&check_in, 0, sizeof(check_in));
                    fgets(buffer_in, 1024, stdin);
                    strcpy(check_in, buffer_in);
                   
                   //controllo comando stat con opzioni
                    if(check_in[5]== 'T' || check_in[5]== 'a' || check_in[5]== 'p' || check_in[5]== 's' ){
                        
                        data1 = regcomp( &r1, "(stat T[1-9]|10)|(stat p)|(stat a)|(stat s)", REG_EXTENDED);
                        data1 = regexec( &r1, buffer_in, 0, NULL, 0);

                        printf("DATA1 =%d\n", data1);
                        if((data1 == 0) && (strlen(buffer_in)>=7 && strlen(buffer_in)<=9)){
                            //se la lunghezza è 9, il comando deve essere stat T10
                            if(strlen(buffer_in)==9){
                                if(strcmp(buffer_in, "stat T10\n")!=0)
                                    printf("stat non valida\n");
                                else{
                                    printf("stat valida\n");
                                    memset(&tavolo_in, 0, sizeof(tavolo_in));
                                    memset(&code_in, 0, sizeof(code_in));
                                    strncpy(tavolo_in, buffer_in+5, 2);
                                    fd = fopen("prenotazioni", "r");
                                    if (NULL == fd) {

                                        printf("file non può essere aperto\n");
                                    }
                                    stat1=0;
                                    while(fgets(str, 100, fd)!= NULL){
                                            
                                        if(strstr(str,"T10")){
                                            strncpy(code_in, str+(strlen(str)-4), 3);
                                            stat1++;
                                            break;
                                        }
                                    }
                                    fclose(fd);
                                    if(stat1==0){
                                        printf("non ci sono prenotazioni per il tavolo selezionato\n");
                                        stat1=1;
                                    }else{
                                        stat1=0;
                                        fd = fopen("comande", "r");
                                        if (NULL == fd) {

                                            printf("file non può essere aperto\n");
                                        }
                                        //si stampano le comande del tavolo T10
                                        while(fgets(str, 100, fd)!= NULL){
                                                
                                            if(strstr(str, code_in)){
                                                stampa(str);
                                                stat1++;
                                            }
                                        }
                                        fclose(fd);
                                    }
                                    if(stat1==0){
                                        printf("non ci sono comande per il tavolo selezionato\n");
                                    }
                                }
                            }else if(strlen(buffer_in)==8){
                                    if(buffer_in[5]=='a' || buffer_in[5]=='p' || buffer_in[5]=='s')
                                        printf("stat non valida\n");
                                    else{
                                        //caso tavolo diverso da T10
                                        printf("stat valida\n");
                                        memset(&tavolo_in, 0, sizeof(tavolo_in));
                                        memset(&code_in, 0, sizeof(code_in));
                                        strncpy(tavolo_in, buffer_in+5, 2);
                                        stat1=0;
                                        fd = fopen("prenotazioni", "r");
                                        if (NULL == fd) {

                                            printf("file non può essere aperto\n");
                                        }
                                        //si cercano le comande del tavolo
                                        while(fgets(str, 100, fd)!= NULL){
                                            
                                            if(strstr(str, tavolo_in)){
                                                strncpy(code_in, str+(strlen(str)-4), 3);
                                                stat1++;
                                                break;
                                            }
                                        }
                                        fclose(fd);
                                        if(stat1==0){
                                            printf("non ci sono prenotazioni per il tavolo scelto\n");
                                            stat1=1;
                                        }else{
                                            stat1=0;
                                            fd = fopen("comande", "r");
                                            if (NULL == fd) {

                                                printf("il file non può essere aperto\n");
                                            }
                                            //caso tavoli diversi dal 10
                                            while(fgets(str, 100, fd)!= NULL){
                                                
                                                if(strstr(str, code_in)){
                                                    stampa(str);
                                                    stat1++;
                                                }
                                            }
                                            fclose(fd);
                                        }
                                        if(stat1==0){
                                            printf("non ci sono comande per il tavolo selezionato\n");
                                        }
                                    }
                            }else if(strlen(buffer_in)==7){
                                printf("stat valida\n");
                                stat1=0;
                                fd = fopen("comande", "r");
                                if (NULL == fd) {

                                    printf("il file non può essere aperto\n");
                                }
                                //stampa le comande in base allo stato
                                while(fgets(str, 100, fd)!= NULL){
                                    
                                    if(buffer_in[5]=='a'){
                                        if(strstr(str, "IN ATTESA")){
                                            stampa(str);
                                            stat1++;
                                        }
                                    }else if(buffer_in[5]=='p'){
                                        if(strstr(str, "IN PREPARAZIONE")){
                                            stampa(str);
                                            stat1++;
                                        }
                                    }else if(buffer_in[5]=='s'){
                                        if(strstr(str, "IN SERVIZIO")){
                                            stampa(str);
                                            stat1++;
                                        }
                                    }
                                }
                                fclose(fd);
                                if(stat1==0){
                                    printf("non ci sono comande in questo stato\n");
                                }
                            }
                           
                        }else{
                            printf("stat non valida\n");
                        }
                        
                   }else if(strcmp(check_in, "stop\n")==0){
                        //si controlla se ci sono ancora comande nel file, se non ci sono
                        //si esce
                        stat1=0;
                        fd = fopen("comande", "r");
                        if (NULL == fd) {

                            printf("il file non può essere aperto\n");
                        }
                                            
                        while(fgets(str, 100, fd)!= NULL){
                                            
                            if(strstr(str, "IN ATTESA") || strstr(str, "IN PREPARAZIONE") || strstr(str, "IN SERVIZIO")){
                                stat1++;
                            }
                        }
                        fclose(fd);
                        if(stat1==0){
                            //si invia quit ai table device e kitchen device ancora connessi
                            for(j=0; j<20; j++){
                                if(sv[j].sd!=0){
                                    strcpy(response, "quit");
                                    ret1=send(sv[j].sd, (void*)response, sizeof(response), 0);
                                    close(sv[j].sd);
                                    FD_CLR(sv[j].sd, &master);
                                }
                            }
                            fflush(stdin);
                            
                            exit(0);
                        }else{
                            printf("ci sono ancora comande da servire\n");
                        }

                    }else if(strcmp(check_in, "stat\n")==0){
                        printf("stat valida\n");
                        //si mostrano le comande
                        fd = fopen("comande", "r");
                        if (NULL == fd) {

                            printf("file can't be opened \n");
                        }
                        //stampa ogni comanda
                        while(fgets(str, 100, fd)!= NULL){
                            stampa(str);
                        }
                        fclose(fd);

                    }else{
                        printf("comando non valido\n");
                    }
                    memset(&check_in, 0, sizeof(check_in));

                    
                }else{
                    //se  non è il listener i, siccome ci sono due socket
                    //è per forza quello connesso, quindi va servito con l'ora
                    memset(&buffer, 0 , sizeof(buffer));
                    memset(&find, 0 , sizeof(find));
                    memset(&numero, 0 , sizeof(numero));
                    //metto la richiesta nel buffer
                    //altrimenti va in loop quando qualcuno si disconnette
                    ch='3';
                    ret = recv(i, (void*)buffer, REQUEST_LEN, 0);
                    //si prende il primo carattere arrivato
                    ch=buffer[0];
                    memset(&check1, 0, sizeof(check1));
                    memset(&check, 0, sizeof(check));
                    strncpy(check, buffer, 5);
                    strncpy(check1, buffer, 7);         //verifica la comanda
                   
                    len = strlen((void*)buffer);
                    
                    if(strcmp(check, "find ")==0){

                        printf("comando find rilevato\n");
                        memset(&response, 0, sizeof(response));
                        //si controlla se il tavolo è prenotabile
                        punt=check_booking(buffer, len);
                        for(j=0; j<10;j++){
                            occupato[j] = *(punt+j);
                        }               
                        printf("\n");

                    }else if((strcmp(check, "book ")==0)){
                        
                        printf("comando book rilevato\n");
                        memset(&find, 0, sizeof(find));
                                                
                        len = strlen((void*)buffer);
                        no_booking=0;
                        //j è la posizione che indica la fine del book
                        for(j=0; j<len; j++){
                            a=buffer[j];
                            if(a =='/')
                                break;
                        }
                        //in support abbiamo il find
                        strncpy(support, buffer+j+1, len-j);
                        //contiene la parte di comando find da memorizzare nel file
                        //con support+5 si parte dal cognome, con strlen(support)-6 si esclude il ritorno carrello
                        strncpy(find, support+5, strlen(support)-6);
                        printf("support=%s\n", support);
                        number=buffer[5];   //prendiamo il primo numero
                        //se il tavolo è il 10, in buffer mettiamo book 10, se è un altro tavolo
                        //mettiamo book numero tavolo
                        if(strstr(buffer, "book 10")){
                            memset(&buffer, 0, sizeof(buffer));
                            strncpy(buffer, "book 10\n", 8);
                        }else{
                            memset(&buffer, 0, sizeof(buffer));
                            strncpy(buffer, "book ", 5);
                            buffer[5]=number;
                            strcat(buffer, "\n");
                        }
                        //si cerca nuovamente i tavol liberi, questo perchè possono essere fatte più find
                        //diverse prima di fare il book
                        len = strlen(support);
                        punt=check_booking(support, len);
                        for(j=0; j<10;j++){
                            occupato[j] = *(punt+j);  
                        }
                         
                        //adesso len è il valore del buffer
                        len = strlen((void*)buffer);
                        if(len == 7){
                            if(occupato[number-49]==1)
                                no_booking=1;
                        }else if(len == 8){
                            if(occupato[9]==1)
                                no_booking=1;
                        }
                        if(no_booking==0){

                            while(1){
                                //si genera il codice casuale di 3 caratteri e ci si mette \n in fondo
                                for(j=0 ; j < 3 ; j++){

                                    code[j]="abcdefghijklmnopqrstuvwxyz0123456789"[random () % 36];
                                }
                            
                                code[3] ='\n';

                                fd = fopen("codici", "r");
                                if (NULL == fd) {

                                    printf("il file non può essere aperto\n");
                                }
                                //si cerca nel file dei codici, se il codice esiste già, se ne crea un altro
                                //altrimenti si esce
                                while(fgets(str, 100, fd)!= NULL){
                                    
                                    if(strcmp(str, code)==0){
                                        j =0;
                                    }
                                }
                                if(j==0){
                                    continue;
                                }else{
                                    break;
                                }
                                
                                fclose(fd);
                            }
                            
                            printf("\n");
                            //si apre il file in append
                            fd = fopen("codici", "a");
                            //si scrive il codice generato in append
                            fprintf(fd, "%s", code);

                            fclose(fd);

                            //si crea la stringa da inserire nel file
                            if(len == 7){
                                strcat(find, " T");
                                numero[0] = buffer[5];
                                strcat(find, numero);
                                strcat(find, " ");
                                strcat(find, code);
                                //costruisco il messaggio da inviare al client, contenente codice e tavolo
                                strncpy(response, code, 3);
                                strcat(response, numero);
                                tavolo=buffer[5];
                                occupato[tavolo-49]=1;
                                for(j=0 ; j < 10 ; j++){
                                    printf("%d\n",occupato[j]);
                                }

                            }else if(len == 8){

                                //il tavolo prenotato è il 10
                                strcat(find, " T10 ");
                                strcat(find, code);
                                //costruisco il messaggio da inviare al client, contenente codice e tavolo
                                strncpy(response, code, 3);
                                strcat(response, "10");
                                occupato[9] = 1;
                                for(j=0 ; j < 10 ; j++){
                                    printf("%d\n",occupato[j]);
                                }
                            }
                            printf("response-> codice e tavolo=%s\n",response);

                            //si inserisce la stringa nel file
                            fd = fopen("prenotazioni", "a");
                            //si scrive il codice generato in append
                            fprintf(fd, "%s", find);

                            fclose(fd);
                        }
                        

                    }else if(strcmp(buffer, "esc\n")==0){
                        //necessario, altrimenti dà errore e si disconnette     
                        ret = 0;
                    }else if(ch=='h'){
                            memset(&check_code, 0, sizeof(check_code));
                            memset(&str, 0, sizeof(str));
                            val=0;
                            printf("table device\n");
                            strcpy(check_code, buffer+1);

                            fd = fopen("codici", "r");
                                
                            if (NULL == fd) {

                                printf("il file non può essere aperto\n");
                            }
                            //si cerca nel file dei codici, se il codice esiste già, se ne crea un altro
                            //altrimenti si esce
                            while(fgets(str, 100, fd)!= NULL){
                                if(strcmp(str, check_code)==0){
                                    val =1;
                                    //si assegna il codice all'elemento del vettore di strutture
                                    //corrispondente
                                    for(j=0; j<20; j++)
                                    {
                                        if(i==sv[j].sd){
                                            strncpy(sv[j].code, check_code, 3);
                                            break;
                                        }
                                    }                        
                                }
                            }
                            fclose(fd);
                            if(val==0){
                                printf("codice non valido\n");
                            }else{
                                printf("codice valido\n");
                            }

                        }else if(strcmp(check1, "comanda")== 0){

                            printf("il comando è comanda\n");

                            strncpy(comanda, buffer+8, len);
                            
                            //si apre il file in append
                            fd = fopen("comande", "a");
                            //si scrive il codice generato in append
                            fprintf(fd, "%s", comanda);

                            fclose(fd);
                            printf("comanda inserita: %s\n", comanda);
                            

                        }else if(strcmp(check, "conto")==0){
                    
                            memset(&code_conto, 0, sizeof(code_conto));
                            strncpy(code_conto, buffer+6, 3);

                            conto = calcola_conto(code_conto);
                            printf("CONTO=%d\n", conto);
                            //si rimuovono dal file le comande del pasto di cui è stato richiesto il conto
                            fd = fopen("comande", "r");
                            temp = fopen("replace.tmp", "w");
                            
                            if (NULL == fd || NULL == temp) {

                                printf("il file non può essere aperto\n");
                            }
                            //si cerca nel file delle comande, se ci sono comande in servizio
                            while(fgets(str, 100, fd)!= NULL){
                                
                                if(strstr(str, code_conto)){
                                    //se la stringa corrisponde al codice non fa niente
                                }else{
                                    //altrimenti ricopia
                                    fputs(str, temp); 
                                }
                            }
                            fclose(fd);
                            fclose(temp);
                            remove("comande");
                            rename("replace.tmp", "comande");
                            ret=100;
                        }else if(strstr(buffer, "IN PREPARAZIONE")){
                            printf("NUOVA COMANDA IN PREPARAZIONE\n");
                        }else if(strstr(buffer, "IN SERVIZIO")){
                            printf("NUOVA COMANDA IN SERVIZIO\n");
                        }

                        if(ret == 0){

                            printf("CHIUSURA client/table/kitchen device rilevata!\n");
                            fflush(stdout);
                            close(i);
                            FD_CLR(i, &master);
                            //si toglie dal vettore l'elemento che si è disconnesso
                            //e si aggiorn ail vettore
                           
                            printf("i= %d\n", i);
                            for(j=0; j<=20; j++)
                            {
                                if(i==sv[j].sd){
                                    sv[j].sd=0;
                                    memset(&sv[j].code, 0, sizeof(sv[j].code));
                                }
                            }
                            //stampo i descrittori aggiornati
                            for(j=0; j<=fdmax+1;j++){
                                if(sv[j].sd!=0){
                                    printf("SV[%d].sd=%d\n", j,  sv[j].sd);
                                    printf("SV[%d].code=%s\n", j,  sv[j].code);
                                }
                            }
                        }else if(ret < 0){

                            perror("ERRORE! \n");
                            close(i);
                            FD_CLR(i, &master);
                        }else{
                            
                            fflush(stdout);
                             if(strcmp(check, "find ")==0){
                            ret1 = send(i, (void*)occupato, 40, 0);

                            if(ret1 == -1){

                                perror("Error: ");
                        
                            }

                        }else if((strcmp(check, "book ")==0)){

                            //se il comando ricevuto è book, invio il codie e i tavoli
                            if(no_booking==0){
                                ret1 = send(i, (void*)response, 5, 0);
                                if(ret1 == -1){
                                    perror("Error: ");
                                }
                            }else{
                                strncpy(response, "AAAAA", 5);
                                ret1 = send(i, (void*)response, 5, 0);
                                 if(ret1 == -1){
                                    perror("Error: ");                        
                                }
                            }
                           
                        }else if(ch=='h'){
                                //si restituisce val, se 1, il codice è corretto, altrimenti no
                                val1=htonl(val);
                                ret1 = send(i, (void*)&val1, sizeof(uint32_t), 0);
                                if(ret1 == -1){
                                    perror("Error: ");
                                }
                            }else if(strcmp(check1, "comanda")==0){
                                
                                //si cercano le comande in attesa dell'utente
                                memset(&response1, 0, sizeof(response1));
                                memset(&code_comanda, 0, sizeof(code_comanda));
                                strncpy(code_comanda, buffer+strlen(buffer)-16, 3);
                                fd=fopen("comande", "r");
                                if (NULL == fd) {
                                printf("il file non può essere aperto\n");
                                }
                               
                                while(fgets(str, 100, fd)!= NULL){
                                    //se la comanda è in attesa ed è relatva al table si assegna
                                    if(strstr(str, "IN ATTESA") && strstr(str, code_comanda)){
                                        strcat(response1, str);
                                    }
                                }
                                fclose(fd);
                                //si inviano le comande in attesa al table interessato
                                ret1 = send(i, (void*)response1, sizeof(response1), 0);
                                if(ret1 == -1){
                                    perror("Error: ");
                                }
                                memset(&star, 0, sizeof(star));
                                
                                for(j=0; j<20 ; j++){
                                    //si inviano gli asterischi ad ogni kitchen
                                    if(strcmp(sv[j].code, "CCC")==0){
                                        
                                        fd = fopen("comande", "r");
                                        if (NULL == fd) {

                                            printf("il file non può essere aperto\n");
                                        }
                                        while(fgets(str, 100, fd)!= NULL){   
                                            if(strstr(str, "ATTESA")){
                                                    strcat(star, "*"); 
                                            }
                                        }
                                        fclose(fd);
                                         ret1 = send(sv[j].sd, (void*)star, sizeof(star), 0);
                                         memset(&star, 0, sizeof(star)); //sennò altri device hanno più asterischi
                                         if(ret1==-1){
                                            perror("errore\n");
                                            exit(1);
                                         }
                                    }
                                }

                            }else if(strcmp(check, "conto")== 0){
                                //si invia il conto
                                conto1=htonl(conto);
                                ret1 = send(i, (void*)&conto1, sizeof(uint32_t), 0);
                                if(ret1 == -1){
                                    perror("Error: ");
                                    exit(1);
                                }
                            }else if(strstr(buffer, "IN PREPARAZIONE")){
                                printf("INVIO COMANDA IN PREPARAZIONE\n");
                                memset(&response1, 0, sizeof(response1));
                                strcpy(response1, buffer);
                                //si invia la comanda in preparazione al relativo table device
                                for(j=0; j<20; j++){
                                    if(strcmp(sv[j].code, "")!=0 && strstr(response1, sv[j].code)){
                                        ret1 = send(sv[j].sd, (void*)response1, sizeof(response1), 0);
                                        if(ret1==-1){
                                            perror("errore\n");
                                            exit(1);
                                        }
                                        break;
                                    }
                                }
                            
                            }else if(strstr(buffer, "IN SERVIZIO")){
                                printf("INVIO COMANDA IN SERVIZIO\n");
                                memset(&response1, 0, sizeof(response1));
                                strcpy(response1, buffer);
                                //stessa cosa per le comande in servizo
                                for(j=0; j<20; j++){
                                    //printf("sv[%d].sd=%d\n", j, sv[j].sd);
                                    //printf("sv[%d].code %s\n", j, sv[j].code);
                                    if(strcmp(sv[j].code, "")!=0 && strstr(response1, sv[j].code)){
                                        
                                        ret1 = send(sv[j].sd, (void*)response1, sizeof(response1), 0);
                                        if(ret1==-1){
                                            perror("errore\n");
                                            exit(1);
                                        }
                                        break;
                                    }
                                }
                                
                            }
                        }
                    
                }

            }

	    }

    }
    return 0;
}
