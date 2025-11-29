# Architettura e Sviluppo Servizi Internet 14 luglio 2025

Si progetti un’applicazione Client/Server che, utilizzando le socket, consenta a un turista di consultare i pacchetti vacanza più apprezzati tra quelli disponibili in una nazione di interesse.
L’applicazione deve presentare la seguente interfaccia:

`consulta_pacchetti_vacanza server porta`

dove server rappresenta il nome logico del Server e porta rappresenta il numero di porta del servizio.
Per prima cosa, il Client si deve interfacciare con l’utente, da cui riceve (via terminale) la nazione (es., “Italia”, ecc.), la tipologia di vacanza (es., “romantica”, “famiglie”, “sportiva”, “enogastronomica”, ecc.), e il livello di budget disponibile di interesse (es., “basso”, “medio”, “alto”, ecc.).
Il Client deve quindi trasmettere le informazioni al Server, che a sua volta dovrà reperire le informazioni sui pacchetti vacanze disponibili corrispondenti agli interessi dell’utente e restituirli al Client in ordine decrescente rispetto alla media di recensioni ottenute dai turisti che hanno acquistato il pacchetto vacanze in precedenza.
A questo proposito, si supponga che le informazioni sui pacchetti vacanze disponibili siano salvate sul Server in una serie di file di testo nel percorso `/var/local/holiday_packages`, ciascuno dei quali conterrà le informazioni per i pacchetti vacanza di una specifica tipologia disponibili in una specifica nazione. Quindi, per esempio, tutte le informazioni sui pacchetti
vacanza disponibili per una vacanza romantica in Italia saranno salvate all’interno del file /var/local/holiday_packages/Italia/romantica.txt, ecc.
Ciascuna riga di tali file conterrà tutte le informazioni relative a uno specifico pacchetto vacanza, con (in quest’ordine) la media di recensioni ottenute dai turisti che hanno acquistato il pacchetto vacanza in precedenza, la località, il nome dell’operatore turistico che offre il pacchetto vacanza, il livello di budget di riferimento per il pacchetto vacanze, ecc.
Una volta ricevute le informazioni dal Server, il Client le stampa a video e si mette in attesa della richiesta successiva. Il Client deve terminare quando l’utente digita “fine”.

> ATTENZIONE: Si realizzino il Client e il Server in C, ma il Client deve essere realizzato anche in Java.

Ovviamente nei PC del laboratorio non avrete permessi di accesso al percorso /var.
Ai fini dell’esame, potete utilizzare un percorso all’interno della vostra home e lasciare un opportuno commento nella soluzione dell’esercizio (es. ”uso il percorso ./holiday_packages al posto di /var/local/holiday_packages”).
