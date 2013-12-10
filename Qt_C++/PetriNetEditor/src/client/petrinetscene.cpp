/** @file: petrinetscene.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro hlavni scenu na kresleni grafu
  */

#include "petrinetscene.h"
#include "petrinetplace.h"
#include "petrinettransition.h"
#include "petrinetarc.h"
#include "appwindow.h"
#include <QGraphicsSceneMouseEvent>
#include <iostream>
#include <QGraphicsItem>
#include <QTextDocument>

// konstruktor
PetriNetScene::PetriNetScene(QObject * parent, QMenu *_contextMenu)
    : QGraphicsScene(parent), contextMenu(_contextMenu), fileName(""), saved(false),
      sceneChanged(false), netFromServerChanged(false), generatedID(1) {
    
    //connect(this, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(sSceneChanged()));
    lists = new Lists;
    version="";
}

/* Reakce na kliknuti mysi do sceny. Pokud je zvolen jiny mod, nez
 * E_move, bude se pridavat novy objekt (misto/prechod/hrana)
 */
void PetriNetScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    //std::cout << "klik" << std::endl;
    // Pokud bylo stisknuto prave tlacitko, nechci provadet zadnou 
    // z nasledujicich akci, nubrz se bude zobrazovat kontextove menu
    if(mouseEvent->buttons() != Qt::LeftButton) return;
    
    PetriNetTransitionGUI *trans;
    PetriNetPlaceGUI *place;
    // seznam objektu, ktere protina bod, kam klikla mys.
    QList<QGraphicsItem *> intersectedItems = items(mouseEvent->scenePos());
    
    switch (AppWindow::getMode()) {
        case AppWindow::E_insPlace:
            place = new PetriNetPlaceGUI(generateID(), getListsPtr(), contextMenu);
            place->setBrush(QColor(Qt::green));     
            place->setPen(QPen(QColor(Qt::black)));
            addItem(place);
            //std::cout << "pozice mista: " << place->scenePos().x() << ", " << place->scenePos().y() << std::endl;
            place->setPosition(mouseEvent->scenePos());
            //place->setPos(mouseEvent->scenePos());
            /*
            std::cout << "pozice elipsy: x = " 
                      << place->scenePos().x()
                      << ", y = " 
                      << place->scenePos().y() 
                      << std::endl;
            */
            addItem(place->getTextPointer());
            place->getTextPointer()->setPos(QPointF(place->scenePos().x() - 20.0, place->scenePos().y() - 20.0));
            
            //std::cout << "sirka textu: " << place->getTextPointer()->document()->size().width() << std::endl;
            //place->getTextPointer()->setTextWidth(54.0);
            
            // scena se zmenila
            setSceneChanged(true);  
            break;
        
        case AppWindow::E_insTrans:
            trans = new PetriNetTransitionGUI(generateID(), getListsPtr(), contextMenu);
            trans->setBrush(QColor(Qt::green));     
            trans->setPen(QPen(QColor(Qt::black)));            
            addTransition(trans, mouseEvent->scenePos());
            //addItem(trans);
            //trans->setPos(mouseEvent->scenePos());
            // scena se zmenila
            setSceneChanged(true);  
            break;
            
        case AppWindow::E_insArc:
            // Seznam objektu, ktere pocatecni bod hrany protina
            //intersectedItems = items(mouseEvent->scenePos());
            if(intersectedItems.length() > 0) {
                // vytvoreni usecky o delce 0 (z bodu kliknuti do bodu kliknuti)
                arc = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(), 
                                               mouseEvent->scenePos()));
                // POZOR, sirka lajny musi byt definovana, jinak bude 0 a to znamena, 
                // ze se sice vykresli, ale pri mouseReleaseEvent nad prazdnou oblasti
                // neprotne sama sebe!!!
                arc->setPen(QPen(QColor(Qt::black), 1.0));
                addItem(arc);   
            }// else {arc = 0;}
        break;
            break;
        
        default: break;
    }
    // volani defaultniho event handleru - resi hybani s objekty.
    QGraphicsScene::mousePressEvent(mouseEvent);
}

// Prida do sceny prechod
void PetriNetScene::addTransition(PetriNetTransitionGUI *trans, QPointF mousePos) {
    // pridani prechodu
    addItem(trans);
    trans->setPosition(mousePos);
    
    // pridani linky uvnitr prechodu
    trans->updateLinePos();
    addItem(trans->getLinePtr());
    
    // pridani text poli
    trans->updateTextPos();
    addItem(trans->getTextGuardPtr());
    addItem(trans->getTextActionPtr());    
}

/*
void PetriNetScene::sSceneChanged() {
    std::cout << "Scena se zmenila" << std::endl;
}
*/

void PetriNetScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    if(mouseEvent->buttons() != Qt::NoButton) {
        //std::cout << "pohyb mysi" << std::endl;
        switch (AppWindow::getMode()) {
            // kresli se hrana
            case AppWindow::E_insArc:
                if(arc != 0) {
                    arc->setLine(QLineF(arc->line().p1(), mouseEvent->scenePos()));
                    //arc->setZValue(1.0);
                }
                break;
            
            // pohyb objektem
            case AppWindow::E_moveItem:
                // volani implicitniho handleru (resi pohyb objektu)
                QGraphicsScene::mouseMoveEvent(mouseEvent);
                // pokud je vybrany nejaky objekt, hybe se objektem a scena se tedy zmenila
                if(selectedItems().length() > 0)
                    setSceneChanged(true);  
                break;
            
            default: break;
        }
    }
}

/* Reakce na uvolneni tlacitka mysi. Reimplementace event handleru pouze
 * kvuli kresleni hran. Pro pripad presun objektu se vola implicitni handler.
 */
void PetriNetScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {    
    QPointF pFrom, pTo;
    
    // rezim vkladani hrany
    if((AppWindow::getMode() == AppWindow::E_insArc) && (arc != 0)) {
        pFrom = arc->line().p1();
        pTo = arc->line().p2();
        
        // seznam objektu, ktere protina pocatecni bod hrany
        QList<QGraphicsItem *> start = items(pFrom);
        if(start.length() > 0) {
            start.removeOne(arc);
        }
        
        // seznam objektu, ktere protina koncovy bod hrany
        QList<QGraphicsItem *> end = items(pTo);
        if(end.length() > 0) {
            end.removeOne(arc);
        }
        
        // odstranim usecku ze sceny a pirpadne ji pozdeji nahradim vyslednou hranou (sipkou)
        removeItem(arc);
        delete arc;     // TODO: na tomto miste to zrjeme hazi segfaulty, ale nevim proc..
                        // kdyztak zrusit...
        
        // odstraneni textovych poli ze seznamu protnutych objektu
        foreach(QGraphicsItem * it, start) {
            if(it->type() == QGraphicsTextItem::Type)
                start.removeOne(it);
        }
        foreach(QGraphicsItem * it, end) {
            if(it->type() == QGraphicsTextItem::Type)
                end.removeOne(it);
        }
        
        // zacatek i konec linky protina nejaky objekt
        if(start.length() > 0 && end.length() > 0) {
            // spojeni misto-prechod            
            if(((start.first()->type()) == PetriNetPlaceGUI::Type) &&
                (end.first()->type()    == PetriNetTransitionGUI::Type)) {
                // kontrola, zda se nejdena o duplicitni hranu
                if(!isArcDuplicate(start.first(), end.first())) {
                    PetriNetArcGUI *newArc = new PetriNetArcGUI(getListsPtr(), start.first(), end.first(), contextMenu);
                    addItem(newArc);
                    addItem(newArc->getTextPointer());
                    newArc->updatePos();
                    newArc->updateTextPos();
                    qgraphicsitem_cast<PetriNetPlaceGUI *>(start.first())->addArcOut(newArc);
                    qgraphicsitem_cast<PetriNetTransitionGUI *>(end.first())->addArcIn(newArc);
                    // scena se zmenila
                    setSceneChanged(true);  
                }
            } else 
            // spojeni prechod-misto
            if(((start.first()->type()) == PetriNetTransitionGUI::Type) &&
                (end.first()->type()    == PetriNetPlaceGUI::Type)) {
                // kontrola, zda se nejdena o duplicitni hranu
                if(!isArcDuplicate(start.first(), end.first())) {
                    PetriNetArcGUI *newArc = new PetriNetArcGUI(getListsPtr(), start.first(), end.first(), contextMenu);
                    addItem(newArc); 
                    addItem(newArc->getTextPointer());
                    newArc->updatePos();
                    newArc->updateTextPos();
                    qgraphicsitem_cast<PetriNetTransitionGUI *>(start.first())->addArcOut(newArc);
                    qgraphicsitem_cast<PetriNetPlaceGUI *>(end.first())->addArcIn(newArc);
                    // scena se zmenila
                    setSceneChanged(true);  
                }
            }
        }
    }
    arc = 0;
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

// Overi, zda neni prave kreslena hrana duplicitni
bool PetriNetScene::isArcDuplicate(QGraphicsItem *from, QGraphicsItem *to) {
    bool duplicate = false;
    PetriNetPlaceGUI *place;
    PetriNetTransitionGUI *transition;
    
    //spojeni mito-prechod
    if(from->type() == PetriNetPlaceGUI::Type) {
        place = qgraphicsitem_cast<PetriNetPlaceGUI *>(from);
        foreach(PetriNetArcGUI *arc, *(place->getArcsOutListPtr())) {
            if(arc->getTo() == to) {
                duplicate = true;
                break;
            }
        }
    } 
    //spojeni prechod-misto
    else {
        transition = qgraphicsitem_cast<PetriNetTransitionGUI *>(from);
        foreach(PetriNetArcGUI *arc, *(transition->getArcsOutListPtr())) {
            if(arc->getTo() == to) {
                duplicate = true;
                break;
            }
        }
    }
    
    return duplicate;
}

// Vystavi graficky obal nad jadrem site a sit vykresli
void PetriNetScene::drawPetriNet(QMenu *contextMenu) {
    PetriNetPlaceGUI *place;
    PetriNetTransitionGUI *transition;
    PetriNetArcGUI *arc;
    
    QGraphicsItem *from, *to;
    
    // vykresleni MIST
    foreach(PetriNetPlaceCore *placeCore, lists->places) {
        place = new PetriNetPlaceGUI(contextMenu,placeCore,0);
        place->setBrush(QColor(Qt::green));     
        place->setPen(QPen(QColor(Qt::black)));
        addItem(place);
        addItem(place->getTextPointer());
    }
    
    // vykresleni PRECHODU
    foreach(PetriNetTransitionCore *transitionCore, lists->transitions) {
        transition = new PetriNetTransitionGUI(contextMenu, transitionCore);
        if(transitionCore->isDoable())
            transition->setBrush(QColor(Qt::blue));     
        else
            transition->setBrush(QColor(Qt::green));     
        transition->setPen(QPen(QColor(Qt::black)));    
        addItem(transition);        
        addItem(transition->getLinePtr());
        addItem(transition->getTextGuardPtr());
        addItem(transition->getTextActionPtr());            
    }
    
    // Vykresleni HRAN
    foreach(PetriNetArcCore *arcCore, lists->arcs) {
        // ziskani ukazatelu na graficke objekty, kter ma hrana propojit
        if(arcCore->getFrom()->getType() == PetriNetNode::E_PlaceCore) {
            from = static_cast<PetriNetPlaceCore *>(arcCore->getFrom())->placeGUI;
            to = static_cast<PetriNetTransitionCore *>(arcCore->getTo())->transGUI;
        } else {
            from = static_cast<PetriNetTransitionCore *>(arcCore->getFrom())->transGUI;
            to = static_cast<PetriNetPlaceCore *>(arcCore->getTo())->placeGUI;
        }
        
        arc = new PetriNetArcGUI(from, to, contextMenu, arcCore);
        addItem(arc);
        addItem(arc->getTextPointer());
        
        // nastaveni ukazatele na tuto hranu do seznamu vstupnich/vystupnich hran
        // u prislusneho prechodu a mista
        if(arc->getFrom()->type() == PetriNetPlaceGUI::Type) {
            qgraphicsitem_cast<PetriNetPlaceGUI *>(arc->getFrom())->addArcOut(arc);
            qgraphicsitem_cast<PetriNetTransitionGUI *>(arc->getTo())->addArcIn(arc);
        } else {
            qgraphicsitem_cast<PetriNetTransitionGUI *>(arc->getFrom())->addArcOut(arc);
            qgraphicsitem_cast<PetriNetPlaceGUI *>(arc->getTo())->addArcIn(arc);
        }
    }
}

// Odtsrani aktualne zobrazenou sit a uvolni pamet.
void PetriNetScene::removeNet() {
    // mista
    foreach(PetriNetPlaceCore *place, lists->places) {
        removeItem(place->placeGUI->getTextPointer());
        removeItem(place->placeGUI);
        delete place->placeGUI;
    }
    
    // prechody
    foreach(PetriNetTransitionCore *trans, lists->transitions) {
        removeItem(trans->transGUI->getLinePtr());
        removeItem(trans->transGUI->getTextGuardPtr());
        removeItem(trans->transGUI->getTextActionPtr());
        removeItem(trans->transGUI);
        delete trans->transGUI;
    }
    
    // hrany
    foreach(PetriNetArcCore *arc, lists->arcs) {
        removeItem(arc->arcGUI->getTextPointer());
        removeItem(arc->arcGUI);
        delete arc->arcGUI;
    }
}

// Po simulaci - zrusi priznak doable a obarveni u vsech prechodu.
void PetriNetScene::resetAllDoableTransitions() {
    foreach(PetriNetTransitionCore *trans, lists->transitions) {
        trans->setDoable(false);
        trans->transGUI->resetColor();
    }
}
