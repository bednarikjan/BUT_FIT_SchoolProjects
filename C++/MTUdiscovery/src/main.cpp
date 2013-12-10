/** @file: main.c  
  * @author: Jan Bednarik (xbedna45)
  * @date: 17.11.2012
  *
  * Hlavni funkce programu. Tisk vysledku.
  */

#include <iostream>
#include "targethost.h"
#include "params.h"
#include "error.h"

using namespace std;

// Hlavni funkce programu
int main(int argc, char *argv[]) {            
    Params params;
        
    try {        
        params.ProcessParams(argc, argv);                
        TargetHost host;
        host.init(params.getAddress(), params.getMaxMTU());                                        
        cout << "\nresume: " << host.DiscoverPathMTU() << " bytes" << endl;                
    } catch(Error &e) {
        cerr << e.what() << endl;
        exit(1);
    }    
    
    
    return 0;
}

