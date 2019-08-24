COSE DA CHIEDERE AD ANDREA:
--lanciare omnet da riga di comando

--Il simulatore è errato: il tempo calcolato da setAdditionalTravelTime in abstractnetworkmanager.h tiene conto dell'accelerazione ma è finta. Omnet tira dritto e non conta l'accelerazione




TODONEXTTIME:

traffico 
	c'è la classe traffico ma non è ancora utilizzata
	-> workaround automessaggio
-fattore traffico



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
--Check che tutto sia in inglese



TODO Non importanti:
evitare che scorra tutti i veicoli per cercare un'ambulanza  - in HeuristicCord.cc




DONE:

DONE-- vettore di feromoni invece che hardcoded 4 feromoni. - in manhattanrouting.cc e ned
DONE-- cambiare indice porte per adattare il suo al nostro o vv.  -manhattanrouting.cc (e pheromone) -- o il suo amod.ned

-fattore feromone -- DONE
-decadenza feromone -- DONE (in manhattanrouting.cc)

-risultati
	Abbiamo i risultati dei feromoni nei quattro canali di ogni nodo






	Limiti del simulatore:

1. Non si possono inserire vettori nei parametri del ned (per il feromone)
	* Risolto costruendo la classe feromone bypassando il ned.

2. Non conviene far decadere il feromone in maniera continua
	* Il workaround consiste nell'aggiornare correttamente (in termini temporali) il valore solo quando un veicolo attraversa il nodo. Alcuni nodi possono non essere aggiornati se non ci sono veicoli che li attraversano.