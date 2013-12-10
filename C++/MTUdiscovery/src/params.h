/** @file: params.h  
  * @author: Jan Bednarik (xbedna45)
  * @date: 17.11.2012
  *
  * Trida pro zpracovani parametru.
  */

#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include "constants.h"

using namespace std;

/** 
 * Trida pro zpracovani parametru.
 */
class Params {
    private:        
        int maxMTU;     /**< parametr -m */
        string address; /**< parametr ADDRESS */        
        bool getMTUfromArg(int arg, char *argv[]);  /**< Prevod parametru na cislo. */
        
    public:
        Params(): maxMTU(DEFAULT_MAX_MTU), address("") {}      	
        string& getAddress() {return address;}
        int     getMaxMTU() {return maxMTU;}             
        void    ProcessParams(int argc, char *argv[]);  /**< Zpracovani parametru. */
};

#endif // PARAMS_H
