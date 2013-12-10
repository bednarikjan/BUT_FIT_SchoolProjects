/** @file: error.h  
  * @author: Jan Bednarik (xbedna45)
  * @date: 17.11.2012
  *
  * Trida pro run-time chyby. Napoveda.
  */
  
#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>
#include <string>

using namespace std;

/** 
 * Trida vyjimek - chybovych hlaseni
 */
class Error : public runtime_error {
    public:
        Error(const string& errMsg) : runtime_error(errMsg) {}
};

/** 
 * Vypis napovedy.
 */
class Help {   
    public:
        static void PrintHelp() {
            printf(
                "NAME\n"
                "\tmypmtud - Discovers MTU on the path to given destination.\n\n"
                "SYNOPSIS\n"
                "\tmypmtud [-m MAXMTU] DESTINATION\n\n"
                "DESCRIPTION\n"
                "\tDiscovers MTU on the path to DESTINATION and prints the result. "
                "The deafult non-customizable settings are ttl (time to live) = 30, "
                "timeout = 3 s.\n\n"
                "OPTIONS\n"                
                "\t-m MAXMTU\n\t\tUses MAXMTU value as a maximum possible MTU along the path.\n"
                "Allowed range for IPv4 DESTINATION: <68, 65535>\n"
                "Allowed range for IPv6 DESTINATION: <1280, 65575>\n\n"
                "AUTHOR\n"            
                "\tmypmtud was written by Jan Bednarik (xbedna45@stud.fit.vutbr.cz).\n"
            );
        }
};

#endif // ERROR_H
