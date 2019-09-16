/*
 ########################################################
 ##           __  __       _____   _____ _             ##
 ##     /\   |  \/  |     |  __ \ / ____(_)            ##
 ##    /  \  | \  / | ___ | |  | | (___  _ _ __ ___    ##
 ##   / /\ \ | |\/| |/ _ \| |  | |\___ \| | '_ ` _ \   ##
 ##  / ____ \| |  | | (_) | |__| |____) | | | | | | |  ##
 ## /_/    \_\_|  |_|\___/|_____/|_____/|_|_| |_| |_|  ##
 ##                                                    ##
 ## Author:                                            ##
 ##    Andrea Di Maria                                 ##
 ##    <andrea.dimaria90@gmail.com>                    ##
 ## Extended by:                                       ##
 ##    Francesco Furnari                               ##
 ##    <franz.furnari@gmail.com>                       ##
 ##    Fabrizio Tornatore                              ##
 ##    <torfab94@gmail.com>                            ##
 ##                                                    ##                    
 ########################################################
 */

# TODONEXTTIME:

10. autobus: punti di raccolta taxi ride sharing
	gli autobus girano per tutti i nodi della redzone
	i nodi hanno un certo numero di persone raccolte da salvare
	gli autobus raccolgono le persone e le scaricano al punto di raccolta
	riprendono il circuito e si fermano quando non trovano più nessuno


pulizia initialize app (e netmanager)
	codice obsoleto
	togliere le sue statistiche che occupano spazio
truckdelaytraveltime è rotto

capire come scrivere la legenda pulita

scrivere la tesi


# TITOLI:
algoritmi bioispirati coordinamento ambulanze
l'altro, gestione dell emergency managment, simmulazione dinamiche produzione informazioni, concetto mezzo pesante,
strategie per la simulazione delle condizioni a contorno di un evento catastrofico

* coordinamento di ambulanze attraverso strategie bioispirate in uno scenario di disastro naturale


* simulazioni delle condizioni a contorno di un evento catastrofico, approccio bioispirato




# TODO Importanti:

CAVALLO DI BATTAGLIA
Usare due topo per implementare dijkstra pesato traffico per ambulanze e pesato feromone per i civili



# TODO mildly important:

I routing devono supportare il segnale di decadenza del feromone di basecoord
(manca dijkstraTraffic)


--Check che tutto sia in inglese (E' già in inglese a meno dei log)



# TODO Non importanti:


# DONE:

-- vettore di feromoni invece che hardcoded 4 feromoni. - in manhattanrouting.cc e ned
-- cambiare indice porte per adattare il suo al nostro o vv.  -manhattanrouting.cc (e pheromone) -- o il suo amod.ned

-- fattore feromone -- DONE
-- decadenza feromone -- DONE (in manhattanrouting.cc)

-risultati
	Abbiamo i risultati dei feromoni nei quattro canali di ogni nodo
	
traffico
	c'è la classe traffico ma non è ancora utilizzata
	-> workaround automessaggio
	(parametrizzare aumento feromone e traffico) done partially
		fix traffic influence
	- emit traffico

-creazione traffico civile (non taxi - indipendente dal coordinatore)
	se arriva a dest lo deleta -> e ne manda un altro

--~~BUG~~ FIXED: manhattanrouting.cc - trafficDelay: abbiamo aggiunto un epsilon pari a +0.00001 perchè c'è un bug nel simulatore (coi double) e finisce con lo schedulare il veicolo nel passato (confrontando i numeri c'è tipo 0.000000001 di differenza)

