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

#define MESSAGE_LEN 20

#define BUFFER_SIZE 1024
#define CHECK_LEN 5
#define CHECK1_LEN 6
#define RESPONSE_LEN 1024 //risposta inviata dal server
void stampa(char *);
void mostra_preparazioni(char *);
char* aggiorna(char *, char *);
//stampa i piatti della comanda in un certo formato
void stampa(char *ordini){

    int len=strlen(ordini);
    
    int i,b; char a;
    b=1;    //serve perchè dopo due caratteri c'è il - che non si riesce a confrontare
            //con un if, se b=3 allora c'è il trattino e si stampa lo spazio
    for(i=0; i<len; i++){
        a=ordini[i];

       if(a == ' '){
        printf("\n");
        b=0;
       }else if( b == 3)
            printf(" ");
        else
            printf("%c", ordini[i]);
        b++;
    }
    printf("\n");
}
//stampa i piatti dal vettore delle preparazioni nel formato 
void mostra_preparazioni(char *preparazioni){

    int len;
    char buf[BUFFER_SIZE];
    char code[3];
    char tavolo[100];           //della stessa dimensione di str, altrimenti stampa anche altri caratteri
    int i=0,j; char a;
    FILE* fd;
    char str[100];              //per la lettura nel file
    char ordini[100];           //contiene i piatti dell'ordine
    //si copia temporaneamente in buf il vettore delle comande gestire dal device
    strncpy(buf, preparazioni, BUFFER_SIZE);
    len=strlen(buf);
    //se la lunghezza è maggiore, ci sono comande accettate dal device, altrimenti non ci sono
    //e non si fa niente
    if(len>0){
        while(1){
            //si cerca la posizione del primo slash, la i indica la poszione di ogni slash nel
            //vettore delle preparazioni
            for(j=0;j<len;j++){
                a=buf[j];
                i++;
                if(a=='/'){
                    break;
                }
            }
            
            //si prende il codice relativo alla comanda
            strncpy(code, buf+(j-5), 3);
            //si prendono solo gli ordini
            strncpy(ordini, buf, j-6);
            //si cerca il codice nelle prenotazioni
            fd = fopen("prenotazioni", "r");
            if (NULL == fd) {

                printf("errore apertura file\n");
            }
                        
            while(fgets(str, 100, fd)!= NULL){
                //si cerca il tavolo corrispondente
                if(strstr(str, code)){
                    strncpy(tavolo, str+(strlen(str)-8), 3);
                        break;
                }                     
            }
            fclose(fd);
            printf("com%c", buf[j-1]);
            //se il tavolo è 10, allora aggiungo uno spazio, sennò si lascia così
            if(strcmp(tavolo, "T10")==0)
                printf(" %s\n", tavolo);
            else
                printf("%s\n", tavolo);
            //si stampano i piatti della comanda selezionata
            stampa(ordini);
            //la unghezza si decrementa del numero di caratteri rimossi da buf
            len-=j;
            //rimuoviamo da buf la stringa relativa alla comanda già visualizzata
            strncpy(buf, preparazioni+i, len);
            memset(&ordini, 0 ,sizeof(ordini));
            memset(&tavolo, 0, sizeof(tavolo));
             memset(&code, 0 ,sizeof(code));
            //se arrivo all'ultimo slash esco dal ciclo
            if(i==strlen(preparazioni))
                break;
        }
    }else{
        printf("non ci sono comande in preparazione attualmente nel kitchen device\n");
    }
     memset(&buf, 0 ,sizeof(buf));
}
//aggiorna il vettore delle preparazioni
char* aggiorna(char *support, char* preparazione){

    int len1 = strlen(preparazione);
    //si assegna la stringa fino allo slash
    //si confronta la stringa assegnata con support, se sono diverse, si svuota assign e si
    //riassegna fino al prossimo slash, se sono uguali si copia penotazioni dopo il secondo slash
    //nella parte prima dello slash
    int slash1=0;   //slash finale della comanad 
    int slash2=0;   //slash precedente a slash1
    int i=0, j=0, a=0;
    int cont=0;     //conta il numero di comande accettate dal device
    char assign[100];
    static char temp[1024]; //contiene il risultato da restituire, lo stato aggiornato
    
    for(i=0; i<len1; i++){
        //si conta il numero di slash nella stringa, quindi il numero di comande nella stringa
        if(preparazione[i]=='/'){
            cont++;
        }
    }
    i=0;    //si rimette i a 0

    for(j=0; j<cont;j++){
        a=0;    //assign deve ripartire dall'inizio ogni volta
        while(i<len1){
            //si prende il primo slash
            if(preparazione[i]=='/'){
                slash1=i;
                i++;        //va aggiornato qua e non fuori, altrimenti con break rimane alla stessa istruzione
                break;
            }else{
                i++;
                a++;
            }
            
            //si prende lo slash precedente
            slash2=slash1;
        }
        memset(&assign, 0, sizeof(assign));
        memset(&temp, 0, sizeof(temp));

        //bisogna prendere il giusto numero di caratteri
        if(slash2==0){
            //se ho solo una comanda
            strncpy(assign, preparazione+slash2, slash1-slash2);
        }else{
            strncpy(assign, preparazione+slash2+1, slash1-slash2-1);
        }
    
        if(strcmp(assign, support)==0){
            //se è la prima comanda si copia solo la parte dopo il primo slash
            if(j==0){
                strncpy(temp, preparazione+slash1+1, len1-slash1);
            }else if(j==(cont-1)){
                //se è l'ultima comanda si assegna tutta la stringa, eccetto l'ultima
                strncpy(temp, preparazione, len1-(slash1-slash2));
            }else{
                //se la coamnda è intermedia, prima si assegna a temp, la parte prima della stringa
                strncpy(temp, preparazione, slash2+1);
                //e dopo si concatena la parte dopo della stringa
                strcat(temp, preparazione+slash1+1);
            }
        
            break;
        }
        
    }

    
    memset(&assign, 0, sizeof(assign));
    memset(&support, 0, sizeof(support));
    
    return temp;

}
int main(int argc, char* argv[]){

    fd_set master;
    fd_set read_fds;
	int fdmax;
//ret è per la connect, sd per la socket, len e ret1 per la recv
    int ret, sd, len; 
    int data1; //data1 serve per il controllo del pattern
    regex_t r1; //per il controllo del pattern

    struct sockaddr_in server_addr;
    int i, new_sd=0;

    char buffer[BUFFER_SIZE];
    char invio[BUFFER_SIZE];    //ciò che invio al server
    char ricevo[BUFFER_SIZE];   //ciò che ricevo dal server
    char check[CHECK_LEN]; //primi 4 caratteri della stringa per il controllo
    char check1[CHECK1_LEN]; //primi 6 caratteri della stringa per il controllo di ready
    FILE* fd;
    FILE* temp;
    FILE* fd1;
    char preparazione[BUFFER_SIZE];     //contiene le comande in preparazione
    char str[100];                      //legge il contenuto del file, ogni riga
    char str1[100];                     //legge il contenuto del file (lettura annidata)
    char support[100];                  //supporto per aggiungere a preparazione
    int cont=0;                         //conta il numero di linee nel file
    int line=0;                         //linea da sostituire
    char code[5];                       //mantiene il codice corrente
    char tavolo[100];                   //contiene il tavolo, così grande altrimenti stampa anche altri caratteri
    char ordini[100];                   //contiene i piatti ordinati
    char number[3];                     //contiene il numero di comanda
    char number_support[3];             //numero di comanda gestita dal kitchen
    char ten;                           //controllo se il tavolo è T10
    char *punt;                         //riceve l'aggiornamento dello stato delle comande
    char mia_comanda[6];                //comande gestite dal kitchen
    int device=3;                       //identifica il client
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
    server_addr.sin_port = htons(4242); //scelta a caso
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    
    ret = connect(sd, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if(ret == -1){
        
        perror("Error: ");
        exit(1);
    }
    //si invia il tipo di client al server
    device=3;
    device1=htonl(device);
    ret = send(sd, (void*)&device1, sizeof(uint32_t), 0);

    if(ret == -1){
        perror("Error: ");
        exit(1);
    }
    
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(sd, &master);
    FD_SET(0, &master);
    fdmax = sd;
    memset(&preparazione, 0, sizeof(preparazione));
    memset(&code, 0, sizeof(code));
    
    printf("\n");
    printf("Digita un comando:\n\n");
    printf("take --> accetta una comanda\n");
    printf("show --> mostra le comande accettate (in preparazione)\n");
    printf("ready  --> imposta lo stato della comanda ad in servizio\n\n");
                
    printf("Inserire comando: \n");
    for(;;){
        read_fds=master;
        ret=select(fdmax+1, &read_fds, NULL, NULL, NULL);
        for(i=0; i <=fdmax; i++){
            if(FD_ISSET(i, &read_fds)){
                if(i==sd){
                    printf("Invio dal server rilevato!\n");
                    //ricevo le comande disponibili dal server
                    memset(&ricevo, 0, sizeof(ricevo));
                    FD_SET(new_sd, &master);
                    if(new_sd > fdmax){fdmax = new_sd;}
                    
                    ret=recv(i, (void*)ricevo, sizeof(ricevo),0);
                    if(ret == -1){
                        perror("Error: ");
                        exit(1);
                    }
                    
                    printf("comande=%s\n", ricevo);
                    //se ricevo quit, esco
                    if(strcmp(ricevo, "quit")==0){
                        fflush(stdout);
                        close(i);
                        FD_CLR(i, &master);
                        exit(0);
                    }
                    memset(&ricevo, 0, sizeof(ricevo));
                    printf("inserisci un comando\n");
                    
                }else if(i==0){
                    //stdin
                    fgets(buffer, 1024, stdin);
                    len = strlen((void*)buffer);
                    
                    strncpy(check, buffer, 5);
                    strncpy(check1, buffer, 6);
                        
                    if(strcmp(check, "take\n")==0){
                                
                        printf("il comando è take\n\n");
                        //nel file delle comande mettiamo la comanda in preparazione
                        //si apre in lettura e scrittura
                        fd = fopen("comande", "r");
                        temp = fopen("replace.tmp", "w");
                                        
                        if (NULL == fd || NULL == temp) {

                            printf("il file non può essere aperto\n");
                        }
                        //si cerca nel file delle comande, se ci sono comande in attesa
                        cont =0;
                        line=0;
                        while(fgets(str, 100, fd)!= NULL){
                            //se la comanda è in attesa si imposta in preparazione
                            //si legge il valore della comanda
                            if(strstr(str, "IN ATTESA")){
                                //si copia omettendo IN ATTESA
                                line++;
                                if(line ==1){
                                    cont=1;
                                    memset(&support, 0, sizeof(support));
                                    memset(&ordini, 0, sizeof(ordini));
                                    strncpy(support, str, strlen(str)-11);
                                    strncpy(ordini, support, strlen(support)-6);
                                    strcat(preparazione, support);
                                    //si aggiunge un separartore tra le comande
                                    strcat(preparazione, "/");
                                    //si scrive IN PREPARAZIONE anzichè IN ATTESA
                                    //si utilizza support
                                    strcat(support, " IN PREPARAZIONE\n"); 
                                }     
                            }
                            //se line vale 1, ho trovato la prima in attesa
                            //altrimenti non è in attesa o sono in attesa ma sono nuove
                            //serve ricontrollare se la stringa è IN ATTESA
                            //altrimenti sostituisce anche righe IN SERVIZIO dato che line
                            //rimane 1 in questo caso
                            if ((line==1) && (strstr(str, "IN ATTESA")))
                                fputs(support, temp);
                            else
                                fputs(str, temp);   
                            }
                            fclose(fd);
                            fclose(temp);
                            remove("comande");
                            rename("replace.tmp", "comande");
                            printf("stato=%s\n", preparazione);
    
                            //prendiamo il codice di prenotazione per ricavare il tavolo
                            strncpy(code, preparazione+(strlen(preparazione)-6), 3);
                            //si cerca il codice nelle prenotazioni
                            fd = fopen("prenotazioni", "r");
                            if (NULL == fd) {

                                printf("il file non può essere aperto\n");
                            }
                            memset(&str, 0, sizeof(str));
                            while(fgets(str, 100, fd)!= NULL){
                                //si cerca il tavolo corrispondente
                                if(strstr(str, code)){
                                    memset(&tavolo, 0, sizeof(tavolo));
                                    strncpy(tavolo, str+(strlen(str)-8), 3);
                                    break;
                                }
                            }
                            fclose(fd);
                            printf("com%c", preparazione[strlen(preparazione)-2]);
                            //se il tavolo è 10, allora aggiungo uno spazio, sennò si lascia così
                            if(strcmp(tavolo, "T10")==0)
                                printf(" %s\n", tavolo);
                            else
                                printf("%s\n", tavolo);
                            //si stampano i piatti della comanda selezionata
                            stampa(ordini);
                        
                            if(cont==0){
                                printf("non ci sono comande in attesa\n");
                            }else{
                                memset(invio, 0, sizeof(invio));
                                strncpy(invio, support, strlen(support));
                                //si invia la comanda al server che la girerà al table device corrispondente
                                ret=send(sd, (void*)invio, strlen(invio), 0);
                                if(ret == -1){
                                    perror("Error: ");
                                    exit(1);
                                }
                           
                            }
                            memset(&support, 0, sizeof(support));  
                            memset(&ordini, 0, sizeof(ordini));
                            memset(&tavolo, 0, sizeof(tavolo));
                            memset(&code, 0, sizeof(code));  
                            
                        }else if(strcmp(check, "show\n")==0){
                
                            printf("il comando è show\n\n");
                            //mostra le comande accettate dal kitchen device
                            mostra_preparazioni(preparazione);

                        //se la stringa è più lunga di 16, non è valida, al massimo è lunga 16, al minimo 14
                        }else if(strcmp(check1, "ready ")==0 && (len == 14 || len == 15 || len == 16)){

                            printf("il comando è ready\n");
                            //i tavoli arrivano fino al 10
                            //per ogni tavolo abbiamo 9 comande massimo
                            data1 = regcomp( &r1, "ready com[1-9][-][T]([1-9]|10)", REG_EXTENDED);
                            data1 = regexec( &r1, buffer, 0, NULL, 0);

                            if(data1 == 0){
                                    printf("ready valido\n");
                                    //se il tavolo non è il 10, prendo 2 caratteri, altrimenti 3
                                    memset(&tavolo, 0, sizeof(tavolo));
                                    if(len==14){
                                        strncpy(tavolo, buffer+11, 2);
                                    }else if(len == 15){
                                        strncpy(tavolo, buffer+11, 3);
                                    }
                                    memset(&number, 0, sizeof(number));
                                    memset(&support, 0, sizeof(support));
                                    memset(&code, 0, sizeof(code)); 
                                    //si prende il numero di comanda è nella posizione 9 del buffer
                                    
                                    strncpy(number, " ", 1);
                                    number[1]=buffer[9];
                                    strcat(number, " ");
                                    
                                    fd = fopen("prenotazioni", "r");
                                                                
                                    if (NULL == fd) {
                                        printf("il file non può essere aperto\n");
                                    }
                                    //si cerca nel file delle comande, se ci sono comande in attesa
                                    cont =0;  
                                    line=0;
                                    while(fgets(str, 100, fd)!= NULL){
                                        //si prende il carattere dopo il primo numero del tavolo
                                        ten=str[strlen(str)-6];
                                        if(strstr(str, tavolo)){
                                            //se il tavolo è T1 e nel file c'è T10, se non faccio il controllo
                                            //strstr(str, tavolo) è true
                                            if(strcmp(tavolo, "T1")==0 && (ten=='0')){
                                            
                                            }else{
                                                //si prende il codice
                                                strncpy(code, str+strlen(str)-4, 3);
                                                
                                                //si cerca se il codice e il numero di comanda sono effettivamente
                                                //di una comanda gestita dal kitchen device
                                                memset(&mia_comanda, 0, sizeof(mia_comanda));
                                                memset(&number_support, 0, sizeof(number_support));
                                                strncpy(mia_comanda, code, 3);
                                                
                                                number_support[0]=number[0];
                                                number_support[1]=number[1];
                                                strcat(mia_comanda, number_support);
                                                strcat(mia_comanda, "/");
                                                
                                                //se la comanda è in preparazione, ma non è quella
                                                //del device si esce e si segnala
                                                if(!strstr(preparazione, mia_comanda)){
                                                    cont=0;
                                                    break;
                                                }

                                                if (NULL == fd) {
                                                    printf("il file non può essere aperto\n");
                                                }
                                                fd1 = fopen("comande", "r");
                                                //se la comanda è relativa al codice ed è accettata dal kitchen
                                                //si prende la linea da aggiornare
                                                while(fgets(str1, 100, fd1)!= NULL){
                                                    cont++;
                                                    if(strstr(str1, code)){
                                                        if(strstr(str1, number)){
                                                            if(strstr(str1, "IN PREPARAZIONE")){
                                                                line=cont;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                                fclose(fd1);
                                                if(line!=0)
                                                    break;
                                                
                                            }
                                            cont =0;
                                        }
                                        
                                    }
                                    fclose(fd);
                                    if(cont == 0){
                                        printf("la comanda selezionata non era in preparazione\n");
                                    }else{
                                        fd = fopen("comande", "r");
                                        temp = fopen("replace.tmp", "w");
                                        
                                        if (NULL == fd || NULL == temp) {

                                            printf("errore apertura file\n");
                                        }
                                        //si inserisce nel file delle comande quella in servizio
                                        cont =0;
                                        
                                        while(fgets(str, 100, fd)!= NULL){
                                            
                                            cont++;
                                            if(cont == line){
                                            //si copia omettendo IN PREPARAZIONE
                                            
                                                strncpy(support, str, strlen(str)-17);
                                                //quando la comanda va in servizio va tolta dal vettore delle preparazioni
                                                punt = aggiorna(support, preparazione);
                                                memset(&preparazione, 0, sizeof(preparazione));
                                                strncpy(preparazione, punt, strlen(punt));
                                                printf("stato aggiornato:%s\n", preparazione);
                                                strcat(support, " IN SERVIZIO\n");
                                                fputs(support, temp);   
                                            }else{
                                                fputs(str, temp); 
                                            }  
                                        }
                                        fclose(fd);
                                        fclose(temp);
                                        //si rimuove il file delle comande vecchio
                                        remove("comande");
                                        //si cambia nome a temp in comande
                                        rename("replace.tmp", "comande"); 
                                        memset(invio, 0, sizeof(invio));
                                        strncpy(invio, support, strlen(support));
                                        ret=send(sd, (void*)invio, strlen(invio), 0);
                                        if(ret==-1)
                                        {
                                            perror("errore\n");
                                            exit(1);
                                        }
                                        printf("COMANDA IN SERVIZIO\n");
                                    
                                    }

                                }else{
                                    printf("ready non valida\n");
                                }
                        
                                memset(&buffer, 0, sizeof(buffer));
                                
                            }else{

                                printf("comando non valido\n");
                            }
                            printf("\n");
                            printf("Digita un comando:\n\n");
                            printf("take --> accetta una comanda\n");
                            printf("show --> mostra le comande accettate (in preparazione)\n");
                            printf("ready  --> imposta lo stato della comanda ad in servizio\n\n");
                
                            printf("INserire stringa: \n");
                }else{
                   
                    if(ret==0){
                        fflush(stdout);
                        close(i);
                        // Tolgo il descrittore del socket connesso dal
                        // set dei monitorati
                        FD_CLR(i, &master);
                    }else if(ret <0){
                        fflush(stdout);
                        close(i);
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