/** @file: socket.h  
  * @author Jan Bednarik
  *
  * Trida implementujici komunikaci pres soket.
  */

#ifndef SOCKET_H
#define SOCKET_H

#include "constants.h"
#include <cstring>
#include <sys/types.h>
#include <netinet/ip_icmp.h> 
#include <netdb.h>

/** RAW soket, prez nejz se komunikuje 
 */
class Socket
{
    public:
        Socket();
        //void init(int aifamily, char *targetAddr, in_addr *adr); /**< Tvorba soketu */
        void init(int aifamily, char *targetAddr); /**< Tvorba soketu */
        void createMsg(unsigned short len, int aifamily, char *targetAddr);        
        int sendMsg(int len);  /**< Zaslani ICMP zpravy cili */        
        void rcvMsg();          /**< Prijem odpovedi */
        
        void debugPrintHdr();
        hostent *host;                 
        
    private:
        
        
        
        
};

#endif // SOCKET_H
