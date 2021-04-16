/*ELENA ORTIZ MORENO CURSO JULIO 2020*/
#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
#include <map>
#include <unordered_set>

double distancia (int fila_0, int columna_0, int fila_destino, int columna_destino) {
	return abs(fila_0 - fila_destino) + abs(columna_0 - columna_destino);
};

// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	Action accion = actIDLE;
	// Estoy en el nivel 1, 2 o 3
	if(!hayplan){
		actual.fila = sensores.posF;
		actual.columna = sensores.posC;
		actual.orientacion = sensores.sentido;
		destino.fila = sensores.destinoF;
		destino.columna = sensores.destinoC;
		hayplan = pathFinding(sensores.nivel, sensores, actual, destino, plan);
	}

	Action sigAccion;
	if (hayplan and plan.size()>0){
		sigAccion = plan.front();
		plan.erase(plan.begin());
	}
	else{
		cout << "No esta implementado" << endl;
	}

	return sigAccion;
}


// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan){
	switch (level){
		case 0: cout << "Busqueda en profundad\n";
			      return pathFinding_Profundidad(origen,destino,plan);
						break;
		case 1: cout << "Busqueda en Anchura\n";
			      return pathFinding_Anchura(origen,destino,plan);
						break;
		case 2: cout << "Busqueda Costo Uniforme\n";
						int bateria;
						return pathFinding_CosteUniforme(sensor, origen, destino, plan);
						break;
		case 3: cout << "Busqueda A*\n";
						return pathFinding_A_estrella(origen, destino, plan, sensor);
						break;
		case 4: cout << "Busqueda Retro Extra\n";
						break;
	}
	cout << "Comportamiento sin implementar\n";
	return false;
}


//---------------------- Implementación de la busqueda en profundidad ---------------------------

// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}




struct nodo{
	estado st;
	list<Action> secuencia;
};

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) or (a.fila == n.fila and a.columna > n.columna) or
	      (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};

bool ComportamientoJugador::hay_aldeano(Sensores sensor) {
	return sensor.superficie[2] == 'a';
}

// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	stack<nodo> pila;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

//───────────────────────────────────────────────────────────IMPLEMENTACIÓN ANCHURA───────────────────────────────────────────────────────────


bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan){
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados; // Lista de Cerrados
	queue<nodo> pila;											// Lista de Abiertos

  nodo current;
	current.st = origen;
	current.secuencia.empty();

	pila.push(current);

  while (!pila.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.front();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

//IMPLEMENTACIÓN COSTE UNIFORME:
// ───────────────── ESTRUCTURAS DE DATOS NECESARIAS PARA UNIFORM COST SEARCH ─────
//

	class estado_A: public estado {
	public:
		double coste = 0;
		bool zapatillas = false;
		bool bikini = false;
	};

	class nodo_A: public nodo {
	public:
		estado_A st;
		double coste_hasta_aqui = 0;
		bool   zapatillas       = false;
		bool   bikini           = false;
	};


	// Estructura:
	// Bateria -> B, fila -> f, columna -> c, zapatilla -> z, bikini -> b
	// ffccBBBBzb
	struct hash_estado_bateria {
		size_t operator()(const estado_A& state) const {
			return 		state.fila * 100000000 + state.columna * 1000000
					+ 	state.coste * 100
					+   state.zapatillas * 10 + state.bikini;
		}
	};
//
// ───────────────────────────────────────────────────── FUNCIONES AUXILIARES ─────
//


int ComportamientoJugador::calcular_coste_bateria(estado state, Action accion, bool zapatillas, bool bikini) {
	int sig_fila = state.fila, sig_col = state.columna;

	if (accion = actFORWARD) {
		switch (state.orientacion) {
			case 0: sig_fila--; break;
			case 1: sig_col++; break;
			case 2: sig_fila++; break;
			case 3: sig_col--; break;
		}
	}

	int costo;
	char casilla = mapaResultado[sig_fila][sig_col];

	switch (casilla)
	{
		case 'A':
			if (bikini)
				costo = 10;
			else
				costo = 100;

			break;

		case 'B':
			if (zapatillas)
				costo = 5;
			else
				costo = 50;

			break;

		case 'T':
			costo = 2;
			break;

		case '?':
			if (bikini && zapatillas)
				costo =  7;
			else if (bikini)
				costo = 10;
			else if (zapatillas)
				costo = 5;
			break;

		case 'X':
			costo = -10;
			break;

		default:
			costo = 1;
			break;
	}

	return costo;
}


// ─────────────────────────────────────────────────────────── COSTE UNIFORME ───────────────────────────────────────────────────────────


bool ComportamientoJugador::pathFinding_CosteUniforme(const Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan) {
	cout << "Calculando ruta\n";
	plan.clear();


	struct prioridad_nodo_bateria {
		bool operator()(const nodo_A& n1, const nodo_A& n2){
			return n1.coste_hasta_aqui < n2.coste_hasta_aqui;
		}
	};


	multiset<nodo_A, prioridad_nodo_bateria> a_expandir;
	set<estado_A, ComparaEstados> generados;


	auto encontrar_en_expandir = [&a_expandir](const nodo_A& nodo) {
		for (multiset<nodo_A, prioridad_nodo_bateria>::const_iterator it = a_expandir.begin(); it != a_expandir.end(); ++it) {
			if ((*it).st.fila == nodo.st.fila && (*it).st.columna == nodo.st.columna && (*it).st.orientacion == nodo.st.orientacion && (*it).zapatillas == nodo.zapatillas && (*it).bikini == nodo.bikini) {
				return it;
			}
		}
		return a_expandir.end();
	};


	nodo_A actual;

	actual.st.fila          = origen.fila;
	actual.st.columna       = origen.columna;
	actual.st.orientacion   = origen.orientacion;
	actual.coste_hasta_aqui = 0;
	actual.st.coste         = 0;
	actual.bikini           = false;
	actual.zapatillas       = false;

	a_expandir.insert(actual);

	while (!a_expandir.empty() && (actual.st.fila != destino.fila || actual.st.columna != destino.columna)) {
		a_expandir.erase(a_expandir.begin());
		generados.insert(actual.st);



		// HIJO DERECHA

		nodo_A hijo_TR = actual;
		hijo_TR.st.orientacion = (hijo_TR.st.orientacion + 1)%4;

		if (generados.find(hijo_TR.st) == generados.end()) {
			hijo_TR.coste_hasta_aqui += calcular_coste_bateria(hijo_TR.st, actTURN_R, hijo_TR.zapatillas, hijo_TR.bikini);
			hijo_TR.st.coste = hijo_TR.coste_hasta_aqui;

			// Buscar por si tenemos que expandirlo
			auto it = encontrar_en_expandir(hijo_TR);

			if (it != a_expandir.end() && hijo_TR.coste_hasta_aqui < it->coste_hasta_aqui) {
				// Sustituir
				a_expandir.erase(it);
				hijo_TR.secuencia.push_back(actTURN_R);
				a_expandir.insert(hijo_TR);

			}
			else {
				hijo_TR.secuencia.push_back(actTURN_R);
				a_expandir.insert(hijo_TR);
			}


		}


	// HIJO IZQUIERDA


		nodo_A hijo_TL = actual;

		hijo_TL.st.orientacion = (hijo_TL.st.orientacion + 3)%4;

		if (generados.find(hijo_TL.st) == generados.end()) {
			hijo_TL.coste_hasta_aqui += calcular_coste_bateria(hijo_TL.st, actTURN_L, hijo_TL.zapatillas, hijo_TL.bikini);
			hijo_TL.st.coste = hijo_TL.coste_hasta_aqui;

			// Buscar por si tenemos que expandirlo
			auto it = encontrar_en_expandir(hijo_TL);

			if (it != a_expandir.end() && hijo_TL.coste_hasta_aqui < it->coste_hasta_aqui) {
				// Sustituir
				a_expandir.erase(it);
				hijo_TL.secuencia.push_back(actTURN_L);
				a_expandir.insert(hijo_TL);

			}
			else {
				hijo_TL.secuencia.push_back(actTURN_L);
				a_expandir.insert(hijo_TL);
			}


		}

		//HIJO DELANTE

		nodo_A hijo_forward = actual;

		if (!HayObstaculoDelante(hijo_forward.st) && generados.find(hijo_forward.st) == generados.end()) {
			hijo_forward.coste_hasta_aqui += calcular_coste_bateria(hijo_forward.st, actFORWARD, hijo_forward.zapatillas, hijo_forward.bikini);
			hijo_forward.st.coste = hijo_forward.coste_hasta_aqui;

			if (mapaResultado[hijo_forward.st.fila][hijo_forward.st.columna] == 'K')
				hijo_forward.bikini = true;
			if (mapaResultado[hijo_forward.st.fila][hijo_forward.st.columna] == 'D')
				hijo_forward.zapatillas = true;

			auto it = encontrar_en_expandir(hijo_forward);

			if (it != a_expandir.end()) {
				if (hijo_forward.coste_hasta_aqui < it->coste_hasta_aqui) {
					a_expandir.erase(it);
					hijo_forward.secuencia.push_back(actFORWARD);
					a_expandir.insert(hijo_forward);
				}
			}
			else {
				hijo_forward.secuencia.push_back(actFORWARD);
				a_expandir.insert(hijo_forward);
			}
		}


		if (!a_expandir.empty()) {
			actual = *(a_expandir.begin());
		}
	}

	if (actual.st.fila == destino.fila && actual.st.columna == destino.columna) {
		cout << "Cargando plan\n";
		plan = actual.secuencia;

		cout << "Longitud del plan:" << plan.size() << endl;
		PintaPlan(plan);

		VisualizaPlan(origen, plan);

		return true;
	}
	else {
		cout << "F en el chat, no he encontrado ruta\n";
		return false;
	}
}





//───────────────────────────────────────────────────────────IMPLEMENTACIÓN A*───────────────────────────────────────────────────────────




bool ComportamientoJugador::pathFinding_A_estrella(const estado &origen, const estado &destino, list<Action> &plan, const Sensores &sensor) {
	cout << "Calculando ruta\n";
	plan.clear();

	auto ordenar_prioridades = [](const nodo_A n1, const nodo_A n2) {
		return n1.coste_hasta_aqui > n2.coste_hasta_aqui;
	};

	priority_queue<nodo_A, vector<nodo_A>, decltype(ordenar_prioridades)> a_expandir (ordenar_prioridades);
	set<estado, ComparaEstados> generados;

	nodo_A actual;

	actual.st.fila        = origen.fila;
	actual.st.columna     = origen.columna;
	actual.st.orientacion = origen.orientacion;
	actual.zapatillas     = zapatillas;
	actual.bikini         = bikini;

	actual.secuencia.clear();

	a_expandir.push(actual);

	double peso_h = 1;
	double peso_g = 1;

	if (bikini && zapatillas) {
		peso_h = 2;
	}

	while (!a_expandir.empty() && (actual.st.fila != destino.fila || actual.st.columna != destino.columna)) {
		a_expandir.pop();
		generados.insert(actual.st);


		//HIJO DERECHA


		nodo_A hijo_TR = actual;
		hijo_TR.st.orientacion = (hijo_TR.st.orientacion + 1)%4;

		int coste_TR =
			  peso_h * distancia(hijo_TR.st.fila, hijo_TR.st.columna, destino.fila, destino.columna)
			+ peso_g * calcular_coste_bateria(hijo_TR.st, actTURN_R, hijo_TR.zapatillas, hijo_TR.bikini);

		hijo_TR.coste_hasta_aqui = coste_TR;
		hijo_TR.st.coste = coste_TR;

		if (generados.find(hijo_TR.st) == generados.end()) { // Si no está el hijo en la lista de generados

			hijo_TR.secuencia.push_back(actTURN_R);
			a_expandir.push(hijo_TR);
		}


		// HIJO IZQUIERDA


		nodo_A hijo_TL = actual;
		hijo_TL.st.orientacion = (hijo_TL.st.orientacion + 3)%4;
		int coste_TL =
			  peso_h * distancia(hijo_TL.st.fila, hijo_TL.st.columna, destino.fila, destino.columna)
			+ peso_g * calcular_coste_bateria(hijo_TL.st, actTURN_L, hijo_TL.zapatillas, hijo_TL.bikini);

		hijo_TL.coste_hasta_aqui = coste_TL;
		hijo_TL.st.coste = coste_TL;

		if (generados.find(hijo_TL.st) == generados.end()) { // Si no está el hijo en la lista de generados
			hijo_TL.secuencia.push_back(actTURN_L);
			a_expandir.push(hijo_TL);
		}


		//HIJO DELANTE

		nodo_A hijo_forward = actual;
		// Comrpobar que en la casilla de delante del origen no hay un NPC
		int sig_fila_origen = origen.fila, sig_col_origen = origen.columna;
		switch (origen.orientacion) {
			case 0: sig_fila_origen--; break;
			case 1: sig_col_origen++; break;
			case 2: sig_fila_origen++; break;
			case 3: sig_col_origen--; break;
		};

		int coste_forward =
			  peso_h * distancia(hijo_forward.st.fila, hijo_forward.st.columna, destino.fila, destino.columna)
			+ peso_g * calcular_coste_bateria(hijo_forward.st, actFORWARD, hijo_forward.zapatillas, hijo_forward.bikini);

		hijo_forward.coste_hasta_aqui = coste_forward;
		hijo_forward.st.coste = coste_forward;

		if ( !HayObstaculoDelante(hijo_forward.st)) {
			if ( generados.find(hijo_forward.st) == generados.end() ) {

				hijo_forward.secuencia.push_back(actFORWARD);
				a_expandir.push(hijo_forward);
			}
		}

		if (!a_expandir.empty()) {
			actual = a_expandir.top();

			if (actual.st.fila == sig_fila_origen && actual.st.columna == sig_col_origen && hay_aldeano(sensor) && !a_expandir.empty()) {
				a_expandir.pop();
				actual = a_expandir.top();
			}
		}
	}

	cout << "Búsqueda terminada\n";

	if (actual.st.fila == destino.fila && actual.st.columna == destino.columna) {
		cout << "Cargando plan\n";
		plan = actual.secuencia;

		cout << "Longitud del plan:" << plan.size() << endl;
		PintaPlan(plan);

		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "F en el chat, no hemos encontrado plan\n";
		return false;

	}
}




// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
