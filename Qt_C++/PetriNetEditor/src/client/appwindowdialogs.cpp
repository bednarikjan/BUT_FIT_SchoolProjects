/** @file: appwindowdialogs.cpp
  * @author Petr Konvalinka
  * @author Jan Bednarik
  *
  * Obsahuje tridy pro dialogova okna
  */

#include "appwindowdialogs.h"
#include "../shared/publictypes.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <iostream>

// konstruktor bazove tridy pro dialogova menu
DialogGeneral::DialogGeneral(QWidget *parent)
    : QDialog(parent) {
    label = new QLabel;
    lineEdit = new QLineEdit;

    okButton = new QPushButton(tr("&OK"));
    cancelButton = new QPushButton(tr("&Cancel"));
    userInput = "";

    // layout pro label a pole pro text
    QVBoxLayout *lineEditLayout = new QVBoxLayout;
    lineEditLayout->addWidget(label);
    lineEditLayout->addWidget(lineEdit);
    
    // layout pro tlacitka
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addStretch();
    
    // hlavni layout
    layout = new QVBoxLayout;
    layout->addLayout(lineEditLayout);
    layout->addLayout(buttonsLayout);

    setLayout(layout);
    // Diky teto volbe nebude aktivni hlavni okno, dokud uzivatel
    // nevyresi dialogove okno
    setModal(true);
    setSizeGripEnabled(true);
}

// konstruktor dialogoveho okna pro misto
DialogPlace::DialogPlace(QWidget *parent)
    : DialogGeneral(parent) {
    label->setText(tr("Insert tokens (comma separated numbers)"));
    setFixedHeight(100);
    setWindowTitle(tr("Place properties"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClicked()));
}

void DialogPlace::okButtonClicked() {
    QString text = lineEdit->text();

    // TODO overeni spravnosti zapisu
    
    userInput = text;
    lineEdit->clear();
    //hide();
    accept();
}

DialogTransition::DialogTransition(QWidget *parent)
    : DialogGeneral(parent) {
    
    actionLabel = new QLabel(tr("Insert output action expression"));
    actionEdit = new QLineEdit;
    action = "";
    
    // layout pro akci
    QVBoxLayout *actionLayout = new QVBoxLayout;
    actionLayout->addWidget(actionLabel);
    actionLayout->addWidget(actionEdit);
    
    layout->insertLayout(1, actionLayout);
    
    label->setText(tr("Insert guard expression"));
    setFixedHeight(150);
    setWindowTitle(tr("Transition properties"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClicked()));
}

void DialogTransition::okButtonClicked() {
    QString guardString = lineEdit->text();
    QString actionString = actionEdit->text();

    // TODO overeni spravnosti zapisu
    if (syntax.checkGuardSyntax(&guardString)) {
         if (syntax.checkActionSyntax(&actionString)) {
            userInput = guardString;
            action = actionString;
            lineEdit->clear();
            actionEdit->clear();
            //hide();
            accept();
         }
         else {
             QMessageBox::critical(this, tr("Error"),
                  "You could insert into action only VARIABLES=CONSTANT|VARIABLE +|-");
         }
    }
    else {
        QMessageBox::critical(this, tr("Error"),
                   "You could insert into guard only VARIABLE RELATION VARIABLE|CONSTANT and &&");

    }
}


DialogArc::DialogArc(QWidget *parent)
    : DialogGeneral(parent) {
    
    label->setText(tr("Insert variable or constant"));
    setFixedHeight(100);
    setWindowTitle(tr("Arc properties"));
    connect(okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClicked()));
}

void DialogArc::okButtonClicked() {
    QString text = lineEdit->text();

    // Overeni spravnosti zapisu
    if (syntax.checkArcSyntax(&text)) {
    
        userInput = text;
        lineEdit->clear();
        //hide();
        accept();
    }
    else {
        QMessageBox::critical(this, tr("Error"),"You could insert only constant or variable");
    }
}

/** Dialogove okno pro pripojeni k serveru a prihlaseni uzivatele, nebo registraci 
  * uzivatele. */
