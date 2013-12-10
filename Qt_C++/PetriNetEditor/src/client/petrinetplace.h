/** @file: petrinetplace.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro mista petriho site
  */

#ifndef PETRINETPLACE_H
#define PETRINETPLACE_H

#include "../shared/petrinetplacecore.h"
#include "petrinetarc.h"
#include "../shared/publictypes.h"
#include <QGraphicsEllipseItem>
#include <iostream>

/** Misto Petriho site. */
class PetriNetPlaceGUI: public QGraphicsEllipseItem {
    //Q_OBJECT

public:
    /** identifikace typu objektu = misto */
    enum { Type = UserType + E_Place };
    
    /** konstruktor - pouziva se pri kreslni objektu uzivatelem.
      * @param _contextMenu kontextove menu objektu
      */
    PetriNetPlaceGUI(int _id, Lists *_lists, QMenu *_contextMenu, QGraphicsScene *_scene = 0);
    
    /** konstruktor - pouziva se pri tvorbe site z XML */
    PetriNetPlaceGUI(QMenu *_contextMenu, PetriNetPlaceCore *_place, QGraphicsScene *_scene = 0);
    
    /** destruktor */
    ~PetriNetPlaceGUI() {delete text; delete place;}
    
    /** Vraci uzivatelsky typ (Type) */
    int type() const { return Type;}
    /** pridani nove hrany do seznamu vystupu */
    void addArcOut(PetriNetArcGUI *arc) {arcsOut.append(arc);}
    /** pridani nove hrany do seznamu vstupu */
    void addArcIn(PetriNetArcGUI *arc) {arcsIn.append(arc);}
    /** smaze vsechny hrany, ktere se s objektem poji */
    void deleteArcs(QGraphicsScene *scene);
    /** Smaze zaznam o dane hrane ze seznamu pripojenych hran */
    void deleteArcFromList(PetriNetArcGUI *arc, E_DeletedArcDirection dir);
    /** Vrati ulozeny retezec uzivatelem zadanych tokenu */
    QString &getTokensString() {return tokensString;}    
    
    /** Ulozi retezec tokenu a do jadra jej ulozi jako vektor intu */
    void setTokens(QString _tokensString) { 
        place->tokensStrToInt(_tokensString);
        setTokensString(place->tokensIntToStr()); 
    }
    /** Ulozi retezec uzivatelem zadanych tokenu */
    void setTokensString(QString _tokensString) {tokensString = _tokensString;}
    /** Vrati ukazatel na objekt textoveho pole */
    QGraphicsTextItem *getTextPointer() {return text;}
    /** Ulozi string tokenu do textoveho pole */
    void setTokensText() {text->setPlainText(tokensString);}
    /** Inicializauje pozici MISTA ve scene a ulozi do jadra */
    void setPosition(QPointF pos) {setPos(pos); place->setCoords(pos);}
    /** Aktualizuje velikost textoveho pole, mista a spravne umisti text */
    void updateSize();
    /** aktualizace pozice ctverce s textem tak, aby zapadl do kruznice */
    void updateTextPos();
    /** Vrati ukazatel na place */
    PetriNetPlaceCore *getPlaceCorePtr() {return place;}
    /** Vrati ukazatel na seznam vystupnich hran */
    QList<PetriNetArcGUI *> *getArcsOutListPtr() {return &arcsOut;}
    
protected:
    /** Reakce na zmenu pozice objektu - prekresleni hrany. */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);    
    /** Vyvolani kontextoveho menu */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    
private:
    PetriNetPlaceCore *place;     /**< trida nesouci zakladni data a metody pro MISTO */
    QGraphicsTextItem *text;      /**< Text (tokeny) zobrazovany uvnitr mista */
    QMenu *contextMenu;           /**< Kontextove menu objektu. */
    QString tokensString;         /**< Retezec tokenu zadanych uzivatelem. */
    QList<PetriNetArcGUI *> arcsOut; /**< Seznam hran vedoucich Z mista */
    QList<PetriNetArcGUI *> arcsIn;  /**< Seznam hran vedoucich DO mista */     
};

#endif // PETRINETPLACE_H
