/** @file: targethost.cpp
  * @author: Jan Bednarik (xbedna45)
  * @date: 17.11.2012
  *
  * Hlavni trida implementujici metody pro ziskani Path MTU.
  */

#include "constants.h"
#include "targethost.h"
#include "error.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <sstream>

// DEBUG
#include <bitset>

using namespace std;

/** Konstruktor */
TargetHost::TargetHost(): min(0), max(0), seq(0) {
    pid = (unsigned short) getpid();
    memset(targetAddr, '\0', sizeof(targetAddr));       
    memset(mySockaddr, '\0', sizeof(mySockaddr));       
}

/** Inicializace - ziskani adresy cile, tvorba soketu. 
 *
 *  @param  targetAddr IPv4 / IPv6 / domenove_jmeno cile 
 *  @return aifamily pouzita rodina protokolu (IPv4 ¦ IPv6)
 */
void TargetHost::init(string& address, int _max) {        
    max = _max;    
    aifamily = resolveAddress(address);    
    sockaddr_in *sa4;
    sockaddr_in6 *sa6;
    bool on;
    int ttl;
    int val;
    
    // nastaveni minimalni hranice MTU podle adresni rodiny
    min = aifamily == AF_INET ? MIN_MTU_4 : MIN_MTU_6;
    
    // overeni korekrnosti hodnoy MTU zadane uzivatelem
    if(((aifamily == AF_INET)  && ((max > MAX_DGRAM_SIZE_4) || (min < MIN_MTU_4))) ||
       ((aifamily == AF_INET6) && ((max > MAX_DGRAM_SIZE_6) || (min < MIN_MTU_6))) || 
        (max < min))        
        throw Error("Error: Unsupported maximal MTU size (parameter -m).");            
    
    // tvorba soketu
    if((s = socket(aifamily, SOCK_RAW, 
                  (aifamily == AF_INET ? IPPROTO_ICMP : IPPROTO_ICMPV6))) == -1)
        throw Error("Error: socket(): Attempt to create socket failed. (root required)");

    switch(aifamily) {
            
        case AF_INET: /* IPv4 */
            // volba manualniho vlozeni IP hlavicky
            on = true;
            if(setsockopt(s, SOL_IP, IP_HDRINCL, (const bool *) &on, sizeof(bool)) == -1)
                throw Error("Error: setsockopt(): " 
                            "Attempt to modify socket options failed (IP_HDRINCL)");
            
            // Zamezeni provadeni Path MTU discovery, aby nefragmentoval odchozi data.           
            val = IP_PMTUDISC_PROBE;
            if (setsockopt(s, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof (int)))
                throw Error("Error: setsockopt(): " 
                            "Attempt to modify socket options failed (IP_MTU_DISCOVER)");                                         
            
            // Vyplneni struktury cile IPv4
            sa4 = (sockaddr_in *)mySockaddr;
            sa4->sin_family = AF_INET;
            sa4->sin_port = 0;        
            inet_pton(aifamily, targetAddr, &(sa4->sin_addr));            
            
            break;
            
        case AF_INET6: /* IPv6 */        
            // nastaveni TTL
            ttl = DEFAULT_TTL;
            if(setsockopt(s, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof (int)))
                throw Error("Error: setsockopt(): " 
                            "Attempt to modify socket options failed (IPV6_UNICAST_HOPS)");  
            
            // Zamezeni provadeni Path MTU discovery, aby nefragmentoval odchozi data.           
            val = IPV6_PMTUDISC_PROBE;
            if (setsockopt(s, IPPROTO_IPV6, IPV6_MTU_DISCOVER, &val, sizeof (int)))
                throw Error("Error: setsockopt(): " 
                            "Attempt to modify socket options failed (IPV6_MTU_DISCOVER)");  
                            
            // Vyplneni struktury cile IPv6
            sa6 = (sockaddr_in6 *)mySockaddr;
            sa6->sin6_family = AF_INET6;
            sa6->sin6_port = 0;        
            inet_pton(aifamily, targetAddr, &(sa6->sin6_addr));            
            
            break;
    }
}

/** 
 *  Zjisteni Path MTU k cilove stanici. Metoda vyuziva algoritmus
 *  binarniho vyhledavani pro nejrychlejsi objeveni vysledneho
 *  Path MTU.
 *
 *  @return pathMTU ziskane path MTU
 */
