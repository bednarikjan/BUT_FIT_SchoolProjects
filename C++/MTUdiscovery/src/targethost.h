/** @file: targethost.h  
  * @author: Jan Bednarik (xbedna45)
  * @date: 17.11.2012
  *
  * Hlavni trida implementujici metody pro ziskani Path MTU.
  */

#ifndef TARGETHOST_H
#define TARGETHOST_H

#include "constants.h"
#include <string>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

/** 
 * Trdia pro navazani sitove komunikace, zjisteni MTU k cili pomoci
 * algoritmu binarniho vyhledavani
 */
class TargetHost
{
    public:
        TargetHost();
        void init(string& address, int _max);  /**< Ziskani adresy cile, tvorba soketu. */
        int  DiscoverPathMTU();                /**< Zjisteni Path MTU */        
    
    private:        
        int s;                       /**< Soket */    
        char targetAddr[IPV6_LEN+1]; /**< Adresa cilove stanice - textova reprezentace. */                
        int min, max;                /**< Hranice pri hledani Path MTU */        
        int aifamily;                /**< Rodina adres */
        char msg[MAX_DGRAM_SIZE_6];  /**< buffer pro odchozi/prichozi ICMP zpravy */         
        unsigned short pid;          /**< id procesu pouzite jako id icmp echo request zpravy */
        unsigned short seq;          /**< Sekvencni cislo ICMP zpravy */                        
        
        /** Adresa cile - nutne pouzit, ikdyz jsem rucne vyplnil IP, protoze 
            to pozaduje funkce sendto(). Funkci send() lze pouzit pouze pro soket v
            connected state (zrejme SOCK_STREAM ale ne SOCK_RAW) */
        char mySockaddr[sizeof(sockaddr_in6)];
        char mySockaddrRcv[sizeof(sockaddr_in6)];        
        hostent *host;                       
        
        int resolveAddress(string& address);   /**< Preklad adresy cile */        
        void createMsg(unsigned short len);    /**< Vyplneni IP a ICMP hlavicky */        
        int sendMsg(int len);                  /**< Zaslani ICMP zpravy cili */        
        void rcvMsg();                         /**< Prijem odpovedi */        
        int waitForMsg(int timeout);           /**< Cekani na odpoved */
        bool msgForMe(int icmpType);           /**< Overi ID a sekvencni cislo prichozi ICMP zpravy */
        unsigned short icmpChecksumRFC(unsigned short *buf, int count); /**< RFC implementace checksum. */                          
        
    // DEBUG
    public:        
        void debugPrintTargetAddr() { cout << "cilova adresa: " << targetAddr << endl; }
        void debugPrintHdr();
};

#endif // TARGETHOST_H
