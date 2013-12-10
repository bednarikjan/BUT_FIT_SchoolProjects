/** @file: socket.cpp
  * @author Jan Bednarik
  *
  * Trida implementujici komunikaci pres soket.
  */

#include "constants.h"
#include "socket.h"
#include "error.h"
#include <string>
#include <iostream>
#include <sys/types.h> 
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <netinet/ip.h> 
#include <cerrno>

// DEBUG
#include <bitset>

using namespace std;

/** Konstruktor */
Socket::Socket(): seq(0) {     
    pid = getpid();
}                

/** Tvorba soketu  
 *  @param aifamily pouzita rodina protokolu (IPv4 ¦ IPv6)
 */
//void Socket::init(int aifamily, char *targetAddr, in_addr *adr) {
void Socket::init(int aifamily, char *targetAddr) {
    if((s = socket(aifamily, SOCK_RAW, IPPROTO_ICMP)) == -1)
        throw Error("Error: socket(): Attempt to create socket failed.");

    // volba manualniho vlozeni IP hlavicky
    const bool on = true;
    if(setsockopt(s, SOL_IP, IP_HDRINCL, (const bool *) &on, sizeof(bool)) == -1)
        throw Error("Error: setsockopt(): Attempt to modify socket options failed");
                
    // Vyplneni struktury cile IPv4
    sinSend.sin_family = AF_INET;
    sinSend.sin_port = 0;        
    inet_pton(aifamily, targetAddr, &(sinSend.sin_addr));            
}

/** Vyplneni odchoziho bifferu IP a ICMP hlavickou a nulovymi daty
*
* @param len aktualni delka odesilaneho datagramu
* @param aifamily Rodina protokolu
* @param targetAddr adresa cile
*/
void Socket::createMsg(unsigned short len, int aifamily, char *targetAddr) {
    // DEBUG
    cout << "Tvorim zpravu delky " << len << endl;
    
    iphdr *ipSend = (iphdr *)msg;
    icmphdr *icmpSend = (icmphdr *)(msg + sizeof(iphdr));
    
    //DEBUG
    cout << "adresa iphdr: " << ipSend << "\nadresa icmphdr: " << icmpSend << endl;
    
    memset((void *)msg, '\0', sizeof(msg));
    
    // IP hlavicka
    ipSend->version = 4;
    ipSend->ihl = 5;
    ipSend->tos = 0;
    //ipSend->tot_len = htons(len);
    ipSend->id = 0;
    ipSend->frag_off = htons(1 << DF_BIT_OFFSET);
    ipSend->ttl = DEFAULT_TTL;
    ipSend->protocol = IPPROTO_ICMP;
    ipSend->check = 0; 
    ipSend->saddr = 0;
    if(inet_pton(aifamily, targetAddr, &(ipSend->daddr)) <= 0)
        throw Error("Error: inet_pton(): Wrong target address.");            
    
    // ICMP hlavicka
    icmpSend->type = ICMP_ECHO;
    icmpSend->code = 0;
    icmpSend->un.echo.id = htons(pid);
    icmpSend->checksum = 0;
    icmpSend->un.echo.sequence = htons(++seq);    
    icmpSend->checksum = icmpChecksumRFC((unsigned short *) icmpSend, len - sizeof(iphdr));                
}

/** Zaslani ICMP zpravy cili
* @param len delka zpravy
*/
int Socket::sendMsg(int len) {            
    cout << "posilam zpravu\n";                                     // TODO
    errno = 0;
    return sendto(s, (char *) msg, len, 0, (sockaddr *)&sinSend, sizeof(sockaddr));         
}

/** Prijem ICMP zpravy od cile (nebo jineho sitoveho prvku na ceste)
*/
void Socket::rcvMsg() {
    unsigned int size;
    int msgLen;
    
    memset(msg, '\0', sizeof(msg));
    size = sizeof(sockaddr_in);
    
    if ((msgLen = recvfrom(s, msg, sizeof(msg), 0, (sockaddr *)&sinRecv, &size)) == -1) {
        throw Error("Error: recvfrom(): Attempt to recieve a message failed.");
    }        
    
    cout << "precetl jsem\n";
}

/** Vypocet checksum ICP hlavicky
* @param icmpSend ukazatel na strukturu icmp
*
* Kod pochazi z RFC 1071
*/
unsigned short Socket::icmpChecksumRFC(unsigned short *buf, int count) {
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

    cout << "checksum = " << sum << ", overeni = " << (long)(sum + ~sum) << endl; // TODO smazat

    return ~sum;    
}

// DEBUG vypis hlavicek IP a ICMP
void Socket::debugPrintHdr() {
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