int TargetHost::DiscoverPathMTU() {       
    int retVal = 0;
    int icmpType, icmpCode ;
    bool msgIsForMe = false;
    bool packetTooBig = false;
    int timeout;
    stringstream code;
    
    // hlavni cyklus odesilani ICMP zpravy
    while(max >= min) {
        createMsg(((max + min) / 2));
        if(sendMsg((max + min) / 2) == -1) {        
            if(errno == EMSGSIZE) {  
                // Lokalni sitove rozhrani nepodporuje tak velky datagram                
                if(aifamily != AF_INET6) {
                    cout << "message too big" << endl;
                    max = ((max + min) / 2) - 1;                                
                    continue;                    
                }
            } else {
                perror("sendto()");
                throw Error("Error: sendto: Attempt to send a message failed.");
            }
        }            
        /* zprava byla odeslana */          
        
        timeout = DEF_TIMEOUT;
        do {
            if((retVal = waitForMsg(timeout)) == -1) {   // select() selhal
                Error("Error: select(): Attempt to wait for incoming data failed.");
            } else if (retVal == 0) {   // vyprsel timeout
                cout << "timeout" << endl;
                // vyprsel timout i pro minimalni MTU
                if(((aifamily == AF_INET)  && (max <= (MIN_MTU_4 + 1))) ||
                   ((aifamily == AF_INET6) && (max <= (MIN_MTU_6 + 1))))
                   throw Error("Error: Destination unreahable");                
                max = ((max + min) / 2) - 1;            
                packetTooBig = false;
                break;
            }
           /* prisla data */
            
            rcvMsg();  
            
            // zjisteni typu ICMP / ICMPv6 odpovedi
            if(aifamily == AF_INET) {
                icmpType = (int)(((icmphdr *)(msg + sizeof(iphdr)))->type);
                icmpCode = (int)(((icmphdr *)(msg + sizeof(iphdr)))->code);
            }
            else { 
                icmpType = (int)(((icmp6_hdr *)(msg))->icmp6_type);                                 
                icmpCode = (int)(((icmp6_hdr *)(msg))->icmp6_code);
            }
                       
            if((msgIsForMe = msgForMe(icmpType)) == true) {    // overeni IS a seq cisla prichozi ICMP            
                // ICMPv4
                if(aifamily == AF_INET) {
                    switch(icmpType) {
                        case ICMP_ECHOREPLY:
                            cout << "ok" << endl;
                            min = ((max + min) / 2) + 1; 
                            break;
                            
                        case ICMP_DEST_UNREACH:
                            if(icmpCode == ICMP_FRAG_NEEDED) {
                                cout << "message too big" << endl;
                                max = ((max + min) / 2) - 1; 
                            // jiny duvod pro nedostupnost cile
                            } else {
                                code << icmpCode;
                                throw Error("Error: Destination unreahable (ICMP type 3 code " + code.str() + ").");
                            }                        
                            break;
                            
                        case ICMP_TIME_EXCEEDED: 
                            code << icmpCode;
                            throw Error("Error: Time Exceeded (ICMP type 11 code " + code.str() + ").");
                            break;
                            
                        default:
                            break;
                    }
                // ICMPv6                            
                } else {
                    switch(icmpType) {
                        case ICMP6_ECHO_REPLY:
                            cout << "ok" << endl;
                            if(packetTooBig) return (min + max) / 2;
                            min = ((max + min) / 2) + 1;                                             
                            break;                    
                            
                        case ICMP6_PACKET_TOO_BIG:
                            cout << "message too big" << endl;
                            // nastaveni horni hranice tak, aby stred =
                            // (nova testovana hodnota) vysel na next hop mtu.
                            max = 2 * ntohl(((icmp6_hdr *)(msg))->icmp6_dataun.icmp6_un_data32[0]) - min;
                            packetTooBig = true;
                            break;    
                            
                         case ICMP6_TIME_EXCEEDED:
                            code << icmpCode;
                            throw Error("Error: Time Exceeded (ICMPv6 type 3 code " + code.str() + ").");
                            break;
                            
                         case ICMP6_DST_UNREACH:                            
                            code << icmpCode;
                            throw Error("Error: Destination Unreachable (ICMPv6 type 1 code " + code.str() + ").");                                                
                            break;
                            
                        default:
                            break;
                    }                           
                }
                        
            } else {
                timeout = 1;
            } 
        } while(!msgIsForMe);       
    }        
    
    return (min + max) / 2;
}

/** Preklad adresy cile
  *
  * @param targetAddr IPv4 / IPv6 / domenove_jmeno cile 
  */
