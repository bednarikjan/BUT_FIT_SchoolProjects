/** @file: petrinetarc.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro hrany petriho site
  */

#ifndef PETRINETARC_H
#define PETRINETARC_H

#include "../shared/petrinetarccore.h"
#include "../shared/publictypes.h"
#include <QGraphicsLineItem>
#include <iostream>

/** Trida pro tvorbu oientovane hrany spojujici
  * misto-prechod nebo prechod-misto. K lince se pripoji
  * jeste polygon tvorici hlavu sipky. 
  */
class PetriNetArcGUI: public QGraphicsLineItem {

public:
    /** identifikace typu objektu = misto */
    enum { Type = UserType + E_Arc };
    
    /** Konstruktor - pouziva se pri kreslni objektu uzivatelem.
      * @param from[in] pocatecni objekt
      * @param to[in] koncovy objekt
      */
    PetriNetArcGUI(Lists *_lists, QGraphicsItem *_from, QGraphicsItem *_to, QMenu *_contextMenu);
    
    /** Konstruktor - pouziva se pri tvorbe site z XML */
    PetriNetArcGUI(QGraphicsItem *_from, QGraphicsItem *_to, 
                   QMenu *_contextMenu, PetriNetArcCore *_arcCore);
    
    /** Destruktor */
    ~PetriNetArcGUI();
    
    /** Vraci uzivatelsky typ (Type) */
    int type() const { return Type;}
    
    /** Aktualizace poctaecniho a koncoveho bodu, pokud se hybe
      * s pocatecnim nebo koncovym objektem
      */
    void updatePos();
    
    QPainterPath shape() const;
    
    /** Smaze zaznam o teto hrane v seznamu pripojenych hran daneho objektu */
    void deleteConnection(E_DeletedArcDirection dir);
    
    /** Vrati retezec uzivatelem zadane promenne nebo konstanty */
    QString getVarOrConstString() {return arcCore->getVarOrConst();}
    
    /** Ulozi retezec uzivatelem zadane promenne nebo konstanty */
    void setVarOrConstString(QString &_varOrConst) {arcCore->setVarOrConst(_varOrConst);}
    
    /** Vrati ukazatel na objekt textoveho pole */
    QGraphicsTextItem *getTextPointer() {return text;}
    /** Ulozi string tokenu do textoveho pole */
    void setVarOrConstText() {text->setPlainText(arcCore->getVarOrConst());}
    /** aktualizace pozice ctverce s textem */
    void updateTextPos();
    
    /** Vrati ukazatel na jadro HRANU */
    PetriNetArcCore *getArcCorePtr() {return arcCore;}
    
    /** Vrati ukazatel na graficky objekt from */
    QGraphicsItem *getFrom() {return from;}
    
    /** Vrati ukazatel na graficky objekt to */
    QGraphicsItem *getTo() {return to;}
    
protected:
    /** Vyvolani kontextoveho menu */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);
    
private:
    PetriNetArcCore *arcCore; /**< trida nesouci zakladni data a metody pro HRANU */
    QGraphicsItem *from;    /**< pocatecni objekt */
    QGraphicsItem *to;      /**< koncovy objekt */
    QMenu *contextMenu;     /**< Kontextove menu objektu. */
    QGraphicsTextItem *text;/**< Text (promenna nebo konstanta) zobrazovana u hrany */
    QPolygonF arcHead;      /**< Hlava sipky */
};

#endif // PETRINETARC_H
