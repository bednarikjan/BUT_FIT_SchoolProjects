/** @file: params.c
  * @author: Jan Bednarik (xbedna45)
  * @date: 17.11.2012
  *
  * Zpracovani parametru.
  */

#include "constants.h"
#include "params.h"
#include "error.h"
#include <cstring>

/** 
 * Zpracovani parametru. 
 */
void Params::ProcessParams(int argc, char *argv[]) {
    bool badParams = false;
    
    switch(argc) {
        // mypmtu -h ¦ mypmtud address
        case 2:
            if(strcmp(argv[1],"-h") == 0) {
                Help::PrintHelp(); 
                exit(0); 
            } else {
                address = argv[1];                    
            }            
            break;
                  
        // mypmtud -m number address            
        case 4:
            if(strcmp(argv[1], "-m") == 0) {                
                if(!getMTUfromArg(2, argv)) { badParams = true; } 
                else { address = argv[3]; }                                                    
            } else { badParams = true; }
            break;
            
        // nespravne parametry    
        default:
            badParams = true;                   
            break;                            
    }
    
    if(badParams)
        throw Error("Error: Invalid options. Try 'mypmtud -h.' for more infromation.");    
}

/**
* Ziskani ciselne hdonoty MTU ze zadanych argumentu
*/
bool Params::getMTUfromArg(int arg, char *argv[]) {
    bool ok = true; 
    char *endptr;
    
    maxMTU = strtol(argv[arg], &endptr, 10);                
                
    if(endptr == argv[2]  || *endptr != '\0' || 
       maxMTU < MIN_MTU_4 || maxMTU > MAX_DGRAM_SIZE_6) {
       ok = false;
       }
       
    return ok;
}


//  // mypmtu -v address
//        case 3:
//            if(strcmp(argv[1],"-v") == 0) {
//                verbose = true;
//                address = argv[2];
//            } else { badParams = true; }
//            break;
            
//             // mypmtud -v -m number address            
//        case 5:            
//            if(strcmp(argv[1], "-v") == 0) {
//                verbose = true;
//            } else { badParams = true; }
            
//            if(strcmp(argv[2], "-m") == 0) {
//                if(!getMTUfromArg(3, argv)) { badParams = true; } 
//                else { address = argv[4]; }                                                    
//            } else { badParams = true; }
//            break;
