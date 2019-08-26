COSE DA CHIEDERE AD ANDREA:
--lanciare omnet da riga di comando

--Il simulatore è errato: il tempo calcolato da setAdditionalTravelTime in abstractnetworkmanager.h tiene conto dell'accelerazione ma è finta. Omnet tira dritto e non conta l'accelerazione

--Ha senso imporre la velocità al veicolo, la velocità di canale = 0 e utilizzare il self message per determinare i tempi?
	(speed in manhattanrouting.cc e .h hardcoded)



TODONEXTTIME:

-creazione traffico civile (non taxi - indipendente dal coordinatore)

segnale creato dall'app per traffico civile nell'initialize

al receive signal se lo smezza
	se arriva a dest lo deleta -> e ne manda un altro


TODO Importanti:

risultati
	* Differenza tempo minimo di esecuzione (in assenza di traffico) con tempo reale di esecuzione
	* 
algoritmi
	* ACO (network manager)
	* AAA (network manager)
	* Custom nostro
scenari
	* emergenziali non ospedalieri




TODO mildly important:

--BUG: manhattanrouting.cc - trafficDelay: abbiamo aggiunto un epsilon pari a +0.00001 perchè c'è un bug nel simulatore (coi double) e finisce con lo schedulare il veicolo nel passato (confrontando i numeri c'è tipo 0.000000001 di differenza)

-- Le richieste vengono generate dal TRipRequestSubmitter con una frequenza del 50% tra normali e di emergenza. (confrontare le idee e valutare la strategia scelta)

-- cambiare selfmessage con sendDelayed(pk,sendDelayTime,"out");


--Check che tutto sia in inglese



TODO Non importanti:
evitare che scorra tutti i veicoli per cercare un'ambulanza  - in HeuristicCord.cc

* Enum vari (gates, tipiveicoli)

* funzione di decay feromone a metodo? manhattanrouting (riga 121 - 132)

DONE:

DONE-- vettore di feromoni invece che hardcoded 4 feromoni. - in manhattanrouting.cc e ned
DONE-- cambiare indice porte per adattare il suo al nostro o vv.  -manhattanrouting.cc (e pheromone) -- o il suo amod.ned

-fattore feromone -- DONE
-decadenza feromone -- DONE (in manhattanrouting.cc)

-risultati
	Abbiamo i risultati dei feromoni nei quattro canali di ogni nodo
	
traffico
	c'è la classe traffico ma non è ancora utilizzata
	-> workaround automessaggio
	(parametrizzare aumento feromone e traffico) done partially
		fix traffic influence
	-? emit traffico




	Limiti del simulatore:

1. Non si possono inserire vettori nei parametri del ned (per il feromone)
	* Risolto costruendo la classe feromone bypassando il ned.

2. Non conviene far decadere il feromone in maniera continua
	* Il workaround consiste nell'aggiornare correttamente (in termini temporali) il valore solo quando un veicolo attraversa il nodo. Alcuni nodi possono non essere aggiornati se non ci sono veicoli che li attraversano.