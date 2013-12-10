/** @file: appwindow.cpp
  * @author Jan Bednarik
  * @author Petr Konvalinka
  *
  * Trida pro hlavni okno programu a pro komunikaci se serverem
  */

#include "appwindow.h"
#include "petrinetplace.h"
#include "petrinettransition.h"
#include "petrinetarc.h"
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <iostream>
#include <QToolBar>
#include <QStatusBar>
#include <QToolButton>
#include <QLabel>
#include <QGraphicsItem>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QInputDialog>
#include <iostream>
#include <cassert>
#include <QDesktopServices>
#include <QFileInfo>


// povinna inicializace staticke slozky tridy AppWindow
AppWindow::E_PetriNetMode AppWindow::mode = AppWindow::E_moveItem;

// konstruktor
AppWindow::AppWindow(): tabsOpendSoFar(0), simulationRunning(false) {
    createActions();
    createMenus();

    // vytvoreni noveho socketu
    socket = new QTcpSocket(this);
    xml= new myXML();
    flagSim=false;

    // napojeni socketovych signalu na nase sloty
    connect(socket, SIGNAL(connected()), SLOT(sConnected()));
    connect(socket, SIGNAL(disconnected()), SLOT(sDisconnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(sReply()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(sError(QAbstractSocket::SocketError)));

    // Widget s kartami pro jednotlive editory
    tabs = new QTabWidget();
    //tabs->setMovable(true); // mozna bude delat problemy, pripaden smazat
    tabs->setTabShape(QTabWidget::Triangular);
    
    // Tvorba nove sceny - pouziti slotu pro tvorbu sceny
    sNew();
    createToolbars();
    createStatusbar();
    
    session = new Session;
    
    // Tvorba dialogovych oken
    dialogPlace = new DialogPlace;
    dialogTransition = new DialogTransition;
    dialogArc = new DialogArc;
    dialogConnect = new DialogConnect;

    
    // rozlozeni widgetu v hlavnim aplikacnim okne
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabs);
    
    // tvorba hlavniho aplikacniho okna
    QWidget *appWindow = new QWidget;    
    appWindow->setLayout(mainLayout);
    
    setCentralWidget(appWindow);
    
    // inicializace rozhrani -> dostupnost voleb menu
    updateInterfaceMode(E_noTab);
    updateInterfaceMode(E_newTab);
    updateInterfaceMode(E_itemLostFocus);
    updateInterfaceMode(E_disconneced);


}

// destruktor
AppWindow::~AppWindow() {
    delete session;
}

// Aktualizuje aktualni mod rozhrani ovlivnujcii dostupnost menu/tlacitek
void AppWindow::updateInterfaceMode(E_InterfaceMode newMode) {
    interfaceMode = newMode;
    
    switch(newMode) {
        case E_noTab:
            actClose->setEnabled(false);
            actSave->setEnabled(false);
            actSaveAs->setEnabled(false);
            actSaveToServer->setEnabled(false);
            actInfo->setEnabled(false);
            actDelete->setEnabled(false);
            actProperties->setEnabled(false);
            actStartSimulation->setEnabled(false);
            actStopSimulation->setEnabled(false);
            actOneStep->setEnabled(false);
            actWholeNet->setEnabled(false);
            break;
            
        case E_newTab:
            actClose->setEnabled(true);
            actSave->setEnabled(true);
            actSaveAs->setEnabled(true);
            actInfo->setEnabled(true);
            // zaktivneni tlacitek jen tehdy, pokud je klient pripojen k serveru
            if(socket->state() == QAbstractSocket::ConnectedState) {
                actSaveToServer->setEnabled(true);
                actOpenFromServer->setEnabled(true);
                actStartSimulation->setEnabled(true);
                //actOneStep->setEnabled(true);
            }
            break;
        
        case E_itemSelected:
            actDelete->setEnabled(true);
            actProperties->setEnabled(true);
            break;
            
        case E_itemLostFocus:
            actDelete->setEnabled(false);
            actProperties->setEnabled(false);
            break;
        
        case E_connected:
            actConnect->setEnabled(false);
            actNewUser->setEnabled(false);
            actOpenFromServer->setEnabled(true);
            actSaveToServer->setEnabled(true);
            actDisconnect->setEnabled(true);
            actStartSimulation->setEnabled(true);
            break;
            
        case E_disconneced:
            actConnect->setEnabled(true);
            actNewUser->setEnabled(true);
            actDisconnect->setEnabled(false);
            actStartSimulation->setEnabled(false);
            actStopSimulation->setEnabled(false);
            actOneStep->setEnabled(false);
            actWholeNet->setEnabled(false);
            actOpenFromServer->setEnabled(false);
            actSaveToServer->setEnabled(false);
            if(actStopSimulation->isEnabled()) {
                updateInterfaceMode(E_stopSimulation);
            }
            break;
            
        case E_startSimulation:
            actNew->setEnabled(false);
            actOpen->setEnabled(false);
            actOpenFromServer->setEnabled(false);
            actClose->setEnabled(false);
            actSave->setEnabled(false);
            actSaveAs->setEnabled(false);
            actSaveToServer->setEnabled(false);
            actInfo->setEnabled(false);
            actExit->setEnabled(false);
            actDelete->setEnabled(false);
            actProperties->setEnabled(false);
            actNewUser->setEnabled(false);
            actDisconnect->setEnabled(false);
            actStartSimulation->setEnabled(false);
            actStopSimulation->setEnabled(true);
            actOneStep->setEnabled(true);
            actWholeNet->setEnabled(true);
            
            mode = E_moveItem;
            foreach(QAbstractButton *button, netButtons->buttons()) {
                button->setEnabled(false);
            }
            simulationRunning = true;
            
            break;
            
        case E_stopSimulation:
            actNew->setEnabled(true);
            actOpen->setEnabled(true);
            actExit->setEnabled(true);
            actStopSimulation->setEnabled(false);
            actOneStep->setEnabled(false);
            actWholeNet->setEnabled(false);
            updateInterfaceMode(E_newTab);
            updateInterfaceMode(E_itemLostFocus);
            updateInterfaceMode(E_connected);
            //getCurrentPetriNetScene()->views().first()->setInteractive(true);
            foreach(QAbstractButton *button, netButtons->buttons()) {
                button->setEnabled(true);
            }
            simulationRunning = false;
            break;
    }
}

// Ziskani pouze jmena souboru bez absolutni cesty - reseni nezavisle na platforme
QString AppWindow::getFileNameExcludingPath(QString &name) {
    QFileInfo pathInfo(name);
    QString result(pathInfo.fileName());
    return result;
}

// Slot - tvorba nove zalozky -> nova scena.
void AppWindow::sNew() {
    PetriNetScene *newScene;
    QGraphicsView *newView;
    
    tabsOpendSoFar++;
    
    // Tvorba sceny a pohledu. Rodicem sceny se zrejme stane AppWindow
    // a rodicem pohledu se stane QTabWidget
    newScene = new PetriNetScene(this, contextMenuItem);   
    newScene->setSceneRect(QRect(0, 0, 3000, 3000));  
    connect(newScene, SIGNAL(selectionChanged()), this, SLOT(sSelectionChanged()));
    newView = new QGraphicsView(newScene);
    //newView->setMouseTracking(false);
    
    QString name = tr("new ") + QString("%1").arg(tabsOpendSoFar);
    
    tabs->addTab(newView, name);
    newScene->setFileName(name);
    
    scenes.push_back(newScene);
    views.push_back(newView);
    
    // zobrazeni nove karty
    tabs->setCurrentIndex(tabs->count()-1);
    
    updateInterfaceMode(E_newTab);    
}

// Slot -> Kontrola zmeny vyberu objektu ve scene. V zavislosti na 
// vybranem/nevybranem objektu se zpristupnuje menu Item.
void AppWindow::sSelectionChanged() {
    
    if(tabs->currentIndex() >= 0 && getCurrentPetriNetScene()->selectedItems().length() > 0)
        updateInterfaceMode(E_itemSelected);
    else
        updateInterfaceMode(E_itemLostFocus);
}

// Otevre ulozeny dokument
void AppWindow::sOpen() {
    
    int maxID;
    // Neni otevrena zadna karta, otevreni nove
    if(tabs->currentIndex() < 0) {
        sNew();
    }
    
    PetriNetScene *curTab = getCurrentPetriNetScene();
    
    QString name = QFileDialog::getOpenFileName(this, tr("Open File"), "../../examples",
                                                tr("Petri Net Project (*.xml)"));
    
    if (!name.isEmpty()) {
        QFile f(name);
        if (!f.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(this, tr("Petri Net Editor"),
                                 tr("Cannot open file %1 for reading:\n%2.")
                                 .arg(name)
                                 .arg(f.errorString()));
            return;
        }
        xml->clear();
        while (!f.atEnd()) {
                xml->data.append(f.readLine());
        }
        xml->data = xml->data.section('>',1);

        // Uzivatel se snazi otevrit soubor v jiz modifikovane karte
        //if(!(curTab->getFileName().isEmpty()) || curTab->getSceneChanged()) {
        if(curTab->getSaved() || curTab->getSceneChanged()) {
            sNew();
            tabs->setCurrentIndex(tabs->count()-1);
            curTab = getCurrentPetriNetScene();
        }
        
        // parsovani vstupniho XML souboru a ulozeni site do programu

        xml->createNetCore(getCurrentPetriNetScene()->getListsPtr(), &maxID);
        getCurrentPetriNetScene()->setGeneratedID(maxID);
        // vytvoreni grafickeho obalu - vykresleni site
        getCurrentPetriNetScene()->drawPetriNet(contextMenuItem);
        
        // tenhe radek deli program od segfaulty pri double open nad stejnou kartou :-)
        curTab->setSceneChanged(true);
        
        QString justName = getFileNameExcludingPath(name);
        curTab->setFileName(justName);
        tabs->setTabText(tabs->currentIndex(),curTab->getFileName());
        
        statusBar()->showMessage(tr("File %1 loaded").arg(justName), 3000);   
    } 
    return;
}

/* Slot - otvre a vykresli sit ze serveru. Nejprve zobrazi seznam dostupnych siti
   na serveru podle filtru, uzivatel vybere jednu, pak server zasle konkretni sit */
void AppWindow::sOpenFromserver() {
    bool ok;
    QString text = QInputDialog::getText(this, tr("Open Petri Nets from server"),
                                         tr("Keyword (if left blank all stored files are shown):"), 
                                         QLineEdit::Normal,
                                         "", &ok);
    if(ok) {
        xml->clear();
        xml->add_req_showNets(text);
        write(xml->data);        
    }
}

// Slot - zavre kartu.
int AppWindow::sClose() {
    int index;  // index zavirane karty
    int answer = QMessageBox::Cancel;
    
    // zvareni karty a uvolneni pameti pro scenu a pohled.
    if((index = tabs->currentIndex()) >= 0) {
        // overeni, zda byla scena pred uzavreim ulozena
        if(getCurrentPetriNetScene()->getSceneChanged()) {
            answer = QMessageBox::question(this,
                         tr("File was not saved"),
                         tr("Save file?"),
                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            
            if (answer == QMessageBox::Yes)
                if(!sSave()) return QMessageBox::Cancel;
        // scena nebyla modifikovana, muze se zavrit
        } else { //if(getCurrentPetriNetScene()->getFileName().isEmpty()) {
            answer = QMessageBox::No;
        }
        
        // POZOR: tlacitko Cancel je zde proto, ze pokud tam bylo jen Yes/No a uzivatel 
        // v dialogovem okne klikl na krizek, bralo se to jako deafultni tlacitko (No) 
        // a tim padem se karta bezostysne zavrela.
        if(answer != QMessageBox::Cancel) {
            tabs->removeTab(index);
        
            delete scenes[index];
            delete views[index];
        
            scenes.remove(index);
            views.remove(index);
            
            updateInterfaceMode(E_itemLostFocus);
            if(tabs->currentIndex() < 0) 
                updateInterfaceMode(E_noTab);
        }
    }
    
    return answer;
}

// Ulozeni PEtriho site do souboru
bool AppWindow::sSave() {
    // soubor jeste nebyl ulozen, vola se SaveAs
    //if(getCurrentPetriNetScene()->getFileName().isEmpty()) {
    if(getCurrentPetriNetScene()->getSaved() == false) {
        return sSaveAs();
    } 
    // soubor uz byl ulozen, tak se jen ulozi
    else {
        return saveFile(getCurrentPetriNetScene()->getFileName());
    }
}

/* Ulozeni pod novym nazvem */
bool AppWindow::sSaveAs() {
    
    // dialogove okno pro vyber mista ulozeni a nazvu souboru
    QString name = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                getCurrentPetriNetScene()->getFileName(),
                                                tr("Petri Net Project (*.xml)"));
    // volani funkce, ktera soubor fyzicky ulozi
    return name.isEmpty() ? false : saveFile(name);    
}

// funkce ulozi soubor fyzicky na disk
bool AppWindow::saveFile(QString &name) {
    QFile f(name);
    
    // otevreni pro zapis a v textovem rezimu (\n se reprezentuje do CR/LF/CRLF podle
    // systemu, na nemz program bezi)
    if (!f.open(QFile::WriteOnly | QFile::Text)) {
        // Otevre varovne okno
        QMessageBox::warning(this, tr("Petri Net Editor"),
                             tr("Cannot open file %1 for writing:\n%2")
                             .arg(name)
                             .arg(f.errorString()));
        return false;
    }
    getCurrentPetriNetScene()->setSaved(true);

    QTextStream out(&f);

    // vytvori xml string
    myXML xml;    

    // nacteni sceny a ulozeni do xml
    xml.createSceneXML(getCurrentPetriNetScene()->getListsPtr());

    // ulozeni do souboru
    out << xml.data;
    
    // Ulozeni noveho jmena souboru
    QRegExp reg("(.*)\\.xml$");
    if (reg.exactMatch(name)) {
          name=reg.cap(1);
    }
    getCurrentPetriNetScene()->setFileName(name);
    // Na 3 s zobrazit status zpravu
    statusBar()->showMessage(tr("File saved"), 3000);
    // Shozeni priznaku, z byla scena modifikovana
    getCurrentPetriNetScene()->setSceneChanged(false);
    // zjisteni jen jmena souboru bez absolutni cesty (reseni nezavisle na platforme - lomitka)
    QString justName = getFileNameExcludingPath(name);
    // Prenastaveni jmena karty
    tabs->setTabText(tabs->currentIndex(), justName);
    
    return true;
}

// Slot - ulozi sit na server, pta se na jmeno, nabidne jmeno lokalni site.
void AppWindow::sSaveToServer() {
    QString justName = getFileNameExcludingPath(getCurrentPetriNetScene()->getFileName());
    
    QRegExp reg("(.*)\\.xml$");
    if (reg.exactMatch(justName)) {
          justName=reg.cap(1);
    }
    bool ok;
    QString text = QInputDialog::getText(this, tr("New Petri net name"),
                                         tr("Name(without file name extension):"), QLineEdit::Normal,
                                         justName, &ok);
    if(ok) {
        if(text.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please type a name"));
        } else {  
            QString description = QInputDialog::getText(this, tr("Description of new net"),
                                                 tr("Description:"), QLineEdit::Normal,
                                                        "", &ok);
            
          // nastaveni jmena souboru na serveru
            getCurrentPetriNetScene()->setNameServer(text);
        // vytvoreni zpravy
            xml->clear();
            xml->add_req_save(getCurrentPetriNetScene()->getListsPtr(),text,description);
        // poslani zpravy
           write(xml->data);
        }
    }
}

// Slot - Zobrazi informace o siti ze serveru.
void AppWindow::sInfo() {
    
    QMessageBox::information(this, tr("Information about the net"),getCurrentPetriNetScene()->getInfo());
    
}

// Slot - Ukonci program
void AppWindow::sExit() {
    int tabsCount = tabs->count();
    
    std::cout << "pocet karet: " << tabsCount << std::endl;
    
    if(tabsCount > 0) {
        for(int i = 0; i < tabsCount; i++) {
            tabs->setCurrentIndex(0); 
            if(sClose() == QMessageBox::Cancel) return;
        }
    }    
    close();
}

// Slot - Navazani spojeni se serverem
void AppWindow::sConnect() {
    dialogConnect->setImplicitFields();
    dialogConnect->show();
    
    // byly zadany pozadovane udaje
    if(dialogConnect->exec() == 1) {
        // dopredna zmena modu, aby uzivatel znovu neklikl na connect nebo new user
        // v pripade neuspesneho pripojeni se vola slot sDisconnected, ktery rozhrani
        // zmeni zpratky na E_disconnected
        updateInterfaceMode(E_connected);
        setSession();

        // pripojnei k serveru
        myConnect();               
        // pripojeni k serveru, pokud se nezdari, navrat z funkce
        if(socket->state() != QAbstractSocket::ConnectedState) return;

        // vytvoreni noveho xml
        xml->clear();
        xml->add_req_authen(session->getLogin(),session->getPassword());
        // poslani zpravy
        write(xml->data);
    }
}

// Slot - Odpojeni od serveru
void AppWindow::sDisconnect() {
    // Ukonceni spojeni
    // Zmena rozhrani se vola ve slotu sDisconnected
    socket->disconnectFromHost();    
}

void AppWindow::sNewUser() {
    dialogConnect->setImplicitFields();
    dialogConnect->show();
    
    // byly zadany pozadovane udaje
    if(dialogConnect->exec() == 1) {
        updateInterfaceMode(E_connected);
        setSession();
        
        // pripojeni k serveru, pokud se nezdari, navrat z funkce
        myConnect();
        if(socket->state() != QAbstractSocket::ConnectedState) return;        

        // vytvoreni noveho xml
        xml->clear();
        xml->add_req_newUser(session->getLogin(),session->getPassword());
        // poslani zpravy
        write(xml->data);        
    }
}

// Slot - zahajeni simulace
void AppWindow::sStartSimul() {

    // nesmi byt zadna zmena
    if (!getCurrentPetriNetScene()->getNetFromServerChanged()) {
        // zkontroluje zda je potreba ulozit na server
            if (!getCurrentPetriNetScene()->getVersion().isEmpty()) {

                xml->clear();
                xml->add_req_startSim(getCurrentPetriNetScene()->getNameServer(),
                                      getCurrentPetriNetScene()->getVersion());
                write(xml->data);
                return;
            }
    }
    // nejprve musime ulozit na server
    QMessageBox::information(this, tr("Info"),"Your net isn't saved on server,you must net first save on the server.");
    flagSim=true;
    sSaveToServer();
    return;
}

// Slot - ukonceni simulace
void AppWindow::sStopSimul() {
    
    // pozadavek na ukonceni simulace
    xml->clear();
    xml->add_req_endSim();
    write(xml->data);
    // zmena uziv. rozhrani
    updateInterfaceMode(E_stopSimulation);
    getCurrentPetriNetScene()->resetAllDoableTransitions();
    getCurrentPetriNetScene()->setNetFromServerChanged();
}

// Slot - jeden krok simulace
void AppWindow::sOneStep() {
    int id;
    PetriNetTransitionGUI *trans;

    QList<QGraphicsItem *> selected =  getCurrentPetriNetScene()->selectedItems();
    if(selected.length() == 1) {
        if(selected.first()->type() == PetriNetTransitionGUI::Type){
            trans = qgraphicsitem_cast<PetriNetTransitionGUI *>(selected.first());
            if(trans->getTransCorePtr()->isDoable()) {
                id = trans->getTransCorePtr()->getID();
                QString strId;
                strId = strId.number(id);
                xml->clear();
                xml->add_req_stepSim(strId);
                write(xml->data);
                return;
            }
        }
    }
    QMessageBox::critical(this, tr("Error"),"You must choose ONE colored transition!");
}

// Slot - simulace cele site najednou
void AppWindow::sWholeNet() {
    xml->clear();
    bool ok,ok2;
    QString max = QInputDialog::getText(this, tr("Whole simulation of petri net"),
                                         tr("Enter max. count of steps:"), QLineEdit::Normal,"10",&ok);

    QRegExp reg("\\s*([0-9])+\\s*");

    if(ok)  {
        if (reg.exactMatch(max)) {
            max.toInt(&ok2);
            if (ok2) {
                xml->add_req_fullSim(max);
                write(xml->data);
                return;
            }
        }
        QMessageBox::critical(this, tr("Error"),"Enter a number");
        return;

    }
}

void AppWindow::sHelp() {
    QMessageBox::information(this, tr("Help"), \
                             "Program slouzi k modelovani a simulovani Petriho siti.\n\nVytvoreni vlastni site: \n" \
                             "Sit obsahuje 3 prvky: mista,prechody a hrany. Tyto prvky si muzete vytvorit pomoci " \
                             "tlacitek nalevo programu. Pomoci tlacitka move muzete ruzne presouvat a oznacovat prvky. " \
                             "Kliknutim pravym tlacitkem na prvky lze upravovat jejich atributy. U mist se doplnuji " \
                             "cisla oddelena carkou, k hranam cislo nebo promenna. Do prechodu se vkladaji dve informace: " \
                             "podminka pro provedeni prechodu a akce, ktera se provede po provedeni odpalu. Podminek muze " \
                             "byt vice a jsou oddeleny znaky &&. Relacni operatory jsou: >,<,==,<=,>=,!=. V akci lze do "  \
                             "vybrane promenne vlozit promenne nebo cisla. Tyto slozky se daji scitat a odcitat." \
                             "\n\nAutentizace:\nK simulovani siti je potreba byt pripojen k serveru. V horni liste" \
                             " kliknete na server a pote se muzete tlacitky New user a Connect registrovat nebo pripojit.\n\n" \
                             "Simulace:\nTed, kdyz jste prihlaseni muzete zacit simulovat sit tlacitkem Simulation->Start." \
                             " Simulaci lze kdykoli ukoncit tlacitkem stop. Simulace ma 2 druhy: krokova a cela. Krokovat muzete"\
                             " po stisknuti tlacitka step. Tento typ je interaktivni. Pred kazdym krokem si muzete vybrat,ktery z proveditelnych"\
                             " prechodu se ma vykonat jednoduchym kliknutim na obarveny prechod. Pokud zadny prechod jiz nelze simulovat,"\
                             "vyskoci varovani a simulace se ukonci.\nU cele simulace si muzete zvolit, kolik se ma provest maximalne kroku.");
}

void AppWindow::sDocumentation() {
    bool opened = false;
    
    QFileInfo doc("../../doc/html/index.html");
    QString str=doc.absoluteFilePath();
    str.prepend("file:///");
    
    opened = QDesktopServices::openUrl(QUrl(str,QUrl::TolerantMode));
    
    if(!opened) {
        QFileInfo doc1("../../../doc/html/index.html");
        str=doc1.absoluteFilePath();
        str.prepend("file:///");
        opened = QDesktopServices::openUrl(QUrl(str,QUrl::TolerantMode));
    }
    
    if(!opened) {
        QFileInfo doc2("../../../../doc/html/index.html");
        str=doc2.absoluteFilePath();
        str.prepend("file:///");
        opened = QDesktopServices::openUrl(QUrl(str,QUrl::TolerantMode));
    }    
    
    if(!opened) {
        QMessageBox::warning(this, tr("Documentation"),
                             tr("Cannot find documentation."));
    }
}

void AppWindow::sAbout() {
    QMessageBox::information(this, tr("About"),"Program: pn2012\nAuthors: Jan bednarik and Petr Konvalinka\n" \
                             "Creating date: 6.5.2012\nInfo: pn2012 is project for course ICP" \
                             "\nDescription: The program is used to simulate petri net");
}

// Smaze vuybrany objekt. Pokud je to misto nebo prechod, tak i jejich hrany.
void AppWindow::sDelete() {
    // nechutny radek s pretypovanim a volanim metod vraci ukazatel na scenu dane zalozky.
    QGraphicsScene *scene = static_cast<QGraphicsView *>(tabs->currentWidget())->scene();
    PetriNetPlaceGUI *place;
    PetriNetTransitionGUI *trans;
    PetriNetArcGUI *arc;
    
    if(simulationRunning)
        return;
        
    
    // Smazani nejprve vsech vybranych hran
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        // Maze se hrana            
        if(item->type() == PetriNetArcGUI::Type) {
            std::cout << "mazu hranu" << std::endl;
            arc = qgraphicsitem_cast<PetriNetArcGUI *>(item);            
            scene->removeItem(arc);
            scene->removeItem(arc->getTextPointer());
            arc->deleteConnection(E_From);
            arc->deleteConnection(E_To);
            delete item;
        }
    }
    
    // Nyni uz jsou v seznamu pouze mista a prechody
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        // Maze se misto
        if(item->type() == PetriNetPlaceGUI::Type) {
            std::cout << "mazu misto" << std::endl;
            place = qgraphicsitem_cast<PetriNetPlaceGUI *>(item);           
            scene->removeItem(place);
            scene->removeItem(place->getTextPointer());
            place->deleteArcs(scene);
            //delete place->getTextPointer();
            delete item;
        } else
        // Maze se prechod
        if(item->type() == PetriNetTransitionGUI::Type) {
            std::cout << "mazu prechod" << std::endl;
            trans = qgraphicsitem_cast<PetriNetTransitionGUI *>(item);           
            scene->removeItem(trans);
            scene->removeItem(trans->getLinePtr());
            scene->removeItem(trans->getTextGuardPtr());
            scene->removeItem(trans->getTextActionPtr());
            trans->deleteArcs(scene);
            delete item;
        } 
    }
    // zmena uzivatelskeho rozhrani
    updateInterfaceMode(E_itemLostFocus);
    
    // scena se zmenila
    getCurrentPetriNetScene()->setSceneChanged(true);  
}