int TargetHost::resolveAddress(string& address) {
    struct addrinfo *target, *tmp; 
    struct addrinfo hints;    
    void *ptr = NULL;
    
    memset(&hints, 0, sizeof (hints));           
    hints.ai_family = PF_UNSPEC;  // akceptuje IPv4 i IPv6
    
    // rezoluce
    if(getaddrinfo(address.c_str(), NULL, &hints, &target) != 0)     
        throw Error("Error: Target host not found.");
    
    // preferovani IPv6 adresy
    if(target->ai_family != AF_INET6) {
        tmp = target;
        while(tmp->ai_next != NULL) {                    
            if(tmp->ai_family == AF_INET6) {
                target = tmp;
                break;
            }
            tmp = tmp->ai_next;
        }                
    }
    
    target->ai_family == AF_INET ? 
        (ptr = &(((struct sockaddr_in  *)target->ai_addr)->sin_addr)) :                                  
        (ptr = &(((struct sockaddr_in6 *)target->ai_addr)->sin6_addr));
    inet_ntop(target->ai_family, ptr, targetAddr, sizeof(targetAddr));                    
    
    return target->ai_family;
}                   

/** 
 * Vyplneni odchoziho bufferu IP a ICMP hlavickou a nulovymi daty
 *
 * @param len aktualni delka odesilaneho datagramu
 * @param aifamily Rodina protokolu
 * @param targetAddr adresa cile
 */
void TargetHost::createMsg(unsigned short len) {    
    iphdr     *ip4Send;
    icmphdr   *icmp4Send;
    icmp6_hdr *icmp6Send;

    memset((void *)msg, '\0', sizeof(msg));
    
    /* IPv4 */
    if(aifamily == AF_INET) {
        ip4Send = (iphdr *)msg;
        icmp4Send = (icmphdr *)(msg + sizeof(iphdr));                

        // IP hlavicka
        ip4Send->version = 4;
        ip4Send->ihl = 5;
        ip4Send->tos = 0;
        ip4Send->tot_len = htons(len);
        ip4Send->id = 0;
        ip4Send->frag_off = htons(1 << DF_BIT_OFFSET);
        ip4Send->ttl = DEFAULT_TTL;
        ip4Send->protocol = IPPROTO_ICMP;
        ip4Send->check = 0; 
        ip4Send->saddr = 0;
        if(inet_pton(aifamily, targetAddr, &(ip4Send->daddr)) <= 0)
            throw Error("Error: inet_pton(): Wrong target address.");            
        
        // ICMP hlavicka
        icmp4Send->type = ICMP_ECHO;
        icmp4Send->code = 0;
        icmp4Send->un.echo.id = htons(pid);
        icmp4Send->checksum = 0;
        icmp4Send->un.echo.sequence = htons(++seq);    
        icmp4Send->checksum = icmpChecksumRFC((unsigned short *) icmp4Send, len - sizeof(iphdr));                    
    } 
    
    /* IPv6 */
    else if (aifamily == AF_INET6) {
        icmp6Send = (icmp6_hdr *)msg;
        
        //ICMPv6 hlavicka
        icmp6Send->icmp6_type = ICMP6_ECHO_REQUEST;
        icmp6Send->icmp6_code = 0;
        icmp6Send->icmp6_cksum = 0;        
        icmp6Send->icmp6_dataun.icmp6_un_data16[0] = htons(pid);   // id
        icmp6Send->icmp6_dataun.icmp6_un_data16[1] = htons(++seq); // sekvencni cislo           
    }    
}

/** Zaslani ICMP zpravy cili
* @param len delka zpravy
*/
int TargetHost::sendMsg(int len) {            
    cout << "Trying " << len << " B ... " << flush;
    
    if(aifamily == AF_INET6) len -= sizeof(ip6_hdr);
    
    errno = 0;    
    return sendto(s, (char *) msg, len, 0, (sockaddr *)mySockaddr, sizeof(sockaddr_in6));             
}

/** Prijem ICMP zpravy od cile (nebo jineho sitoveho prvku na ceste)
*/
void TargetHost::rcvMsg() {
    unsigned int size;
    int msgLen;
    
    memset(msg, '\0', sizeof(msg));
    size = aifamily == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    
    if ((msgLen = recvfrom(s, msg, sizeof(msg), 0, (sockaddr *)mySockaddrRcv, &size)) == -1) {
        throw Error("Error: recvfrom(): Attempt to recieve a message failed.");
    }        
}

/** Vypocet checksum ICP hlavicky
* @param icmp4Send ukazatel na strukturu icmp
*
* Kod prevzat a mirne upraven z RFC 1071 (http://www.faqs.org/rfcs/rfc1071.html)
*/
unsigned short TargetHost::icmpChecksumRFC(unsigned short *buf, int count) {
   /* Compute Internet Checksum for "count" bytes
    *         beginning at location "addr".
    */
    unsigned long sum = 0;    

    while( count > 1 )  {
    /*  This is the inner loop */
        sum += *(buf++);        
        count -= 2;
   }

    /*  Add left-over byte, if any */
    if( count > 0 )
        sum += * (unsigned char *) buf;

    /*  Fold 32-bit sum to 16 bits */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;    
}

