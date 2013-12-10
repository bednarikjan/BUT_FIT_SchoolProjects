/** @file: petrinetscene.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro hlavni scenu na kresleni grafu
  */

#ifndef PETRINETSCENE_H
#define PETRINETSCENE_H

#include "../shared/lists.h"
#include "petrinettransition.h"
#include <QGraphicsScene>
#include <QMenu>

/** Hlavni scena pro kresleni grafu. */
class PetriNetScene: public QGraphicsScene {
    Q_OBJECT

private:
    QGraphicsLineItem *arc;    /**< aktualne kreslena hrana */
    QMenu *contextMenu;
    QString fileName;          /**< jmeno souboru, v nemz je sit ulozena */
    QString version;           /**< verze souboru,pokud je ulozen na serveru */
    QString nameServer;         /**< jmeno souboru,pod kterym je ulozen na serveru */
    bool saved;                /**< priznak, zda uz byl soubor ulozen lokalne na disk */
    bool sceneChanged;         /**< Priznak modifikace site (pri zavreni karty nabidne ulozeni)*/
    bool netFromServerChanged; /**< Jako sceneCganged, ale pouziva se pro zjisteni, zda
                                  * byla modifikovana sit nactena ze serveru - kvuli simulaci */
    int generatedID;           /**< ID generovane pro mista a prechody */
    Lists *lists;              /**< Seznamy ukazatelu na mista, prechody a hrany */
    QString info;              /**< Popis ulozene site a logy simulaci */
    
    /** Overi, zda neni prave kreslena hrana duplicitni */
    bool isArcDuplicate(QGraphicsItem *from, QGraphicsItem *to);
    
protected:
    /** reimplementace eventu pro reakci na kliknuti tlacitkem do sceny */
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    
    /** reakce na pohyb mysi - vola se vzdy, pokud je stiskle tlacitko mysi a
      * hybe se kurzorem. */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    
    /** reakce na uvolneni tlacitka mysi */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    
//public slots:
    //void sSceneChanged();
    
public:
    /** konstruktor. 
     * @param parent otec sceny - posila se konstruktoru QWidgetu - bude to AppWindow
     */
    PetriNetScene(QObject *parent, QMenu *_contextMenu);
    
    /** Vrati jmeno souboru, v nemz je sit ulozena */
    QString &getFileName() {return fileName;}
    /** Vrati verzi souboru, ktere je i na serveru */
    QString &getVersion() {return version;}
    /** Vrati jmeno souboru, ktere je i na serveru */
    QString &getNameServer() {return nameServer;}
    /** Vrati informace o souboru. */
    QString &getInfo() {return info;}
    
    /** Nastavi jmeno souboru, v nemz je sit ulozena */
    void setFileName(QString &_fileName) {fileName = _fileName;}
    /** Nastavi verzi souboru, ktera je ulozeno i na serveru */
    void setVersion(QString &_version) {version=_version;}
    /** Nastavi jmeno souboru, ktera je ulozeno i na serveru */
    void setNameServer(QString &_nameServer) {nameServer=_nameServer;}
    /** Nastavi informace o souboru. */
    void setInfo(QString &_info) {info=_info;}
    
    /** Vrati priznak, zda uz byl soubor ulozen lokalne na disk*/
    bool getSaved() {return saved;}
    
    /** Nastavi priznak, zda uz byl soubor ulozen lokalne na disk*/
    void setSaved(bool _saved) {saved = _saved;}
    
    /** Nastavi priznak, zda se scena zmenila.
      * netFromServerChanged je pouze typu SET (proto vzdy true). Na false
      * ji muze nastavit pouze pri nacteni site ze serveru */
    void setSceneChanged(bool _sceneChanged) {sceneChanged = _sceneChanged; 
                                              netFromServerChanged = true;}
    
    /** Vrati priznak, zda se scena zmenila */
    bool getSceneChanged() {return sceneChanged;}
    
    /** shodi priznak modifikace nactene site ze serveru (pouze pri nacteni site ze serveru) */
    void resetNetfromServerChanged() {netFromServerChanged = false;}
    
    void setNetFromServerChanged() {netFromServerChanged = true;}
    
    /** vrati priznak modifikace nactene site ze serveru */
    bool getNetFromServerChanged() {return netFromServerChanged;}
    
    /** Prida do sceny prechod */
    void addTransition(PetriNetTransitionGUI *trans, QPointF mousePos);
    
    /** Generovani noveho cisla ID pro misto nebo prechod
      * Cislo se nejprve vrati a pak inkrementuje */    
    int generateID() {return generatedID++;}
    
    /** Vrati ukazatel na objekt nesouci seznamy mist, orechodu a hran */
    Lists *getListsPtr() {return lists;}
    
    /** Vystavi graficky obal nad jadrem site a sit vykresli */
    void drawPetriNet(QMenu *contextMenu);
    
    void setGeneratedID(int id) {generatedID = id;}
    
    /** Odtsrani aktualne zobrazenou sit a uvolni pamet. */
    void removeNet();
    
    /** Po simulaci - zrusi priznak doable a obarveni u vsech prechodu */
    void resetAllDoableTransitions();
};

#endif // PETRINETSCENE_H