/* Slot - otevre dialogove okno pro misto, prechod nebo hranu */
void AppWindow::sProperties() {
    // nechutny radek s pretypovanim a volanim metod vraci ukazatel na scenu dane zalozky.
    QGraphicsScene *scene = static_cast<QGraphicsView *>(tabs->currentWidget())->scene();
    
    PetriNetPlaceGUI *place;
    PetriNetTransitionGUI *trans;
    PetriNetArcGUI *arc;
    
    if(simulationRunning)
        return;
    
    // ziskani seznamu vybranych objektu. V tuto chvili by mel byt 
    // v kazdem pripade vybran prave jeden!
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    assert(selectedItems.length() == 1);
    
    // Otevreni dialogoveho okna pro...
    switch(selectedItems.first()->type()) {
        // misto
        case PetriNetPlaceGUI::Type:
            place = qgraphicsitem_cast<PetriNetPlaceGUI *>(selectedItems.first());
            std::cout << "Properties pro MISTO" << std::endl;
            dialogPlace->setLineEdit(place->getTokensString());
            dialogPlace->show();
            
            if (dialogPlace->exec() == 1) {
                place->setTokens(dialogPlace->getUserInput());
                std::cout << "Uzivatel zadal: " << dialogPlace->getUserInput().toStdString() << std::endl;
                // ulozi zadany string do textoveho pole mista
                place->setTokensText();                
                // Prepocita velikost textoveho pole a mista a prekresli
                place->updateSize();
                // scena se zmenila
                getCurrentPetriNetScene()->setSceneChanged(true);  
            }
            break;
        
        // prechod
        case PetriNetTransitionGUI::Type:
            trans = qgraphicsitem_cast<PetriNetTransitionGUI *>(selectedItems.first());    
            std::cout << "Properties pro PRECHOD" << std::endl;
            dialogTransition->setLineEdit(trans->getGuardString());
            dialogTransition->setActionEdit(trans->getActionString());
            dialogTransition->show();
            
            if (dialogTransition->exec() == 1) {
                trans->setGuardString(dialogTransition->getUserInput());
                trans->setActionString(dialogTransition->getAction());
                std::cout << "Uzivatel zadal: " << dialogTransition->getUserInput().toStdString() << std::endl;
                std::cout << "Uzivatel zadal: " << dialogTransition->getAction().toStdString() << std::endl;
                // ulozi zadany string do textoveho pole guard a action
                trans->setGuardText();                
                trans->setActionText();
                // Prepocita velikost textoveho pole guard a prechodu a prekresli
                trans->updateSize();
                trans->updateLinePos();
                
                // scena se zmenila
                getCurrentPetriNetScene()->setSceneChanged(true);  
            }
            break;
        
        // hranu            
        case PetriNetArcGUI::Type:
            arc = qgraphicsitem_cast<PetriNetArcGUI *>(selectedItems.first());    
            std::cout << "Properties pro HRANU" << std::endl;
            dialogArc->setLineEdit(arc->getVarOrConstString());
            dialogArc->show();
            
            if(dialogArc->exec() == 1) {
                arc->setVarOrConstString(dialogArc->getUserInput());
                std::cout << "Uzivatel zadal: " << arc->getVarOrConstString().toStdString() << std::endl;
                arc->setVarOrConstText();
                // scena se zmenila
                getCurrentPetriNetScene()->setSceneChanged(true);  
            }
            break;
    
        default: std::cout << "Tento pripad by nikdy nemel nastat" << std::endl; 
            break;
    }
}

