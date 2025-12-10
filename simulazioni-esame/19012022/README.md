# Reti di Calcolatori – 19 gennaio 2022

Si progetti un’applicazione Client/Server che, utilizzando le socket, permetta a un
contabile di revisionare le principali spese sostenute dalla propria azienda tra quelle
effettuate in un determinato mese e appartenenti a una categoria di interesse.
L’applicazione deve presentare la seguente interfaccia:

esamina_spese_principali server porta

dove server rappresenta il nome logico del Server e porta rappresenta il numero di
porta del servizio.
Per prima cosa, il Client si deve interfacciare con l’utente, da cui riceve (via
terminale) il mese in formato YYYYMM (es., “202106” per giugno 2021, ecc.), la
categoria di spesa (es., “personale”, “cancelleria”, ecc.), e il numero N di spese di
interesse. Il Client deve quindi trasmettere le informazioni al Server, che a sua volta
dovrà reperire le informazioni sulle N spese più onerose (ovverosia che hanno il
maggiore ammontare corrispondente) di interesse e restituirle al Client.
A questo proposito, si supponga che le informazioni sulle spese sostenute siano
salvate sul Server in una serie di file di testo all’interno del percorso
/var/local/expenses . Più precisamente, le informazioni sono organizzate in cartelle 1
contenenti tutte le informazioni relative a uno specifico mese, a loro volta divise in
file di testo contenenti le informazioni su una specifica categoria di spesa. (Quindi,
per esempio, le informazioni sulle spese di cancelleria sostenute a settembre 2020
saranno salvate nel file /var/local/expenses/202009/cancelleria.txt). Ciascuna riga di
tali file conterrà tutte le informazioni relative a una singola spesa effettuata, con (in
quest’ordine) l’ammontare, la descrizione della spesa, il nome del fornitore, la data
della fattura, ecc.
Una volta ricevute le informazioni dal Server, il Client le stampa a video e si mette in
attesa della richiesta successiva. Il Client deve terminare quando l’utente digita
“fine”.

ATTENZIONE: Si realizzino il Server in linguaggio C e il Client sia in linguaggio C
che in linguaggio Java.
1 Ovviamente nei PC del laboratorio non avrete permessi di accesso al percorso /var. Ai fini
dell’esame, potete utilizzare un percorso all’interno della vostra home e lasciare un opportuno
commento nella soluzione dell’esercizio (es. ”uso il percorso ./expenses al posto di
/var/local/expenses”).
