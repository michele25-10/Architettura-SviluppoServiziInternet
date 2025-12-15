Reti di Calcolatori – 20 dicembre 2021

Si progetti un’applicazione Client/Server che, utilizzando le socket, permetta a un
utente in ritardo con l’acquisto dei regali di cercare articoli disponibili per l’acquisto
“last minute” tra quelli appartenenti a una tipologia di interesse. L’applicazione deve
presentare la seguente interfaccia:

regali_last_minute server porta

dove server rappresenta il nome logico del Server e porta rappresenta il numero di
porta del servizio.
Per prima cosa, il Client si deve interfacciare con l’utente, da cui riceve (via
terminale) la categoria di regali (es., “giocattoli”, “elettronica”, ecc.), il nome del
produttore (es., “LEGO”, “Apple”, ecc.), e la modalità di ordinamento relativamente al
prezzo dell’articolo (che potrà quindi essere “crescente” o “decrescente”) di
interesse. Il Client deve quindi trasmettere le informazioni al Server, che a sua volta
dovrà reperire le informazioni sugli articoli di interesse e restituirle al Client
nell’ordine desiderato.
A questo proposito, si supponga che le informazioni sugli articoli disponibili per
l’acquisto last minute siano salvate sul Server in una serie di file di testo all’interno
del percorso /var/local/last_minute_gifts , organizzati per categoria di regalo. (Quindi, 1
per esempio, le informazioni sui giocattoli saranno salvate nel file
/var/local/last_minute_gifts/giocattoli.txt.) Ciascuna riga di tali file conterrà tutte le
informazioni relative a un singolo articolo disponibile per l’acquisto last minute, con
(in quest’ordine) il prezzo, il nome del produttore, il nome dell’articolo, il codice
dell’articolo, ecc.
Una volta ricevute le informazioni dal Server, il Client le stampa a video e si mette in
attesa della richiesta successiva. Il Client deve terminare quando l’utente digita
“fine”.

ATTENZIONE: Si realizzino il Server in linguaggio C e il Client sia in linguaggio C
che in linguaggio Java.

1 Ovviamente nei PC del laboratorio non avrete permessi di accesso al percorso /var. Ai fini
dell’esame, potete utilizzare un percorso all’interno della vostra home e lasciare un opportuno
commento nella soluzione dell’esercizio (es. ”uso il percorso ./last_minute_gifts al posto di
/var/local/last_minute_gifts”).