// nastaveni noveho modu programu podle stisknuteho tlacitka z nastrojove listy
void AppWindow::sNetButtonsClicked(int b) {
    // TODO smazat
    std::cout << "Stisknuto tlacitko c.: " << b << std::endl;
    
    mode = E_PetriNetMode(b);
}

// Naplni clensky objekt tridy Session informacemi o sezeni.
void AppWindow::setSession() {
    session->setLogin(dialogConnect->getLoginString());
    session->setPassword(dialogConnect->getPasswordString());
    session->setServer(dialogConnect->getServerString());
    session->setPort(dialogConnect->getPortString());
}

// Tvorba akci, ktere vyvolavaji polozky menu a tlacitka toolBaru.
void AppWindow::createActions() {
    //New net
    actNew = new QAction(tr("&New"), this);
    actNew->setShortcuts(QKeySequence::New);
    actNew->setStatusTip(tr("Create a new tab"));
    connect(actNew, SIGNAL(triggered()), this, SLOT(sNew()));
    
    // Open net
    actOpen = new QAction(tr("&Open"), this);
    actOpen->setShortcuts(QKeySequence::Open);
    actOpen->setStatusTip(tr("Open a net saved locally"));
    connect(actOpen, SIGNAL(triggered()), this, SLOT(sOpen()));
    
    // Open from server
    actOpenFromServer = new QAction(tr("Open from server"), this);
    actOpenFromServer->setStatusTip(tr("Open a net saved on server"));
    connect(actOpenFromServer, SIGNAL(triggered()), this, SLOT(sOpenFromserver()));
    
    // Close
    actClose = new QAction(tr("&Close"), this);
    actClose->setShortcuts(QKeySequence::Close);
    actClose->setStatusTip(tr("Close net"));
    connect(actClose, SIGNAL(triggered()), this, SLOT(sClose()));
    
    // Save
    actSave = new QAction(tr("&Save"), this);
    actSave->setShortcuts(QKeySequence::Save);
    actSave->setStatusTip(tr("Save net locally"));
    connect(actSave, SIGNAL(triggered()), this, SLOT(sSave()));
    
    // SaveAs
    actSaveAs = new QAction(tr("Save as..."), this);
    actSaveAs->setShortcuts(QKeySequence::SaveAs);
    actSaveAs->setStatusTip(tr("Save net as... locally"));
    connect(actSaveAs, SIGNAL(triggered()), this, SLOT(sSaveAs()));
    
    // Save to server
    actSaveToServer = new QAction(tr("Save to server"), this);
    actSaveToServer->setStatusTip(tr("Save net to server"));
    connect(actSaveToServer, SIGNAL(triggered()), this, SLOT(sSaveToServer()));
    
    // Info
    actInfo = new QAction(tr("Info"), this);
    actInfo->setStatusTip(tr("Show net's information"));
    connect(actInfo, SIGNAL(triggered()), this, SLOT(sInfo()));
    
    // Exit
    actExit = new QAction(tr("Exit"), this);
    actExit->setStatusTip(tr("Exit program"));
    // close() je public slot tridy qWidget
    connect(actExit, SIGNAL(triggered()), this, SLOT(sExit()));
    
    // Connect
    actConnect = new QAction(tr("Connect"), this);
    actConnect->setStatusTip(tr("Connect to server"));
    connect(actConnect, SIGNAL(triggered()), this, SLOT(sConnect()));
    
    // Disconnect
    actDisconnect = new QAction(tr("Disconnect"), this);
    actDisconnect->setStatusTip(tr("Disconnect from server"));
    connect(actDisconnect, SIGNAL(triggered()), this, SLOT(sDisconnect()));
    
    // New user
    actNewUser = new QAction(tr("New user"), this);
    actNewUser->setStatusTip(tr("New user registration"));
    connect(actNewUser, SIGNAL(triggered()), this, SLOT(sNewUser()));
    
    // Start simulace
    actStartSimulation = new QAction(tr("Start"), this);
    actStartSimulation->setStatusTip(tr("Start simulation"));
    connect(actStartSimulation, SIGNAL(triggered()), this, SLOT(sStartSimul()));
    
    // Ukonceni simulace
    actStopSimulation = new QAction(tr("Stop"), this);
    actStopSimulation->setStatusTip(tr("Stop simulation"));
    connect(actStopSimulation, SIGNAL(triggered()), this, SLOT(sStopSimul()));
    
    // One step
    actOneStep = new QAction(tr("One step"), this);
    actOneStep->setStatusTip(tr("Do one step of the simulation"));
    connect(actOneStep, SIGNAL(triggered()), this, SLOT(sOneStep()));
    
    // Whole net
    actWholeNet = new QAction(tr("Whole net"), this);
    actWholeNet->setStatusTip(tr("Run simulation of the whole net"));
    connect(actWholeNet, SIGNAL(triggered()), this, SLOT(sWholeNet()));
    
    // Help
    actHelp = new QAction(tr("Help"), this);
    actHelp->setShortcuts(QKeySequence::HelpContents);
    actHelp->setStatusTip(tr("Show help"));
    connect(actHelp, SIGNAL(triggered()), this, SLOT(sHelp()));
    
    // Documentation
    actDocumentation = new QAction(tr("Documentation"), this);
    actDocumentation->setStatusTip(tr("Show documentation"));
    connect(actDocumentation, SIGNAL(triggered()), this, SLOT(sDocumentation()));
    
    // About
    actAbout = new QAction(tr("About"), this);
    actAbout->setStatusTip(tr("About"));
    connect(actAbout, SIGNAL(triggered()), this, SLOT(sAbout()));
    
    // Kontextove menu Item
    // Delete
    actDelete = new QAction(tr("&Delete"), this);
    actDelete->setShortcuts(QKeySequence::Delete);
    actDelete->setStatusTip(tr("Delete item"));
    connect(actDelete, SIGNAL(triggered()), this, SLOT(sDelete()));
    
    // Properties
    actProperties = new QAction(tr("&Properties"), this);
    actProperties->setShortcuts(QKeySequence::Preferences);
    actProperties->setStatusTip(tr("Edit properties"));
    connect(actProperties, SIGNAL(triggered()), this, SLOT(sProperties()));
}

