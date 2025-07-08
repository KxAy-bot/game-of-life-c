# Game of Life in C - Terminal Version
Questo progetto è una semplice implementazione del gioco della vita di Conway, scritto in C e funzionante in terminale.
L'implementazione risulta classica con l'aggiunta del premere `spazio` per fare fast-forward, la possibilità di scegliere seeds tramite menu apposito e una modalità editor per creare configurazioni personalizzate. Verrà generato un file chiamato `seeds.txt` che verrà ripulito ogni 100 seeds.

---

## Come compilare
Per compilare il programma, assicurati di avere `make` installato e poi esegui:
```bash
make
```

## Comandi

### Modalità Simulazione
- **SPACE** - Abilita/disabilita il fast-forward
- **R** - Riavvia il gioco
- **E** - Entra in modalità editor
- **Q** o **CTRL+C** - Esci dal programma

### Modalità Editor
- **Tasto sinistro del mouse** - Seleziona la cella da attivare/disattivare
- **C** - Pulisce la griglia (rimuove tutte le celle)
- **Q** - Salva la configurazione corrente e avvia la simulazione
- **CTRL+C** - Esci dal programma

La modalità editor permette di creare configurazioni personalizzate cliccando sulle celle per attivarle o disattivarle, offrendo un modo intuitivo per sperimentare con diversi pattern iniziali del Game of Life.
