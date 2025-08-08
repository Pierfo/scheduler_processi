Il programma avvia l'esecuzione di tre processi paralleli, i quali operano tutti sullo stesso buffer. Tali processi sono:
- `insert_into_buffer`, che inserisce dati nel buffer
- `remove_from_buffer`, che rimuove dati dal buffer
- `monitor_buffer_level`, che monitora la percentuale di riempimento del buffer e, in caso questa scenda sotto il 25%, aumenta la priorità del primo processo in modo da riportarla a un valore accettabile.

## Compilazione ed esecuzione

Per compilare il programma è sufficiente compilare ed eseguire lo script `compile_all.cpp`; il file eseguibile sarà generato nella cartella `build_main`. 
L'esecuzione del programma avviene con il comando 
```shell
sudo ./main sec nsec
```
dove `sec` e `nsec` indicano rispettivamente per quanti secondi e nanosecondi il programma deve eseguire (ad esempio `sudo ./main 20 500000000` indica che il programma deve eseguire per `20` secondi e `500000000` nanosecondi). Il programma dev'essere eseguito in modalità `sudo`, altrimenti non avrebbe l'autorizzazione ad alterare le priorità dei vari processi.

## Scheduling dei processi

Com'è stato detto all'inizio, le priorità dei processi `insert_into_buffer` e `remove_from_buffer` sono alterate dinamicamente a seconda della percentuale di riempimento del buffer: in "condizione di equilibrio" entrambi i processi sono schedulati con politica `SCHED_OTHER` e ad ambedue è attributo un `nice value` pari a `0`; nel momento in cui la percentuale scende al di sotto del 25% si fa passare il processo `insert_into_buffer` alla politica `SCHED_RR` con priorità statica definita dalla funzione
```c
max(floor((25 - percentage) * (MAXIMUM_PRIORITY / 25.0)), prio)
```
dove `percentage` è la percentuale, `MAXIMUM_PRIORITY` è il massimo valore di priorità statica concesso e `prio` è il valore attuale di priorità statica. Al contempo viene anche aumentato il `nice value` del processo `remove_from_buffer` al valore 
```c
max(floor(prio * 19 / MAXIMUM_PRIORITY), nice)
```
dove `nice` è il nice value attuale.
Una volta che la percentuale di riempimento è tornata a un valore accettabile, allora si decrementano progressivamente la priorità statica e il nice value rispettivamente di `insert_into_buffer` e `remove_from_buffer`, fino a ritornare alla condizione di equilibrio.
Per quanto riguarda i processi `main` e `monitor_buffer_level`, a essi è attribuita priorità massima e politica di scheduling `SCHED_FIFO`, così da assicurarsi che la loro esecuzione non sia ostacolata dagli altri due procesi.

## buffer.h

Il file `include/buffer.h` definisce la classe `buffer`, la quale implementa il buffer condiviso. Tale classe è definita sotto un template in cui il primo parametro è il tipo di dato che può essere inserito mentre il secondo è la dimensione, pertanto se si vuole implementare un buffer di interi con 1000 posizioni è necessario usare la sintassi 
```c
buffer<int, 1000>
```
La classe `buffer` garantisce che solo un processo per volta possa accedere ai dati al suo interno, inoltre, essa è in grado di
- sospendere l'esecuzione di `insert_into_buffer`, nel caso in cui il buffer sia pieno
- sospendere l'esecuzione di `remove_from_buffer`, nel caso in cui il buffer sia vuoto
- sospendere l'esecuzione di `monitor_buffer_level`, nel caso in cui non è stata effettuato alcun inserimento/rimozione dall'ultima volta che è stata letta la percentuale

## Personalizzazione

All'utente è fornita la possibilità di personalizzare alcuni aspetti del funzionamento del programma, in particolare, è possibile
- modificare le dimensioni del buffer e/o il tipo di oggetti che può contenere
- definire una propria funzione da eseguire per generare il prossimo elemento da inserire nel buffer
- definire una propria funzione da eseguire dopo aver estratto un elemento dal buffer

Per definire le due funzioni è necessario construire due function object distinti in cui si effettua overriding dell'operatore `()`, si devono inoltre seguire i seguenti vincoli:
- la funzione che genera gli elementi non deve richiedere alcun argomento e deve restituire un oggetto tale da poter essere inserito nel buffer
- la funzione da eseguire dopo aver estratto un elemento non deve restituire alcun oggetto e deve richiedere, come argomento, un oggetto di tipo compatibile con quelli contenuti nel buffer

Per applicare la propria implementazione è necessario modificare lo `struct shared_memory_object` definito in `include/shared_memory_object.h`, secondo le seguenti regole:
- per modificare le dimensioni del buffer e/o il tipo di oggetti che può contenere basta modificare i parametri template del campo `shared_buffer`
- per definire la funzione da eseguire per generare gli elementi basta modificare il tipo del campo `action_before_insertion`
- per definire la funzione da eseguire dopo aver estratto un elemento basta modificare il tipo del campo `action_after_extraction`
  
In questa versione del programma il buffer ha dimensione `1001` e contiene istanze di `matrix<N>` (classe che implementa matrici NxN), la funzione che genera gli elementi da inserire costruisce una matrice NxN e restituisce la sua inversa, mentre la funzione da eseguire dopo un'estrazione inverte la matrice passata per argomento. I due function object sono definiti in `include/actions.h`.