// Tvorba polozek menu.
void AppWindow::createMenus() {
    // File
    menuFile = menuBar()->addMenu(tr("&File"));
    menuFile->addAction(actNew);
    menuFile->addAction(actOpen);
    menuFile->addAction(actOpenFromServer);
    menuFile->addAction(actClose);
        menuFile->addSeparator();
    menuFile->addAction(actSave);
    menuFile->addAction(actSaveAs);
    menuFile->addAction(actSaveToServer);
        menuFile->addSeparator();
    menuFile->addAction(actInfo);
        menuFile->addSeparator();
    menuFile->addAction(actExit);    
    
    // kontextove menu Item
    contextMenuItem  = menuBar()->addMenu(tr("&Item"));
    contextMenuItem ->addAction(actDelete);
    contextMenuItem ->addAction(actProperties);
    
    // Server
    menuServer = menuBar()->addMenu(tr("&Server"));
    menuServer->addAction(actNewUser);
    menuServer->addAction(actConnect);
    menuServer->addAction(actDisconnect);
    
    // Simulation
    menuSimulation = menuBar()->addMenu(tr("Si&mulation"));
    menuSimulation->addAction(actStartSimulation);
    menuSimulation->addAction(actStopSimulation);
        menuSimulation->addSeparator();
    menuSimulation->addAction(actOneStep);
    menuSimulation->addAction(actWholeNet);
        
    menuBar()->addSeparator();
    
    // Help
    menuHelp = menuBar()->addMenu(tr("&Help"));
    menuHelp->addAction(actHelp);
    menuHelp->addAction(actDocumentation);
        menuFile->addSeparator();
    menuHelp->addAction(actAbout);
}

