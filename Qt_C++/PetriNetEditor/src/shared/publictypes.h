/** @file: publictypes.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida obsahuje typ objektu petriho site vykresleneho ve scene
  */

#ifndef PUBLICTYPES_H
#define PUBLICTYPES_H

// geometrie hlavniho okna
#define APP_X  300
#define APP_Y  100
#define APP_WIDTH  640
#define APP_HEIGHT 480

// implicitne nastaveny server a port
#define IMPLICIT_SERVER "127.0.0.1"
#define IMPLICIT_PORT "48825"

/** Typ objektu Petriho site vykreslenho ve scene.
  * Pouzivaji tridy PetriNetPlaceGUI a PetriNetTransitionGUI
  * pro identifikaci objektu pri pretypovani z QGraphicsItem
  */
enum E_PetriNetObjectType { 
    E_Transition = 3,
    E_Place = 4,
    E_Arc = 5
};

/** Role objektu, k nemuz se poji hrana, ktera ma byt smazana. */
enum E_DeletedArcDirection {
    E_From,
    E_To
};

#endif // PUBLICTYPES_H
