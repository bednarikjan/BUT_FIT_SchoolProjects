/** @file: petrinetplace.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro mista petriho site
  */

#include "petrinetplace.h"
#include <QGraphicsScene>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QTextDocument>
#include <iostream>
#include <cmath>

// konstruktor
PetriNetPlaceGUI::PetriNetPlaceGUI(int _id, Lists *_lists, QMenu *_contextMenu, QGraphicsScene *_scene)
    : QGraphicsEllipseItem(-20.0, -20.0, 40.0, 40.0, 0, _scene), 
      contextMenu(_contextMenu) {
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    place = new PetriNetPlaceCore(_id, _lists, this);
    
    // nove textove pole pridruzene k mistu
    text = new QGraphicsTextItem;
    text->setFlag(QGraphicsItem::ItemIsSelectable, false);
    text->setFlag(QGraphicsItem::ItemIsMovable, false);
}

// konstruktor - pouziva se pri tvorbe site z XML
PetriNetPlaceGUI::PetriNetPlaceGUI(QMenu *_contextMenu, 
                                   PetriNetPlaceCore *_place,
                                   QGraphicsScene *_scene)
    : QGraphicsEllipseItem(-20.0, -20.0, 40.0, 40.0, 0, _scene),
      place(_place), contextMenu(_contextMenu) {
    
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    
    // nove textove pole pridruzene k mistu
    text = new QGraphicsTextItem;
    text->setFlag(QGraphicsItem::ItemIsSelectable, false);
    text->setFlag(QGraphicsItem::ItemIsMovable, false);   
    
    setTokensString(place->tokensIntToStr());
    setTokensText();
    setPos(place->getCoords());
    updateSize();
    // z nejakeho duvodu setPos() maze souradnice v jadru, proto je musim znova nastavit
    // nejspis za to muze event handelr itemchanged..
    place->setCoords(scenePos());
    
    place->placeGUI = this;
}

// reakce na zmenu pozice objektu - prekresleni hrany
QVariant PetriNetPlaceGUI::itemChange(GraphicsItemChange change, const QVariant &value) {
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
        
        // update souradnic ulozenych v jadre MISTA
        place->setCoords(scenePos());
        // update pozice textoveho pole
        updateTextPos();
    }
    
    return value;
}    

// Vyvolani kontextoveho menu
void PetriNetPlaceGUI::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    // Zruseni vyberu vseh objektu (zajima nas jen tento jeden)
    scene()->clearSelection();
    setSelected(true);
    //std::cout << "jsem tu" << std::endl;
    contextMenu->exec(event->screenPos());
}

// smaze vsechny hrany, ktere se s objektem poji
void PetriNetPlaceGUI::deleteArcs(QGraphicsScene *scene) {
    
    // smazani hran vedoucich Z mista
    foreach(PetriNetArcGUI *arc, arcsOut) {
        arc->deleteConnection(E_From);
        scene->removeItem(arc);
        delete arc;
    }
    
    // smazani hran vedoucich DO mista
    foreach(PetriNetArcGUI *arc, arcsIn) {
        arc->deleteConnection(E_To);
        scene->removeItem(arc);
        delete arc;
    }
}

// Smaze zaznam o dane hrane ze seznamu pripojenych hran.
void PetriNetPlaceGUI::deleteArcFromList(PetriNetArcGUI *arcToDelete, E_DeletedArcDirection dir) {
    int i = 0;
    
    // Bude se mazat ze seznamu vstupnich hran.
    if(dir == E_From) {
        foreach(PetriNetArcGUI *arc, arcsIn) {
            if(arc == arcToDelete) {
                arcsIn.removeAt(i);
                break;
            }
            i++;
        }
    }
    // Bude se mazat ze seznamu vystupnich hran.
    else {
        foreach(PetriNetArcGUI *arc, arcsOut) {
            if(arc == arcToDelete) {
                arcsOut.removeAt(i);
                break;
            }
            i++;
        }
    }
}

// Aktualizuje velikost textoveho pole, mista a spravne umisti text
void PetriNetPlaceGUI::updateSize() {
    qreal textLen;
    qreal idealLen;
    qreal newPlaceSize;
    
    // zjisteni delky textoveho pole
    textLen = text->document()->size().width();
    std::cout << "sirka zadaneho textu je: " << textLen << std::endl;
    
    std::cout << "sirka stranky: " 
              << text->document()->size().width() 
              << " delka stranky: " 
              << text->document()->size().height() 
              << std::endl;
    
    // spocitani idelani delky tak, aby pole bylo ctvercove
    idealLen = sqrt(text->document()->size().width() * text->document()->size().height());
    std::cout << "idealni sirka zadaneho textu je: " << idealLen << std::endl;
    
    // zmena rozmeru textoveho pole
    text->setTextWidth(idealLen);
    
    // spocitani nove delky ctverce obalujiciho kruznici mista
    // hodnotu priblizne 1.4 jsem spocital jako funkcni zavislost delky ctverce
    // obalujiciho kruznici na delce ctverce obalujiciho text. Vychzai to z pocitani
    // pruseciku diagonaly velkeho ctverce s kruznici.
    newPlaceSize = 1.4 * idealLen;
    
    // zmena rozmeru kruznice (obalujici ctverec bude mit 1,4 * delku ctverce textu)
    setRect(QRectF(-newPlaceSize/2, -newPlaceSize/2, newPlaceSize, newPlaceSize));
    
    // aktualizace pozice ctverce s textem tak, aby zapadl do kruznice
    // newPlaceSize/2 - (newPlaceSize - idealLen) / 2
    updateTextPos(); 
}

// aktualizace pozice ctverce s textem tak, aby zapadl do kruznice
void PetriNetPlaceGUI::updateTextPos() {
    qreal newPos = text->textWidth() / 2;
    
    text->setPos(scenePos()-QPointF(newPos, newPos));    
}
