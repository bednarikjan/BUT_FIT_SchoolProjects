/** @file: constants.h  
  * @author: Jan Bednarik (xbedna45)
  * @date: 17.11.2012
  *
  * Symbolicke konstanty. Nektere jsou potreba pouze pro danou tridu, 
  * nektere jsou sdilene.
  */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define DEFAULT_MAX_MTU   1500    /**< Vychozi hodnota max. MTU podle zadani (Params)*/
#define MIN_MTU_4         68      /**< Min. velikost Path MTU pro IPv4 podle RFC. (Params, TargetHost) */
#define MIN_MTU_6         1280    /**< Min. velikost Path MTU pro IPv6 podle RFC 2460. (Params, TargetHost) */
#define MAX_DGRAM_SIZE_4  65535   /**< Max. datagram IPv4 urcen 16b rozsahem pole Total Length IPv4 hlavicky (Socket)*/
#define MAX_DGRAM_SIZE_6  65575   /**< Max. datagram IPv6 urcen 16b rozsahem pole Payload Length IPv6 hlavicky + 40B hlavicka IPv6 (Socket)*/
#define IPV6_LEN          39      /**< Max. delka IPv6 adresy ve znacich (TargetHost) */
#define DF_BIT_OFFSET     14      /**< Offset bitu DF pole frag_off v IPv4 hlavicce (Socket) */
#define DEFAULT_TTL       30      /**< Vychozi hodota TTL podle zadani (Socket) */
#define ICMP_HDR_BYTE8    8       /**< Pocet (8bitovych) bajtu ICMP hlavicky */
#define DEF_TIMEOUT       3       /**< Vychozi timeout pri cekani na ICMP odpoved */

#endif // CONSTANTS_H