DialogConnect::DialogConnect(QWidget *parent)
    : QDialog(parent), 
      server(IMPLICIT_SERVER), port(IMPLICIT_PORT) {
    
    // labely a textova pole pro...
    // login
    loginLabel = new QLabel(tr("login"));
    loginEdit = new QLineEdit;
    // heslo
    passwordLabel = new QLabel(tr("password"));
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    // adresu serveru
    serverLabel = new QLabel(tr("server name or address"));
    serverEdit = new QLineEdit;
    // cislo portu
    portLabel = new QLabel(tr("port"));
    portEdit = new QLineEdit;

    // tlacitka
    okButton = new QPushButton(tr("&OK"));
    cancelButton = new QPushButton(tr("&Cancel"));
    
    // propojeni tlacitek s akcemi
    connect(okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(cancelButtonClicked()));
    
    // layout pro prihlaseni uzivatele
    QVBoxLayout *loginLayout  = new QVBoxLayout;
    loginLayout->addWidget(loginLabel);
    loginLayout->addWidget(loginEdit);
    loginLayout->addWidget(passwordLabel);
    loginLayout->addWidget(passwordEdit);
    loginLayout->addStretch();
    
    // layout pro zadani serveru a portu
    QVBoxLayout *serverPortLayout  = new QVBoxLayout;
    serverPortLayout->addWidget(serverLabel);
    serverPortLayout->addWidget(serverEdit);
    serverPortLayout->addWidget(portLabel);
    serverPortLayout->addWidget(portEdit);
    serverPortLayout->addStretch();

    // layout pro tlacitka
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    buttonsLayout->addStretch();
    
    // hlavni layout
    layout = new QVBoxLayout;
    layout->addLayout(loginLayout);
    layout->addSpacing(20);
    layout->addLayout(serverPortLayout);
    layout->addSpacing(20);
    layout->addLayout(buttonsLayout);
    layout->addStretch();

    setLayout(layout);
    // Diky teto volbe nebude aktivni hlavni okno, dokud uzivatel
    // nevyresi dialogove okno
    setModal(true);
    setFixedSize(230,270);
}

void DialogConnect::okButtonClicked() {
    // pokud uzivatel nechal nektere pole prazdne
    if(   loginEdit->text().isEmpty() ||
       passwordEdit->text().isEmpty() ||
         serverEdit->text().isEmpty() ||
           portEdit->text().isEmpty()) {
        // vypise se varovani
        QMessageBox::warning(this, tr("Petri Net Editor"), 
                             tr("All fields must be filled."));
        return;
    }
    
    // ulozeni zadanych hodnot
    login = loginEdit->text();
    password = passwordEdit->text();
    server = serverEdit->text();
    port = portEdit->text();
    
    // vycisteni textovych poli
    loginEdit->clear();
    passwordEdit->clear();
    serverEdit->clear();
    portEdit->clear();
    
    //hide();
    accept();
}
// Nastavi textova pole na implicitni hodnoty
void DialogConnect::setImplicitFields() {
    serverEdit->setText(IMPLICIT_SERVER);
    portEdit->setText(IMPLICIT_PORT);
}

// konstruktor dialogu pro vybrani site
myGroup::myGroup(QWidget *parent) :
    QDialog(parent)
{
    listWidget = new QListWidget();
    listWidget->setSortingEnabled(true);
    layout = new QHBoxLayout();
    mainLayout = new QVBoxLayout;

}

void myGroup::end() {
    reject();
}
void myGroup::ok() {
    accept();
}
void  myGroup::add(QString item) {
    listWidget->addItem(new QListWidgetItem(item));
}
void  myGroup::create() {

        quitButton = new QPushButton(tr("&Cancel"));
        okButton = new QPushButton(tr("&Ok"));
        this->listWidget->sortItems();
        this->listWidget->setCurrentRow(0);
        QHBoxLayout *bottomLayout = new QHBoxLayout;
        bottomLayout->addStretch();
        bottomLayout->addWidget(okButton);
        bottomLayout->addWidget(quitButton);

        layout->addWidget(listWidget);

        mainLayout->addLayout(layout);
        mainLayout->addLayout(bottomLayout);

        setLayout(mainLayout);
        setModal(true);
        // nastaveni signalu
        connect(quitButton, SIGNAL(clicked()),this, SLOT(end()));
        connect(okButton, SIGNAL(clicked()),this, SLOT(ok()));
        setWindowTitle(tr("Choose petri net from server"));
}
// vrati jmeno vybraneho souboru
QString myGroup::getname() {
    return this->listWidget->selectedItems()[0]->text();
}
