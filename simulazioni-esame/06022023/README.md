# Arch. di Reti / Reti di Calcolatori – 6 febbraio 2023

Si progetti un’applicazione Client/Server che, utilizzando le socket, aiuti un
appassionato di rugby a consultare le statistiche di gioco dei propri beniamini
nell’intera storia del famoso torneo 6 Nazioni. Il Client deve presentare la seguente
interfaccia:

> statistiche_6_nazioni server porta

dove server rappresenta il nome logico del Server e porta rappresenta il numero di
porta del servizio.
Per prima cosa, il Client si deve interfacciare con l’utente, da cui riceve (via
terminale) le stringhe alfanumeriche nome_giocatore, squadra, ed edizione che
rappresentano rispettivamente il nome del giocatore (ad esempio: “Sergio Parisse”,
“Brian O’Driscoll”, “Alun Wyn Jones”, ecc.), la squadra (ad esempio: “Italia”, “Galles“,
ecc.) e l’edizione del torneo (ad esempio: “2019”) di interesse. Il Client dovrà quindi
trasmettere le informazioni al Server, che a sua volta dovrà selezionare le statistiche
corrispondenti e restituirle al Client.
A questo proposito, si supponga che sul Server tutte le statistiche siano salvate nella
directory /var/local/6_nazioni . Tale directory conterrà diversi file, ciascuno dei quali 1
conterrà le statistiche di gioco per una specifica squadra. Quindi, il nome del file
contenente i dati per la squadra italiana nel torneo 6 Nazioni sarà
“/var/local/6_nazioni/Italia.txt”. In ciascun file saranno riportate le statistiche dei vari
giocatori, una per riga. Ciascun giocatore avrà diverse statistiche (es. “numero punti
segnati”, “partite giocate”), ognuna delle quali relativa a una specifica edizione del
torneo. Ogni riga del file summenzionato conterrà quindi il nome di un giocatore (es.
“Sergio Parisse”), il nome di una statistica (es. “numero punti segnati”), l’edizione di
riferimento del torneo (es. “2022”), e il valore misurato per la statistica (es. “5”).
Una volta ricevute le informazioni dal Server, il Client le stampa a video e si mette in
attesa della richiesta successiva. Il Client deve terminare quando l’utente digita
“fine”.

ATTENZIONE: Si realizzino il Client e il Server in C, ma il Client deve essere
realizzato anche in Java.
1 Ovviamente nei PC del laboratorio non avrete permessi di accesso al percorso /var. Ai fini
dell’esame, potete utilizzare un percorso all’interno della vostra home e lasciare un opportuno
commento nella soluzione dell’esercizio (es. ”uso il percorso ./6_nazioni al posto di
/var/local/6_nazioni”)
