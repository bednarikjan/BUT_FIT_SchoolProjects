/** @file: petrinetarc.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro hrany petriho site
  */

#include "petrinettransition.h"
#include "petrinetplace.h"
#include "petrinetarc.h"
#include "../shared/publictypes.h"
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <cmath>

const qreal Pi = 3.14;

// konstruktor
PetriNetArcGUI::PetriNetArcGUI(Lists *_lists, QGraphicsItem *_from, QGraphicsItem *_to, QMenu *_contextMenu)
    : from(_from), to(_to) , contextMenu(_contextMenu) {
    
    PetriNetNode *fromNode, *toNode;
    // Nastaveni usecky. Poc. a konc. bod je stred poc. a konc. objektu namapovany
    // do souradnic sceny.
    //setLine(QLineF(mapFromItem(from, 0.0, 0.0), mapFromItem(to, 0.0, 0.0)));
    
    // Hrana se musi zobrazovat pod ostatnimy objekty
    setZValue(-1.0);
    
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setPen(QPen(QBrush(QColor(Qt::black)), 3.0, Qt::SolidLine, Qt::RoundCap));
    
    // zjisteni, zda je from Place nebo Transition a zavolani konstruktoru PetriNetArcCore.
    if(from->type() == PetriNetPlaceGUI::Type) {
        fromNode = (qgraphicsitem_cast<PetriNetPlaceGUI *>(from))->getPlaceCorePtr();
        toNode = (qgraphicsitem_cast<PetriNetTransitionGUI *>(to))->getTransCorePtr();
    } else {
        fromNode = (qgraphicsitem_cast<PetriNetTransitionGUI *>(from))->getTransCorePtr();
        toNode = (qgraphicsitem_cast<PetriNetPlaceGUI *>(to))->getPlaceCorePtr();
    }
    arcCore = new PetriNetArcCore(_lists, this, fromNode, toNode);
    arcCore->setVarOrConst("new");
    
    // nove textove pole pridruzene k hrane
    text = new QGraphicsTextItem;
    text->setFlag(QGraphicsItem::ItemIsSelectable, false);
    text->setFlag(QGraphicsItem::ItemIsMovable, false);
    text->setDefaultTextColor(QColor(Qt::darkGreen));
    setVarOrConstText();
}

// Konstruktor - pouziva se pri tvorbe site z XML
PetriNetArcGUI::PetriNetArcGUI(QGraphicsItem *_from, QGraphicsItem *_to, 
               QMenu *_contextMenu, PetriNetArcCore *_arcCore)
    : arcCore(_arcCore), from(_from), to(_to), contextMenu(_contextMenu) {

    setLine(QLineF(mapFromItem(from, 0.0, 0.0), mapFromItem(to, 0.0, 0.0)));
    setZValue(-1.0);
    
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setPen(QPen(QBrush(QColor(Qt::black)), 3.0, Qt::SolidLine, Qt::RoundCap));
    
    // nove textove pole pridruzene k hrane
    text = new QGraphicsTextItem;
    text->setFlag(QGraphicsItem::ItemIsSelectable, false);
    text->setFlag(QGraphicsItem::ItemIsMovable, false);
    text->setDefaultTextColor(QColor(Qt::darkGreen));
    
    setVarOrConstText();
    updateTextPos();
    
    arcCore->arcGUI = this;
}

// destruktor
PetriNetArcGUI::~PetriNetArcGUI() {
    delete text;
    delete arcCore;
}

QPainterPath PetriNetArcGUI::shape() const {
    QPainterPath path = QGraphicsLineItem::shape();
    path.addPolygon(arcHead);
    return path;
}

void PetriNetArcGUI::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
          QWidget *) {
    
    QPen myPen = pen();
    myPen.setColor(QColor(Qt::black));
    myPen.setWidthF(1.0);
    qreal arrowSize = 20;
    painter->setPen(myPen);
    painter->setBrush(QColor(Qt::black));
    
    setLine(QLineF(to->pos(), from->pos()));
    
    double angle = ::acos(line().dx() / line().length());
    if (line().dy() >= 0)
        angle = (Pi * 2) - angle;

    QPointF headEnd;
    // x
    if(line().dx() <= 0) {
        headEnd.setX(line().p1().x() - abs(line().dx())/3);
    } else {
        headEnd.setX(line().p1().x() + abs(line().dx())/3);
    }
    // y
    if(line().dy() <= 0) {
        headEnd.setY(line().p1().y() - abs(line().dy())/3);
    } else {
        headEnd.setY(line().p1().y() + abs(line().dy())/3);
    }
    
    QPointF arcP1 = headEnd + QPointF(sin(angle + Pi / 3) * arrowSize,
                                    cos(angle + Pi / 3) * arrowSize);
    QPointF arcP2 = headEnd + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
                                    cos(angle + Pi - Pi / 3) * arrowSize);

    arcHead.clear();
    arcHead << headEnd << arcP1 << arcP2;
    painter->drawLine(line());
    painter->drawPolygon(arcHead);
}

// Vyvolani kontextoveho menu
void PetriNetArcGUI::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    // Zruseni vyberu vseh objektu (zajima nas jen tento jeden)
    scene()->clearSelection();
    setSelected(true);
    contextMenu->exec(event->screenPos());
}

// Aktualizace pozice koncovych bodu hrany
void PetriNetArcGUI::updatePos() {
    setLine(QLineF(mapFromItem(from, 0.0, 0.0), mapFromItem(to, 0.0, 0.0)));
}

// Smaze zaznam o teto hrane v seznamu pripojenych hran daneho objektu.
void PetriNetArcGUI::deleteConnection(E_DeletedArcDirection dir) {
    // maze se zaznam v koncovem objektu
    if(dir == E_From && to) {
        // pocatecni objekt je misto
        if(to->type() == PetriNetPlaceGUI::Type)
            qgraphicsitem_cast<PetriNetPlaceGUI *>(to)->deleteArcFromList(this, E_From);
        // pocatecni objekt je prechod
        else
            qgraphicsitem_cast<PetriNetTransitionGUI *>(to)->deleteArcFromList(this, E_From);
        to = 0;
    } 
    // maze se zaznam v pocatecnim objektu
    else if(dir == E_To && from){
        // pocatecni objekt je misto
        if(from->type() == PetriNetPlaceGUI::Type)
            qgraphicsitem_cast<PetriNetPlaceGUI *>(from)->deleteArcFromList(this, E_To);
        // pocatecni objekt je prechod
        else
            qgraphicsitem_cast<PetriNetTransitionGUI *>(from)->deleteArcFromList(this, E_To);
        from = 0;
    }
}

// aktualizace pozice ctverce s textem
void PetriNetArcGUI::updateTextPos() {
    qreal newX, newY;
    
    newX = line().x1() + 2*(line().x2() - line().x1())/3;
    newY = line().y1() + 2*(line().y2() - line().y1())/3;
    
    text->setPos(QPointF(newX, newY));
}
