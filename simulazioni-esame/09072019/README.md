# Reti di Calcolatori – 9 luglio 2019

Si progetti un’applicazione Client/Server che, utilizzando le socket, permetta a un
appassionato bevitore di caffé di consultare le informazioni sulle macchine da caffé presenti
sul mercato. L’applicazione deve presentare la seguente interfaccia:

> coffee_machines server porta

dove server rappresenta il nome logico del Server e porta rappresenta il numero di porta
del servizio. Per prima cosa, il Client si deve interfacciare con l’utente, da cui riceve (via
terminale) username, password, e la categoria di macchine di interesse (es.,
“superautomatiche”, “capsule”, “cialde”, “pour over”, “cold brew”, ecc.). Il Client deve quindi
trasmettere le informazioni al Server, che a sua volta dovrà verificare l'autorizzazione
dell’utente invocando un'apposita funzione, che si suppone essere già implementata con il
seguente prototipo:

```c
int autorizza(const char *username, const char *password);
```

Se la funzione autorizza restituisce il valore 1, l'utente è autorizzato ad accedere al
servizio e il Server dovrà quindi: analizzare tutte le informazioni sulle macchine da caffé di
interesse all’interno del proprio database; selezionare tra queste solo le informazioni relative
alla media di recensione, al nome del modello e al prezzo; elencare le informazioni in ordine
di media di recensione decrescente (ovverosia dalla media più elevata a quella più bassa);
considerare solo le prime 10 (diconsi dieci) macchine in elenco; e infine restituire il risultato
al Client. Nel caso l’utente non fosse autorizzato, il Server dovrà rifiutarsi di eseguire il
servizio e inviare un messaggio di errore al Client.

A questo proposito, si supponga che le informazioni sulle macchine per caffé disponibili in
commercio siano salvate sul Server in una serie di file di testo all’interno del percorso
/var/local/macchine_caffé/, ciascuno dei quali conterrà le informazioni per una specifica
tipologia di macchine. (Quindi, per esempio, le informazioni sulle macchine
superautomatiche saranno salvate nel file “/var/local/macchine_caffé/superautomatiche.txt”,
ecc.) Ciascuna riga di tale file conterrà tutte le informazioni relative a una singola macchina,
con (in quest’ordine) media di recensione (che si suppone essere un numero intero da 0 a
100), nome del produttore, nome del modello, prezzo, ecc.
Una volta ricevute le informazioni dal Server, il Client le stampa a video e si mette in attesa
della richiesta successiva. Il Client deve terminare quando l’utente digita “fine”.
ATTENZIONE: Si realizzino il Client e il Server in C, ma il Client deve essere realizzato
anche in Java.