// Tvorba nastrojove listy.
void AppWindow::createToolbars() {
    toolbarNet = new QToolBar(tr("Net"));
    addToolBar(Qt::LeftToolBarArea, toolbarNet);
    
    // skupina tlacitek pro kreslni/editaci site
    netButtons = new QButtonGroup();
    netButtons->setExclusive(true); // Sitsknuto muze byt pouze jedno tlacitko soucasne
    connect(netButtons, SIGNAL(buttonClicked(int)), this, SLOT(sNetButtonsClicked(int)));
    
    QVBoxLayout *loginLayout  = new QVBoxLayout;
    // tlacitko pro sipku-pohyb
    loginLayout ->addWidget(createNetButton(tr("Move"), 
                                      tr(":/images/movearrow.png"), 
                                      E_moveItem)); 
    // tlacitko pro misto
    loginLayout ->addWidget(createNetButton(tr("Place"), 
                                      tr(":/images/place.png"), 
                                      E_insPlace)); 
    // tlacitko pro prechod
    loginLayout ->addWidget(createNetButton(tr("Transition"), 
                                      tr(":/images/trans.png"), 
                                      E_insTrans)); 
    // tlacitko pro hranu
    loginLayout ->addWidget(createNetButton(tr("Arc"), 
                                      tr(":/images/arc.png"), 
                                      E_insArc));     
        
    QWidget *wNetButtons = new QWidget();
    wNetButtons->setLayout(loginLayout );
    
    toolbarNet->addWidget(wNetButtons);
}

