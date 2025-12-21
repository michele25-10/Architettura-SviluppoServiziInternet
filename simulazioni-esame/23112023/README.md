# Arch. di Reti / Reti di Calcolatori – 23 novembre 2023

Si progetti un’applicazione Client/Server che, utilizzando le socket, aiuti un
programmatore a verificare lo stato della compilazione del codice del progetto software
a cui egli sta lavorando. Il progetto, infatti, è di dimensione tale da non permetterne la
compilazione sul PC dello sviluppatore e da richiedere invece un apposito Server di
compilazione, che si occupa di compilare il codice del progetto durante le ore notturne e
di salvare i rapporti delle compilazioni.
Il Client deve presentare la seguente interfaccia:

> verifica_compilazione server porta

dove server rappresenta il nome logico del Server e porta rappresenta il numero di
porta del servizio.
Per prima cosa, il Client si deve interfacciare con l’utente, da cui riceve (via terminale) lo
username dell’utente, il nome del progetto software che il nome della versione (es.
“stabile” o “sviluppo”) a cui il programmatore è interessato. Il Client deve quindi
comunicare le informazioni inserite dall’utente al Server, che a sua volta dovrà reperire
le informazioni sui rapporti di compilazione di interesse e restituire le informazioni al
Client.
A questo proposito, si supponga che sul Server i rapporti di compilazione siano salvati
in una serie di file testuali nella cartella /var/local/compilation_reports, che conterrà un
file per ogni progetto software. (Quindi, i dati di compilazione per “mioprogetto” saranno
contenuti nel file /var/local/compilation_reports/mioprogetto.txt.) A sua volta, il file
conterrà una riga per ciascuna compilazione. In ciascuna riga sono contenuti lo
username dello sviluppatore che ha lanciato la compilazione, il nome della versione del
progetto, la data di compilazione, la stringa di risultato (che potrà essere, ad esempio,
“positivo” o “negativo”), ecc.
Una volta ricevute le informazioni dal Server, il Client le stampa a video e si mette in
attesa della richiesta successiva. Il Client deve terminare quando l’utente digita “fine”.

ATTENZIONE: Si realizzino il Client e il Server in C, ma il Client deve essere realizzato
anche in Java.
