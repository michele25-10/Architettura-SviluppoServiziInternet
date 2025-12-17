# Architettura e Sviluppo Servizi Internet
## 20 dicembre 2024

Si progetti un'applicazione Client/Server che, utilizzando le socket, consenta a un
operatore di Field Service Management, che quindi effettua interventi di manutenzione
e riparazione di apparecchiature per conto dei propri clienti, di visualizzare le N
operazioni più onerose effettuate per un cliente specifico in una giornata di interesse.
Il Client deve presentare la seguente interfaccia:

> visualizza_interventi_onerosi server porta

dove server rappresenta il nome logico del Server e porta rappresenta il numero di
porta del servizio.
Per prima cosa, il Client si deve interfacciare con l'utente, da cui riceve (via terminale) il
nome del cliente, il numero N e la data di interesse (in formato YYYYMMDD: ad
esempio per fare riferimento alla giornata di oggi l'utente dovrebbe inserire la stringa
"20241220") per il controllo. Una volta ricevute le informazioni il Client deve quindi
comunicare le informazioni inserite dall'utente al Server, che a sua volta dovrà reperire
le informazioni sulle N operazioni più onerose (ovverosia che hanno richiesto il numero
maggiore di ore di lavoro) effettuate per il cliente di interesse nella data di interesse e
restituirle al Client.

A questo proposito, si supponga che sul Server i rapporti di stato siano salvati in una
serie di file testuali nella cartella /var/local/field_service/, organizzati in cartelle relative a
uno specifico cliente, ciascuna delle quali conterrà un file per ogni specifica giornata.
(Quindi, i dati sugli interventi effettuati per il cliente “Coimbra Energy” nella giornata di
oggi saranno contenuti nel file di testo con il percorso /var/local/field_service/Coimbra
Energy/20241220.txt). A loro volta, i file di testo conterranno una riga per ciascun
intervento effettuato. In ciascuna riga sono contenuti la durata in ore dell’intervento,
l’orario di inizio dell'intervento in formato HHMMSS, l’ID dell’asset su cui il tecnico è
intervenuto, il nome del tecnico che ha svolto l’intervento, ecc.
Una volta ricevute le informazioni dal Server, il Client le stampa a video e si mette in
attesa della richiesta successiva. Il Client deve terminare quando l'utente digita “fine”.
ATTENZIONE: Si realizzino il Client e il Server in C, ma il Client deve essere realizzato
anche in Java.