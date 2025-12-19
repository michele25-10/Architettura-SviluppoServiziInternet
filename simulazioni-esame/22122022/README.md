# Arch. di Reti / Reti di Calcolatori – 22 dicembre 2022

Si progetti un’applicazione Client/Server che, utilizzando le socket, permetta a un
utente di consultare il listino di giocattoli acquistabili online e scoprire quali di essi
siano ancora disponibili. L’applicazione deve presentare la seguente interfaccia:

> regali_di_natale server porta

dove server rappresenta il nome logico del Server e porta rappresenta il numero di
porta del servizio. Per prima cosa, il Client si deve interfacciare con l’utente, da cui
riceve (via terminale) username, password, e la tipologia di giocattolo di interesse
(es., “costruzioni”, “bambole”, “macchinine”, ecc.). Il Client deve quindi trasmettere le
informazioni al Server, che a sua volta dovrà verificare l'autorizzazione dell’utente
invocando un'apposita funzione, che si suppone già implementata, con il prototipo:

```c
int autorizza(const char *username, const char *password);
```

Se la funzione autorizza restituisce il valore 1, l'utente è autorizzato ad accedere al
servizio e il Server dovrà quindi reperire le informazioni sui giocattoli della tipologia di
interesse che sono ancora disponibili (si noti infatti che in questo periodo dell’anno i
magazzini di giocattoli si svuotano rapidamente ed è quindi possibile che alcuni
particolari giocattoli risultino esauriti), elencarle in ordine di prezzo crescente e
restituirle al Client. Altrimenti, il Server dovrà rifiutarsi di eseguire il servizio.
A questo proposito, si supponga che le informazioni sul listino dei giocattoli siano
salvate sul Server in una serie di file di testo all’interno del percorso /var/local/toys ,
1
ciascuno dei quali conterrà le informazioni per una specifica tipologia di giocattoli.
Quindi, per esempio, le informazioni sui giocattoli di tipo “costruzioni” saranno
salvate nel file /var/local/toys/costruzioni.txt. Ciascuna riga di tali file conterrà tutte le
informazioni relative a uno specifico modello di giocattolo, con (in quest’ordine) il
prezzo, il nome del modello di giocattolo, il nome del produttore, la disponibilità
(“disponibile” o “esaurito”), ecc.
Una volta ricevute le informazioni dal Server, il Client le stampa a video e si mette in
attesa della richiesta successiva. Il Client deve terminare quando l’utente digita
“fine”.

ATTENZIONE: Si realizzino il Client e il Server in C, ma il Client deve essere
realizzato anche in Java.
1 Ovviamente nei PC del laboratorio non avrete permessi di accesso al percorso /var. Ai fini
dell’esame, potete utilizzare un percorso all’interno della vostra home e lasciare un opportuno
commento nella soluzione dell’esercizio (es. ”uso il percorso ./toys al posto di /var/local/toys”).
