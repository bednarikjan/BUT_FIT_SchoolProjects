/** @file: appwindow.h
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Trida pro hlavni okno programu a pro komunikaci se serverem
  */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "petrinetscene.h"
#include "appwindowdialogs.h"
#include "session.h"
#include "../shared/myXML.h"
#include "syntaxchecker.h"
#include <QMainWindow>
#include <QGraphicsView>
#include <QVector>
#include <QButtonGroup>
#include <QMessageBox>
#include <QDebug>
#include <QtNetwork>


/** Hlavni okno programu.
  */
class AppWindow : public QMainWindow {    
    Q_OBJECT

public:
    AppWindow();    /**< konstruktor */
    ~AppWindow();   /**< destruktor */
    
    /** Mody rozhrani ovlivnujici dostupnost menu/tlacitek */
    enum E_InterfaceMode {
        E_itemSelected,
        E_itemLostFocus,
        E_connected,
        E_disconneced,
        E_noTab,
        E_newTab,
        E_startSimulation,
        E_stopSimulation
    };
    
    /** Mody programu podle stisknuteho tlacitka z nastrojove listy. */
    enum E_PetriNetMode {
        E_moveItem,   /**< sipka - pohyb objketu po scene*/
        E_insPlace,   /**< misto - vlozeni mista*/
        E_insTrans,   /**< prechod - vlozeni prechodu*/
        E_insArc      /**< hrana - spojeni misto-prechod/prechod-misto*/
    }; 
    
    /** Nastaveni modu programu. @param[in] _mode novy mod */
    void setMode(E_PetriNetMode _mode) {mode = _mode;}
    
    /** Ziskani aktualniho modu. @return aktualni mod programu */
    static E_PetriNetMode getMode() {return mode;}
    
    /** Ziskani pouze jmena souboru bez absolutni cesty - reseni nezavisle na platforme. */
    QString getFileNameExcludingPath(QString &name);
    
    /** Naplni clensky objekt tridy Session informacemi o sezeni */
    void setSession();
    
    /** Aktualizuje mod rozhrani */
    void updateInterfaceMode(E_InterfaceMode newMode);
    
    SyntaxChecker syntax;

private:
    QTcpSocket* socket; /**< Socket pro komunikaci.*/
    void myConnect();   /**< Pripojeni serveru.*/
    void write(QString data); /**< Odesle zpravu serveru. */
    QByteArray rawdata; /**< Data pro cteni zpravy. */
    myXML* xml; /**< Data ve formatu xml. */
    QString myName; /**< Jmeno souboru v aktualni zalozce */
    bool flagSim;   /**< Priznak, zda chce uzivatel zacit simulovat */
    bool choose(QString str,QString *name,QString *version); /**< Vrati jmeno a verzi vybrane uzivatelem */

    int tabsOpendSoFar; /**< Pocet doposud otevrenych tabu - generovani cisla noveho tabu */
    bool simulationRunning;

    bool loadConf(); /**< nacte konfiguracni nastaveni */

    // GUI aplikace
    QMenu *menuFile;            /**< <em>File</em>*/    
    QMenu *menuServer;          /**< <em>Server</em>*/    
    QMenu *menuSimulation;      /**< <em>Simulation</em>*/
    QMenu *menuHelp;            /**< <em>Help</em>*/        
    QMenu *contextMenuItem;     /**< Kontextove menu - prave tlacitko an objekt*/        
    QToolBar *toolbarNet;       /**< toolbar pro praci s objekty site */
    QButtonGroup *netButtons;   /**< vyber kreslici/hybaci akce*/
    QTabWidget *tabs;           /**< karty pro jednotlive site */
    
    // akce pro menu File
    QAction *actNew;            /**< menu <em>File</em> -> <em>New net</em>*/
    QAction *actOpen;           /**< menu <em>File</em> -> <em>Open</em>*/
    QAction *actOpenFromServer; /**< menu <em>File</em> -> <em>Open from server</em>*/
    QAction *actClose;          /**< menu <em>File</em> -> <em>Close</em>*/
    QAction *actSave;           /**< menu <em>File</em> -> <em>Save</em>*/
    QAction *actSaveAs;         /**< menu <em>File</em> -> <em>Save As</em>*/
    QAction *actSaveToServer;   /**< menu <em>File</em> -> <em>Save to server</em>*/
    QAction *actInfo;           /**< menu <em>File</em> -> <em>Info</em>*/
    QAction *actExit;           /**< menu <em>File</em> -> <em>Exit</em>*/
    
    // akce pro menu Server
    QAction *actNewUser;        /**< menu <em>Server</em> -> <em>Connect</em>*/
    QAction *actDisconnect;     /**< menu <em>Server</em> -> <em>Disconnect</em>*/
    QAction *actConnect;        /**< menu <em>Server</em> -> <em>New user</em>*/
    