/**
*   Cekani na ICMP odpoved cile nebo jineho sitoveho prvku na ceste
*   @param timeout delka cekani v sekundach
*   @return vysledek fce select()
*/
int TargetHost::waitForMsg(int timeout) {
    fd_set rfds;
    struct timeval tv;        

    FD_ZERO(&rfds);
    FD_SET(s, &rfds);

    // Nastaveni timeoutu
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return select(s + 1, &rfds, NULL, NULL, &tv);    
}

/**
*   Overi ID a sekvencni cislo prichozi ICMP zpravy
*   @param icmpType Typ ICMP zpravy
*   @return zda je zprava pro me
*/
bool TargetHost::msgForMe(int icmpType) {
    bool myMsg = false;
    icmphdr *hdr4;
    icmp6_hdr *hdr6;
    
    // ICMPv4
    if(aifamily == AF_INET) {
        switch(icmpType) {        
            case ICMP_ECHOREPLY:
                hdr4 = (icmphdr *)(msg + sizeof(iphdr));            
                break;
            
            case ICMP_TIME_EXCEEDED:    
            case ICMP_DEST_UNREACH:
                /* ID a seq neni v hlvaicce ICMP odpovedi. V datove casti se zpet posila
                IP hlavicka puvodni zpravy a 64 bitu dat, v nichz je moje puvodni ICMP
                hlavicka -> na tu nastavuji ukazatel hdr */
                hdr4 = (icmphdr *)(msg + 2 * sizeof(iphdr) + sizeof(icmphdr));
                break;
                
            // neznama ICMP odpoved
            default:                   
                return myMsg;
                break; 
        }
    // ICMPv6                
    } else {
        switch(icmpType) {        
            case ICMP6_ECHO_REPLY:            
                hdr6 = (icmp6_hdr *)(msg);
                break;
            
            case ICMP6_TIME_EXCEEDED:
            case ICMP6_DST_UNREACH:    
            case ICMP6_PACKET_TOO_BIG:            
                hdr6 = (icmp6_hdr *)(msg + sizeof(icmp6_hdr) + sizeof(ip6_hdr));
                break;                  
            
            // neznama ICMP odpoved
            default:                   
                return myMsg;
                break; 
        }            
    }    
    
    // overeni spravnosti sekvencniho cisla ICMP zpravy
    if((aifamily == AF_INET) && 
       (ntohs(hdr4->un.echo.id) == pid) && 
       (ntohs(hdr4->un.echo.sequence) == seq))
        myMsg = true;
    
    if((aifamily == AF_INET6) && 
       (ntohs(hdr6->icmp6_dataun.icmp6_un_data16[0]) == pid) && 
       (ntohs(hdr6->icmp6_dataun.icmp6_un_data16[1]) == seq))        
        myMsg = true;                            
    
    return myMsg;
}  


// DEBUG vypis hlavicek IP a ICMP
void TargetHost::debugPrintHdr() {
    iphdr *ip = (iphdr *)msg;
    icmphdr *icmp = (icmphdr *)((char *)msg + 20);
    
    std::bitset<16> b(ip->frag_off);
    
    cout << "\n--- debugPrintHdr() ---\n";
    cout << "adresa ip: " << ip << "\nadresa icmp: " << icmp << endl;
    
    // IP hlavicka
    cout << "IP HEADER\n-----\n";    
    cout << "version: " << ip->version << endl;
    cout << "ihl: " << ip->ihl << endl;
    cout << "tos(DSCP,ECN): " << (int)ip->tos << endl;
    cout << "Total Length: " << ip->tot_len << endl;
    cout << "ID: " << ip->id << endl;
    cout << "Flags + Offset: " << b << endl;
    cout << "TTL: " << (int)ip->ttl << endl;
    cout << "Protocol: " << (ip->protocol == IPPROTO_ICMP ? "IPPROTO_ICMP" : "unknown") << endl;
    cout << "checksum: " << ip->check << endl;
    cout << "saddr: " << ip->saddr << endl;
    cout << "daddr: " << ip->daddr << endl;
                    
    // ICMP hlavicka    
    cout << "\n\nICMP HEADER\n------\n";
    cout << "type: " << (icmp->type == ICMP_ECHO ? "ICMP_ECHO" : "unknown") << endl;
    cout << "code: " << (int)icmp->code << endl;
    cout << "checksum: " << icmp->checksum << endl;
    cout << "id: " << icmp->un.echo.id << endl;
    cout << "seq: " << icmp->un.echo.sequence << endl;        
}