~~-- cambiare selfmessage con sendDelayed(pk,sendDelayTime,"out");  (NON è POSSIBILE FARLO a causa dell'aggiornamento del traffico)~~

-- funzione di decay feromone a metodo? manhattanrouting (riga 121 - 132)

1. feromoni di tipo diverso
2. velocità nei veicoli come attributo del veicolo
3. crezione mezzo pesante (fattore traffico  valore 20 veicoli normali)

check su routing
evitare che i nodi morti pubblichino richieste
parametrizzare raggio di esplosione e rendere possibile la non esplosione

4. heatmap

1. ottimizzazione bomba

6. mezzo pesante circuito in zone di ammassamento = implementare una truck request
7. traffico (civile) che scappa
9. le emergenze dovrebbero essere nella zona rossa

epicentri multipli random
numero di ospedali dinamico
--il primo ospedale è sicuramente buono
--gli altri ospedali potrebbero essere distrutti

emergency request valuta l'ospedale più vicino per partire
12. meta di chi scappa invece di andare a bordo va all ammassamento piu vicino (facciamo 50% con rand

la truck request è diretta verso un collectionpointAddress random

routing semplice con circumnavigazione DONE CON DIJKSTRA

emit e segnali
	differenza normalizzata tra tempo stimato nel caso migliore contro caso reale ambulanza
	grafico ambulanze idle
	civili in quanto scappano
	feromone
	traffico
	grafico tempi di risposta con tempi assoluti e non differenza
	tempo di attesa da richiesta a pickup

11. civili che scappano in maniera realistica: burst subito e poi sempre meno

-creato feromone secondo tipo ma non rilasciato

differenziare ammassamento e raccolta

distanza 200m


se 3 hop non prendo la macchina
dislocazione zone di raccolta




densità = 200 civili per nodo
veicoli civili che scappano = 50% densità
chiamate d'emergenza totali = 50% densità
veicoli civili che scappano + chiamate d'emergenza totali = 100%


triprequest: "scheduleat" "selfmessage" "build emergency"

burst entro 2 minuti chiamate  (uniform 0,120)
burst ridotto entro 10 min      (uniform 120 600)
burst/2 nella restante mezzora	(uniform 600 1800)

burst1 50% +burst2 33% +burst3 17% = numero fisso di chiamate emergenziali per nodo


risultati
	* Differenza tempo minimo di esecuzione (in assenza di traffico) con tempo reale di esecuzione
	 
algoritmi
	* Custom nostro: manhattan + circumnaviga
	* ACO (network manager)
	* AAA (network manager)

* cambiare colore ospedali - > bianco
TruckStartNodes -> sono gli storagepoint  soluzione:vettore


codici di emergenza differenti
	* crea richiesta codice rosso
	* l'ambulanza piu vicina mette il codice rosso in cima alla lista stop points
	* se l'ambulanza ha un codice rosso si controllano le altre ambulanze
	* se tutte le ambulanze stanno gestendo un codice rosso lo mettono in coda (secondo terzo etc)

emit specifico per i rossi (quanto e quando vengono salvati)


Polish:
1. App.cc
2. ManhattanRouting
3. StopPoint
4. Pheromone
5. Traffic
6. TripRequestSubmitter
7. HeuristicCoord

 
Algoritmi:
	* ManhattanRouting Dijkstra non pesato [x]
	* Dijkstra pesato su traffico [x]  (aaa per tutti)
	* Dijkstra pesato su feromone [x]  (aaa per tutti)
	* Dijkstra pesato su feromone per civili e non pesato per le ambulanze [x]

Python comparazione curve diverse simulazioni NO MORE NEEDED

sistemare setadditionaltraveltime in netmanager (accelerazione e decelerazione)

display look and feel figo TOP

creare sottomodulo initialize
creare routing aco based di giovanni


# Limiti del simulatore:

1. Non si possono inserire vettori nei parametri del ned (per il feromone)
	* Risolto costruendo la classe feromone bypassando il ned.

2. Non conviene far decadere il feromone in maniera continua
	~~* Il workaround consiste nell'aggiornare correttamente (in termini temporali) il valore solo quando un veicolo attraversa il nodo. Alcuni nodi possono non essere aggiornati se non ci sono veicoli che li attraversano.~~
	* Il workaround consiste nell'aggiornare correttamente in termini temporali il valore da basecoord, emettendo un segnale decayFeromone che tutti i nodi sono in grado di leggere (modulo routing)

3. Delay di canale nel nodo ricevitore
	~ L'idea è mettere tempo di canale = 0 e trasferire la logica temporale nel concetto del selfmessage nel routing.

4. Dobbiamo usare il suo dijkstra, possiamo considerare un solo peso alla volta
	* Il workaround è che possiamo istanziare diverse topologie che si riferiscono alla stessa ma che considerano pesi diversi


nella sua shell 
simulazioni da riga di comando
per vedere le combinazioni
./AMoD_Simulator -x AMoD_Network -f simulations/omnetpp.ini -g


opp_runall -j2 ./AMoD_Simulator -c AMoD_Network -u Cmdenv -f simulations/omnetpp.ini -r 0..143

-j7 simulazioni contemporanee
./AMoD_Simulator  fa riferimento al .exe
AMoD_Network nome rete
Cmdenv omnet fornisce 2 environment - questo è da riga di comando
-r 0..1727 num di simulazioni

filter basic
name(differenceFrom*) || name(emergencyRequest*) || name(redCodeRequest*) || name(signal_ambulanceTravelTime*)

name(differenceFromRedCodeRequestToPickup:stats:max)


CAPITOLI TESI:
Scenario
Simulatore
Algoritmi applicati
Risultati



git diff --stat 227313141dd2ccc972ff61eab72ee9e3a2829a09 8c54719c6ae89e4ef570c