    // akce pro menu Simualation
    QAction *actStartSimulation;/**< menu <em>Simulation</em> -> <em>Start</em>*/
    QAction *actStopSimulation; /**< menu <em>Simulation</em> -> <em>Stop</em>*/
    QAction *actOneStep;        /**< menu <em>Simulation</em> -> <em>One setp</em>*/
    QAction *actWholeNet;       /**< menu <em>Simulation</em> -> <em>Whole Net</em>*/
    
    // akce pro menu Help
    QAction *actHelp;           /**< menu <em>Help</em> -> <em>Help</em>*/
    QAction *actDocumentation;  /**< menu <em>Help</em> -> <em>Documentation</em>*/
    QAction *actAbout;          /**< menu <em>Help</em> -> <em>About</em>*/
    
    // akce pro kontextove menu Item
    QAction *actDelete;          /**< kontextove menu Item <em>Delete</em> */
    QAction *actProperties;      /**< kontextove menu Item <em>Properties</em> */
    
    // Widgety pro kresleni grafu
    QVector<PetriNetScene *> scenes;    /**< vektor aktualne oteverenych scen */
    QVector<QGraphicsView *> views;     /**< vektor aktualne otevrenych pohledu */
    
    // Dialogova okna
    DialogPlace *dialogPlace;
    DialogTransition *dialogTransition;
    DialogArc *dialogArc;
    DialogConnect *dialogConnect;


    
    Session *session;      /**< Info o pripojni (login, passwd, server, port) */
    
    static E_PetriNetMode mode;    /**< aktualni mod pri tvorbe petriho site (zda a co se bude kreslit/hybat)*/
    E_InterfaceMode interfaceMode; /**< aktualni mod rozhrani, ovlivnuje dostupnost menu/tlacitek */
    
    // privatni metody:
    /** 
     * Tvorba akci, ktere vyvolavaji polozky menu a tlacitka toolBaru.
     * @return void
     */
    void createActions();
    
    /** Tvorba polozek menu.
     * @return void */
    void createMenus();
    
    /** Tvorba nastrojovych list. */
    void createToolbars();
    
    /** Tvorba stavove listy. */
    void createStatusbar();
    
    /** Tvorba tlacitka nastrojove listy. 
     * @param[in] name popis tlacitka
     * @param[in] icon cesta k ikone
     * @return vytvorene tlacitko s popiskem (jako QWidget)
     */
    QWidget *createNetButton(const QString &label, const QString &iconName, E_PetriNetMode type);
    
    /** Vrati ukazatel na objekt PetriNetScene, ktery je jako scena zobrazen
     * v aktalne otevrene karte.
     */
    PetriNetScene *getCurrentPetriNetScene() {return scenes[tabs->currentIndex()];}
    
    /** Ulozi soubor na disk */
    bool saveFile(QString &name);
    
private slots:
    // ------------- MENU -------------- //
    void sNew();            /**< Vytvori novou kartu. */
    void sOpen();           /**< Otevre sit z lokalniho disku. */
    void sOpenFromserver(); /**< Otevre sit ze serveru. */
    int  sClose();          /**< Zavre kartu. */
    bool sSave();           /**< Ulozi zmeny lokalne. */
    bool sSaveAs();         /**< Ulozi sit lokalne s vyberem umisteni. */
    void sSaveToServer();   /**< Ulozi zmeny na server. */
    void sInfo();           /**< Zobrazi informace o siti ze serveru. */
    void sExit();
    
    void sDelete();          /**< smaze vybrane objekty. */
    void sProperties();      /**< nabidne dialogove okno pro zadani parametru objektu. */
    
    void sConnect();        /**< Pripoji se k serveru. */
    void sDisconnect();     /**< Odpojeni od serveru */
    void sNewUser();        /**< Registrace noveho uzivatele na server. */
    
    void sStartSimul();     /**< Pozada server o zahajeni simulace. */
    void sStopSimul();      /**< Pozada server o ukonceni simulace. */
    void sWholeNet();       /**< Spusti simulaci cele site. */
    void sOneStep();        /**< Jeden krok simulace. */
    
    void sHelp();           /**< Zobrazi napovedu. */
    void sDocumentation();  /**< Zobrazi dokumentaci generovanou doxygenem. */
    void sAbout();          /**< O programu. */

    void sConnected();       /**< Pripojeno k serveru. */
    void sDisconnected();    /**< Odpojeni od serveru. */
    void sReply();           /**< Prichozi odpoved od serveru. */
    void sError(QAbstractSocket::SocketError);           /**< Chyba spojeni. */
    
    // ------------- nastrojova lista -------------- //
    /** 
     * Nastaveni noveho modu programu podle stisknuteho tlacitka.
     * @param b index tlacitka 
     */
    void sNetButtonsClicked(int b); 
    
    /** Slot -> Kontrola zmeny vyberu objektu ve scene. V zavislosti na 
     * vybranem/nevybranem objektu se zpristupnuje menu Item. */
    void sSelectionChanged();

};

#endif // MAINWINDOW_H
