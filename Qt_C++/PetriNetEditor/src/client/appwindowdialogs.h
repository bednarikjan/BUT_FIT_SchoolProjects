/** @file: appwindowdialogs.h
  * @author Jan Bednarik
  *
  * Obsahuje tridy pro dialogova okna
  */

#ifndef APPWINDOWDIALOGS_H
#define APPWINDOWDIALOGS_H

#include <QLineEdit>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QtGui>
#include "syntaxchecker.h"

/** Bazova trida pro dialogova okna */
class DialogGeneral: public QDialog {    
    Q_OBJECT
    
    public:
        /** Konstruktor */
        DialogGeneral(QWidget *parent = 0);
        /** vraceni zadaneho retezce */
        QString &getUserInput() {return userInput;}
        /** Vypise do textoveho pole ulozeny retezec. */
        void setLineEdit(QString text) {lineEdit->setText(text);}
        /** Kontrola syntaxe u vstupu */
        SyntaxChecker syntax;

        
    public slots:
        void cancelButtonClicked() {reject();}
    
    protected:
        QLabel *label;
        QLineEdit *lineEdit;
        QPushButton *okButton;
        QPushButton *cancelButton;
        QString userInput;
        QVBoxLayout *layout;
};                                  


/** Dialogove okno pro misto */
class DialogPlace: public DialogGeneral {
    Q_OBJECT
    
    public:
        DialogPlace(QWidget *parent = 0);
        
        
    public slots:
        void okButtonClicked();
};


/** Dialogove okno pro prechod */
class DialogTransition: public DialogGeneral {
    Q_OBJECT
    
    public:
        DialogTransition(QWidget *parent = 0);
        
        /** vraceni retezce pro akci */
        QString &getAction() {return action;}
        
        /** Vypise do textoveho pole pro vystupni akci ulozeny retezec. */
        void setActionEdit(QString text) {actionEdit->setText(text);}
        
    public slots:
        void okButtonClicked();
    
    private:
        QLabel *actionLabel;
        QLineEdit *actionEdit;    
        QString action;
};

/** Dialogove okno pro hranu */
class DialogArc: public DialogGeneral {
    Q_OBJECT
    
    public:
        DialogArc(QWidget *parent = 0);
        
        
    public slots:
        void okButtonClicked();
};

/** Dialogove okno pro pripojeni k serveru a registraci uzivatele */
class DialogConnect: public QDialog {
    Q_OBJECT
    
    public:    
        /** Konstruktor */
        DialogConnect(QWidget *parent = 0);
        
        /** Nastavi textova pole na implicitni hodnoty */
        void setImplicitFields();
        
        /** Vrati uzivateluv login */
        QString &getLoginString() {return login;}
        /** Vrati uzivateluv password */
        QString &getPasswordString() {return password;}
        /** Vrati adresu serveru */
        QString &getServerString() {return server;}
        /** Vrati port */
        QString &getPortString() {return port;}

        
    public slots:
        void okButtonClicked();
        void cancelButtonClicked() {reject();}
    
    private:    
        QVBoxLayout *layout;
        
        QLabel *loginLabel;
        QLineEdit *loginEdit;
        QLabel *passwordLabel;
        QLineEdit *passwordEdit;
        QLabel *serverLabel;
        QLineEdit *serverEdit;
        QLabel *portLabel;
        QLineEdit *portEdit;
        QPushButton *okButton;
        QPushButton *cancelButton;
        
        QString login;
        QString password;
        QString server;
        QString port;
};

class QGroupBox;
class QLabel;
class QPushButton;
class QRadioButton;


/** Dialogove okno pro vybrani souboru k otevreni ze serveru */
class myGroup : public QDialog
{
    Q_OBJECT
public:
    explicit myGroup(QWidget *parent = 0);

public slots:
    void end();
    void ok();
    void add(QString item);
    void create();
public:
      QHBoxLayout *layout;
      QVBoxLayout *mainLayout;
      QListWidget *listWidget;

      QPushButton *quitButton;
      QPushButton *okButton;

      QString getname();    /** Vrati jmeno vybraneho souboru */
      //QString getversion();     
};

#endif // APPWINDOWDIALOGS_H