// Tvorba tlacitka nastrojove listy
QWidget *AppWindow::createNetButton(const QString &label, 
                                    const QString &iconName,
                                    E_PetriNetMode type) {
    QIcon icon(iconName);
    
    QToolButton *button = new QToolButton;
    button->setIcon(icon);
    button->setIconSize(QSize(40, 40));
    button->setCheckable(true);
    button->setStatusTip(type == E_moveItem ? ("Move items") : 
                                                             (QString("Create new ") += label));
    if(type == E_moveItem) button->setChecked(true);
    netButtons->addButton(button, int(type));
    
    QVBoxLayout *loginLayout  = new QVBoxLayout;
    loginLayout ->addWidget(button, 0, Qt::AlignHCenter);
    loginLayout ->addWidget(new QLabel(label), 0, Qt::AlignHCenter);
    
    QWidget *widget = new QWidget;
    widget->setLayout(loginLayout );
    
    return widget;
}

// Tvorba stavoveho radku.
void AppWindow::createStatusbar() {
    statusBar()->showMessage(tr("Ready"));
}

// Pripojeni k serveru
void AppWindow::myConnect()
{
    QHostAddress addr;
    addr=addr.LocalHost;   

    int port=session->getPort().toInt();
    socket->connectToHost((session->getServer()),port);
    if(!socket->waitForConnected(2000))       
        emit socket->error();
    
}

