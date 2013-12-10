/** @file: petrinettransition.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro prechod petriho site
  */

#include "petrinettransition.h"
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QTextDocument>

// konstruktor
PetriNetTransitionGUI::PetriNetTransitionGUI(int _id, Lists *_lists, QMenu *_contextMenu, QGraphicsScene *_scene)
    : QGraphicsRectItem(-40.0, -20.0, 80.0, 40.0, 0, _scene),
      contextMenu(_contextMenu) {
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    transitionCore = new PetriNetTransitionCore(_id, _lists, this);
    
    // nova stredova cara
    centerLine = new QGraphicsLineItem;
    
    // nove textove pole pridruzene k mistu
    textGuard = new QGraphicsTextItem;
    textAction = new QGraphicsTextItem;
    textGuard->setFlag(QGraphicsItem::ItemIsSelectable, false);
    textGuard->setFlag(QGraphicsItem::ItemIsMovable, false);
    textAction->setFlag(QGraphicsItem::ItemIsSelectable, false);
    textAction->setFlag(QGraphicsItem::ItemIsMovable, false);
}

// konstruktor - pouziva se pri tvorbe site z XML
PetriNetTransitionGUI::PetriNetTransitionGUI(QMenu *_contextMenu, 
                                             PetriNetTransitionCore *_transitionCore, 
                                             QGraphicsScene *_scene)
    : QGraphicsRectItem(-40.0, -20.0, 80.0, 40.0, 0, _scene),
      transitionCore(_transitionCore), contextMenu(_contextMenu) {

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    // nova stredova cara
    centerLine = new QGraphicsLineItem;
    
    // nove textove pole pridruzene k mistu
    textGuard = new QGraphicsTextItem;
    textAction = new QGraphicsTextItem;
    textGuard->setFlag(QGraphicsItem::ItemIsSelectable, false);
    textGuard->setFlag(QGraphicsItem::ItemIsMovable, false);
    textAction->setFlag(QGraphicsItem::ItemIsSelectable, false);
    textAction->setFlag(QGraphicsItem::ItemIsMovable, false);
    
    setGuardText();
    setActionText();
    setPos(transitionCore->getCoords());
    updateSize();
    updateLinePos();
    transitionCore->setCoords(scenePos());
    
    // z nejakeho duvodu setPos() maze souradnice v jadru, proto je musim znova nastavit
    // nejspis za to muze event handelr itemchanged..
    transitionCore->transGUI = this;
}

// destruktor
PetriNetTransitionGUI::~PetriNetTransitionGUI() {
    delete centerLine;
    delete textGuard;
    delete textAction;
    delete transitionCore;
}

// reakce na zmenu pozice objektu - prekresleni hrany
QVariant PetriNetTransitionGUI::itemChange(GraphicsItemChange change, const QVariant &value) {
    if(change == QGraphicsItem::ItemPositionChange) {
        // update pozice hran vedoucich do mista
        if(arcsIn.length() > 0)
            foreach(PetriNetArcGUI *arc, arcsIn) {
                arc->updatePos();           
                arc->updateTextPos();
            }
        
        // update pozice hran vedoucich z mista
        if(arcsOut.length() > 0)
            foreach(PetriNetArcGUI *arc, arcsOut) {
                arc->updatePos();           
                arc->updateTextPos();
            }
        
        updateLinePos();
        updateTextPos();
        
        // update souradnic ulozenych v jadre PRECHODU
        transitionCore->setCoords(scenePos());
    }
    
    return value;
}   

// Vyvolani kontextoveho menu
void PetriNetTransitionGUI::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    // Zruseni vyberu vseh objektu (zajima nas jen tento jeden)
    scene()->clearSelection();
    setSelected(true);
    contextMenu->exec(event->screenPos());
}

// smaze vsechny hrany, ktere se s objektem poji
void PetriNetTransitionGUI::deleteArcs(QGraphicsScene *scene) {
    
    // smazani hran vedoucich Z prechodu
    foreach(PetriNetArcGUI *arc, arcsOut) {
        arc->deleteConnection(E_From);
        scene->removeItem(arc);
        delete arc;
    }
    
    // smazani hran vedoucich DO prechodu
    foreach(PetriNetArcGUI *arc, arcsIn) {
        arc->deleteConnection(E_To);
        scene->removeItem(arc);
        delete arc;
    }
}

// Smaze zaznam o dane hrane ze seznamu pripojenych hran.
void PetriNetTransitionGUI::deleteArcFromList(PetriNetArcGUI *arcToDelete, E_DeletedArcDirection dir) {
    int i = 0;
    
    // Bude se mazat ze seznamu vstupnich hran.
    if(dir == E_From) {
        foreach(PetriNetArcGUI *arc, arcsIn) {
            if(arc == arcToDelete) {
                arcsIn.removeAt(i);
            }
            i++;
        }
    }
    // Bude se mazat ze seznamu vystupnich hran.
    else {
        foreach(PetriNetArcGUI *arc, arcsOut) {
            if(arc == arcToDelete) {
                arcsOut.removeAt(i);
            }
            i++;
        }
    }
}

// Aktualizace pozici a delku hrany
void PetriNetTransitionGUI::updateLinePos() {
    QPointF center = scenePos();
    QPointF left = center;
    QPointF right = center;
    qreal halfWidth = rect().width()/2 - 1.0;
    
    left.setX(center.x() - halfWidth);
    right.setX(center.x() + halfWidth);
    
    centerLine->setLine(QLineF(left, right));
}

// Prepoicta velikost prechodu a prekresli
void PetriNetTransitionGUI::updateSize() {
    qreal textGuardLen; 
    qreal textActionLen;
    qreal maxWidth;
    
    textGuardLen = textGuard->document()->size().width();
    textActionLen = textAction->document()->size().width();
    
    // vyber vetsi sirky
    maxWidth = textGuardLen > textActionLen ? textGuardLen : textActionLen;
    
    // nastaveni nove sirky obdelniku
    setRect(-maxWidth/2, -rect().height()/2, maxWidth, rect().height());
    
    // aktualizce pozici obou textovych poli
    updateTextPos();
}

// aktualizace pozic text. poli tak, aby zapadly do obdelniku prechodu
void PetriNetTransitionGUI::updateTextPos() {
    QPointF rectCenter = scenePos();
    QPointF newGuardPos;
    QPointF newActionPos;

    newGuardPos.setX(rectCenter.x() - rect().width()/2);    
    newGuardPos.setY(rectCenter.y() - rect().height()/2);
    
    newActionPos.setX(rectCenter.x() - rect().width()/2);    
    newActionPos.setY(rectCenter.y());
    
    textGuard->setPos(newGuardPos);
    textAction->setPos(newActionPos);
}
