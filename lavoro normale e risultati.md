# COSE DA CHIEDERE AD ANDREA:
--lanciare omnet da riga di comando

~~ --Il simulatore è errato: il tempo calcolato da setAdditionalTravelTime in abstractnetworkmanager.h tiene conto dell'accelerazione ma è finta. Omnet tira dritto e non conta l'accelerazione ~~

--ha senso imporre la velocità al veicolo, la velocità di canale = 0 e utilizzare il self message per determinare i tempi?
	(speed in manhattanrouting.cc e .h hardcoded)



# TODONEXTTIME:


4. esploratore; implementazione snapshot per esplorazione della rete
	alla fine, i non esplorati non sono raggiungibili
	non è ammissibile accettare richieste da loro

4. heatmap
5. punti di ammassamento in ingresso ee uscita ai bordi della mesh
6. mezzo pesante circuito in zone di ammassamento
6. traffico (civile) che scappa
7. ambulanza più posti
8. autobus: punti di raccolta taxi ride sharing
9. più ospedali
10.

routing - djstra!?


Inizializzazione nodi: 
1. Epicentro esplosione parametrizzato
2. l'epicentro, prima di esplodere, ha una probabilità di inviare un pacco bomba ai nodi limitrofi
3. il nodo limitrofo, che riceve un pacco bomba, ha una probabilità di inviare un altro pacco bomba ai SUOI nodi vicini
4. poi esplode







-rilascio feromone secondo tipo

// esploratore?? ci serve?!
# TITOLI:
algoritmi bioispirati coordinamento ambulanze
l'altro, gestione dell emergency managment, simmulazione dinamiche produzione informazioni, concetto mezzo pesante,
strategie per la simulazione delle condizioni a contorno di un evento catastrofico

* coordinamento di ambulanze attraverso strategie bioispirate in uno scenario di disastro naturale


* simulazioni delle condizioni a contorno di un evento catastrofico, approccio bioispirato


# TODO Importanti:

risultati
	* Differenza tempo minimo di esecuzione (in assenza di traffico) con tempo reale di esecuzione
	 
algoritmi
	* ACO (network manager)
	* AAA (network manager)
	* Custom nostro

scenari
	* emergenziali non ospedalieri



* calcolo corretto setAdditionalTravelTime
* gestire coda richieste pendenti

# TODO mildly important:


-- Le richieste vengono generate dal TripRequestSubmitter con una frequenza del 50% tra normali e di emergenza. (confrontare le idee e valutare la strategia scelta)


--Check che tutto sia in inglese (E' già in inglese a meno dei log)

-- ManhattanNetworkManager::getTimeDistance(
refactor e rimozione metodo (parte di rimozione additionalTravelTime)


# TODO Non importanti:
* evitare che scorra tutti i veicoli per cercare un'ambulanza  - in HeuristicCord.cc
* scorrere meno nodi in manhattanrouting topo disconnessione
* cambiare colore ospedali

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

# Limiti del simulatore:

1. Non si possono inserire vettori nei parametri del ned (per il feromone)
	* Risolto costruendo la classe feromone bypassando il ned.

2. Non conviene far decadere il feromone in maniera continua
	* Il workaround consiste nell'aggiornare correttamente (in termini temporali) il valore solo quando un veicolo attraversa il nodo. Alcuni nodi possono non essere aggiornati se non ci sono veicoli che li attraversano.

3. Delay di canale nel nodo ricevitore
	~ L'idea è mettere tempo di canale = 0 e trasferire la logica temporale nel concetto del selfmessage nel routing.




simulazioni da riga di comando
per vedere le combinazioni
./AMoD_Simulator -x AMoD_Network -f simulations/omnetpp.ini -g


* opp_runall -j2 ./AMoD_Simulator -c AMoD_Network -u Cmdenv -f simulations/omnetpp.ini -r 0..1

-j7 simulazioni contemporanee
./AMoD_Simulator  fa riferimento al .exe
AMoD_Network nome rete
Cmdenv omnet fornisce 2 environment - questo è da riga di comando
-r 0..1727 num di simulazioni

