/** @file: petrinettransition.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro prechod petriho site
  */

#ifndef PETRINETTRANSITION_H
#define PETRINETTRANSITION_H

#include "../shared/petrinettransitioncore.h"
#include "../shared/publictypes.h"
#include "petrinetarc.h"
#include <QGraphicsRectItem>
#include <iostream>
#include <QBrush>

/** Prechod petriho site. */
class PetriNetTransitionGUI: public QGraphicsRectItem {
    //Q_OBJECT

public:
    /** identifikace typu objektu = prechod */
    enum { Type = UserType + E_Transition };
    
    /** konstruktor - pouziva se pri kreslni objektu uzivatelem. */
    PetriNetTransitionGUI(int _id, Lists *_lists, QMenu *_contextMenu, QGraphicsScene *_scene = 0);
    
    /** konstruktor - pouziva se pri tvorbe site z XML */
    PetriNetTransitionGUI(QMenu *_contextMenu, PetriNetTransitionCore *_transitionCore, 
                          QGraphicsScene *_scene = 0);
    
    /** destruktor */
    ~PetriNetTransitionGUI();
    /** Vraci uzivatelsky typ */
    int type() const { return Type;}
    /** pridani nove hrany do seznamu vystupu, to same pro jadro */
    void addArcOut(PetriNetArcGUI *arc) {
        arcsOut.append(arc); 
        //transitionCore->appendArcOut(arc->getArcCorePtr());
    }
    /** pridani nove hrany do seznamu vstupu, to same pro jadro */
    void addArcIn(PetriNetArcGUI *arc) {
        arcsIn.append(arc);
        //transitionCore->appendArcIn(arc->getArcCorePtr());
    }
    /** smaze vsechny hrany, ktere se s objektem poji */
    void deleteArcs(QGraphicsScene *scene);
    /** Smaze zaznam o dane hrane ze seznamu pripojenych hran */
    void deleteArcFromList(PetriNetArcGUI *arc, E_DeletedArcDirection dir);
    
    /** Vrati ulozeny retezec uzivatelem zadane straze */
    QString getGuardString() {return transitionCore->getGuard();}    
    
    /** Ulozi retezec uzivatelem zadane straze */
    void setGuardString(QString &_guard) {transitionCore->setGuard(_guard);}    
    
    /** Vrati ulozeny retezec uzivatelem zadane vystupni akce */
    QString getActionString() {return transitionCore->getAction();}    
    
    /** Ulozi retezec uzivatelem zadane vystupni akce */
    void setActionString(QString &_action) {transitionCore->setAction(_action);}
    
    /** Vrati ukazatel na stredovou linku */
    QGraphicsLineItem *getLinePtr() {return centerLine;}
    
    /** Aktualizace pozici a delku hrany */
    void updateLinePos();
    
    /** Nastavi text do guard */
    void setGuardText() {textGuard->setPlainText(transitionCore->getGuard());}
    
    /** Nastavi text do action */
    void setActionText() {textAction->setPlainText(transitionCore->getAction());}
    
    /** Prepoicta velikost prechodu a prekresli */
    void updateSize();
    
    /** aktualizace pozic text. poli tak, aby zapadly do obdelniku prechodu */
    void updateTextPos();
    
    /** vrati ukazatel na text. pole guard */
    QGraphicsTextItem *getTextGuardPtr() {return textGuard;}
    
    /** vrati ukazatel na text. pole action */
    QGraphicsTextItem *getTextActionPtr() {return textAction;}
    
    /** Vrati ukazatel na jadro prechodu */
    PetriNetTransitionCore *getTransCorePtr() {return transitionCore;}
    
    /** Inicializauje pozici PRECHODU ve scene a ulozi do jadra */
    void setPosition(QPointF pos) {setPos(pos); transitionCore->setCoords(pos);}
    
    /** Resetuje obarveni */
    void resetColor() {setBrush(QBrush(QColor(Qt::green)));} 
    
    /** Vrati ukazatel na seznam vystupnich hran */
    QList<PetriNetArcGUI *> *getArcsOutListPtr() {return &arcsOut;}
    
    
protected:
    /** Reakce na zmenu pozice objektu - prekresleni hrany. */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    /** Vyvolani kontextoveho menu */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    
private:
    PetriNetTransitionCore *transitionCore; /**< trida nesouci zakladni data a metody pro PRECHOD */
    QList<PetriNetArcGUI *> arcsOut; /**< Seznam hran vedoucich Z prechodu */
    QList<PetriNetArcGUI *> arcsIn;  /**< Seznam hran vedoucich DO prechodu */
    QMenu *contextMenu;           /**< Kontextove menu objektu. */
    QGraphicsLineItem *centerLine;/**< Stredova cara oddelujici straz a akci. */
    QGraphicsTextItem *textGuard; /**< Text (straz) zobrazovany uvnitr prechodu */
    QGraphicsTextItem *textAction;/**< Text (akce) zobrazovany uvnitr prechodu */
};

#endif // PETRINETTRANSITION_H
