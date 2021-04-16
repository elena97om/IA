/*ELENA ORTIZ MORENO CURSO JULIO 2020*/
#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado {
  int fila;
  int columna;
  int orientacion;
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayplan=false;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    estado actual, destino;
    bool zapatillas = false, bikini = false;
    list<Action> plan;
    bool hayplan, puedoDescubrir;
    int pasos_recalcular;
    bool recargas_calculadas_CU = false;
    bool recalcular_nuevos_descub = false;
    int cuenta_nuevos_descub = 0;

    bool acabo_de_calcular = false;
    bool powerup_avistado = false;
    const int limite_inf_bateria = 2800;

    // Métodos privados de la clase

    int calcular_coste_bateria(estado state, Action accion, bool zapatillas, bool bikini);

    bool pathFinding(int level, Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_CosteUniforme(const Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_A_estrella(const estado &origen, const estado &destino, list<Action> &plan, const Sensores & sensor);

    bool hay_aldeano(Sensores sensor);

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);

};

#endif