// klient navazal spojeni se serverem
void AppWindow::sConnected()
{

}
// zavreni spojeni
void AppWindow::sDisconnected()
{
    updateInterfaceMode(E_disconneced);
    statusBar()->showMessage(tr("Disconnected"), 
                             3000);
}

// pokud prijde nejaka odpoved
void AppWindow::sReply()
{
    // nacteni zpravy do xml
    xml->data=socket->readAll();

    //vrati typ odpovedi
    switch (xml->getReply()) {

        //odpoved na autentizaci
        case T_rep_authen: {
            if (xml->getResult()) {
                statusBar()->showMessage(tr("Conected user %1 succesfully")
                                         .arg(dialogConnect->getLoginString()),3000);
            }
            else {
                socket->close();
                QMessageBox::critical(this, tr("Error"),"Wrong login or password!");
            }
            break;
        }

        // odpoved na pridani noveho uzivatele
        case T_rep_new: {
            if (xml->getResult()) {
              statusBar()->showMessage(tr("Created user profile and connected user %1 successfully.")
                                         .arg(dialogConnect->getLoginString()), 5000);
              QMessageBox::information(this, tr("Accept"),"New user has been added successfully");
            }
            else {
                socket->close();

                QMessageBox::critical(this, tr("Error"),"User is already exists or Unknown Error");
            }
            break;
        }

        // odpoved na ulozeni site
        case T_rep_save: {
            QString version;
            if (xml->get_rep_save(&version)) {

                 QMessageBox::information(this, tr("Success"),"Your Petri net has been saved successfully");
                 getCurrentPetriNetScene()->setVersion(version);
                 getCurrentPetriNetScene()->resetNetfromServerChanged();
                 // pokud byl predtim zadan pozadavek na simulaci
                 if (flagSim) {
                     flagSim=false;
                     xml->clear();
                     xml->add_req_startSim(getCurrentPetriNetScene()->getNameServer(),
                                           getCurrentPetriNetScene()->getVersion());
                     write(xml->data);
                 }
            }
            else {
                socket->close();
                flagSim=false;
                QMessageBox::critical(this, tr("Unknown error"),"Unknown error,failure save net");
            }
             break;
        }

        // odpoved na nacteni site
        case T_rep_load: {
            int maxID;
            QString simLog,version; // log a verze souboru
            if (xml->get_rep_load(&simLog,&version)) {// vyjmeme log, ktery zobrazime
               xml->delete_message(); // v datech mame pouze scenu

               PetriNetScene *curTab = getCurrentPetriNetScene();
               if(curTab->getSaved() || curTab->getSceneChanged()) {
                    sNew();
                    tabs->setCurrentIndex(tabs->count()-1);
                    curTab = getCurrentPetriNetScene();
               }
               xml->createNetCore(getCurrentPetriNetScene()->getListsPtr(), &maxID);  // vytvoreni nove sceny v nove zalozce
               getCurrentPetriNetScene()->setGeneratedID(maxID);
               getCurrentPetriNetScene()->drawPetriNet(contextMenuItem);
               curTab->setSceneChanged(true);
               curTab->resetNetfromServerChanged();
               QString justName = getFileNameExcludingPath(myName);
               curTab->setInfo(simLog);
               curTab->setFileName(justName);
               curTab->setVersion(version);
               curTab->setNameServer(myName);
               tabs->setTabText(tabs->currentIndex(),curTab->getFileName());
               statusBar()->showMessage(tr("File %1 loaded").arg(justName), 3000);
            }
            else {
                QMessageBox::critical(this, tr("Unknown error"),"Unknown error,failure open net");
            }
            break;
        }

        // odpoved na zobrazeni ulozenych siti
        case T_rep_showNets: {
            QString str,version;
            if (xml->get_rep_showNets(&str)) {// vyjmeme retezec s jmeny souboru
                xml->clear();                                
                if (choose(str,&myName,&version)) { // vybere jmeno a verzi,co si zada uzivatel

                    xml->add_req_load(myName,version);
                    write(xml->data);
                }
            }
            else {
                 QMessageBox::critical(this, tr("Unknown error"),"UPSSS,Unknown error");
            }
            break;
        }

        // odpoved na start simulace
        case T_rep_startSim: {
            int maxID;
            if (xml->getResult()) {
                QMessageBox::information(this, tr("Success"),"You can begin simulation");
                xml->delete_message(); // v datech mame pouze scenu

                updateInterfaceMode(E_startSimulation);
                
                // smazani puvnodi site a vykresleni nove
                // obarveni proveditelnych prechodu
                getCurrentPetriNetScene()->removeNet();
                xml->createNetCore(getCurrentPetriNetScene()->getListsPtr(), &maxID);
                getCurrentPetriNetScene()->setGeneratedID(maxID);
                getCurrentPetriNetScene()->drawPetriNet(contextMenuItem);
            }
            else {
                socket->close();
                QMessageBox::critical(this, tr("Error"),"Unknown error by read net");
            }
            break;
        }

        // odpoved na celou simulaci
        case T_rep_fullSim: {
            int maxID;
            xml->delete_message(); // v datech mame pouze scenu
            //DOPLNIT!! konec simulace,potreba jeste vykreslit vysledek
            
            getCurrentPetriNetScene()->removeNet();
            xml->createNetCore(getCurrentPetriNetScene()->getListsPtr(), &maxID);
            getCurrentPetriNetScene()->setGeneratedID(maxID);
            getCurrentPetriNetScene()->drawPetriNet(contextMenuItem);
            
            QMessageBox::information(this, tr("Info"),"Simulation was terminated");
            updateInterfaceMode(E_stopSimulation);
            
            getCurrentPetriNetScene()->setNetFromServerChanged();
            getCurrentPetriNetScene()->resetAllDoableTransitions();
            
            break;
        }

        // odpoved na simulaci jednoho kroku,posle obarvene prechody
        case T_rep_stepSim: {
            int maxID;
            xml->delete_message(); // v datech mame pouze scenu
            //DOPLNIT!! obarveni prechodu podle sceny,ktera je v xml->data
            
            getCurrentPetriNetScene()->removeNet();
            xml->createNetCore(getCurrentPetriNetScene()->getListsPtr(), &maxID);
            getCurrentPetriNetScene()->setGeneratedID(maxID);
            getCurrentPetriNetScene()->drawPetriNet(contextMenuItem);
            break;
        }

        // ukonceni simulace
        case T_rep_endSim: {
            QMessageBox::information(this, tr("Info"),"Simulation was terminated");
            // zmena uziv. rozhrani
            updateInterfaceMode(E_stopSimulation);
            getCurrentPetriNetScene()->setNetFromServerChanged();
            break;
        }
        // neznama chyba, nejspise pri parsovani
        case T_bad_rep: {
            QMessageBox::critical(this, tr("Unknown error"),"UPSSS,Bad message");
            break;
        }
    }

}

// nejaka chyba pri spojeni
void AppWindow::sError(QAbstractSocket::SocketError)
{
    QMessageBox::critical(this, tr("Network error"), socket->errorString());

    this->sDisconnected();
}

// posle socket serveru
void AppWindow::write(QString data) {
    QByteArray pom;
    pom.append(data);
    socket->write(pom);
    socket->flush();
}
// Vrati jmeno a verzi vybrane uzivatelem
bool AppWindow::choose(QString str,QString *name,QString *version) {

    if (str.isEmpty()) {
        QMessageBox::information(this, tr("Info"),"No net found");
        return false;
    }

    QStringList list=str.split(":"); // rozparsovani stringu na seznam jmen a verzi souboru

    QString tmp;
    myGroup *group;
    group = new myGroup();
    for (int i = 0; i < (list.size());i+=2) {
        tmp.append(list.at(i));
        tmp.append(" :: v");  // oddelovac
        tmp.append(list.at(i+1));
        group->add(tmp);
        tmp.clear();
    }
    // otevreni dialogoveho okna
    group->create();
    group->show();

   if (group->exec()) {
       str=group->getname();
       *name = str.section(" :: v",0,0);
       *version = str.section(" :: v",1);      
       return true;
   }
   return false;

}


