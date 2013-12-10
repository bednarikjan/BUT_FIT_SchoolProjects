/* Projekt: Klient/server
 * Predmet: IPK
 * Program: client
 * Autor: Jan Bednarik
 * Login: xbedna45
 * Datum: 7.4.2012
 */

#include <unistd.h>	// getopt()
#include <stdexcept>	// Standardni vyjimky
#include <string>
#include <iostream>
#include <cstdlib>	// atoi()
#include <cctype>	// isdigit(), isalpha()
#include <sys/socket.h>	// struct sockaddr, socket(), connect(), AF_INET, SOCK_STREAM
#include <netinet/in.h>	// struct sockaddr_in
#include <netdb.h>	// struct hostent, gethostbyname()

#define IPV4 4
#define IPV6 6
#define PORT_MIN 0
#define PORT_MAX 65535
#define MAX_MSG_LEN 100

using namespace std;

const char *helpMsg = ""
"Klient/server DNS rezoluce - klient\n"
"-----------------------------------\n\n"
"Synopsis:\n"
"server HOST:PORT [-4] [-6] DOMENJMENO\n\n"
"Modifikacni parametry:\n"
"-h - napoveda\n"
"-4 - vraci IPv4 adresu\n"
"-6 - vraci IPv4 adresu\n";


class CParams;
class CSocket;
class CMessage;
  
// Trida pro zpracovani parametru
// =======================================================================================
class CParams {
  private:
    string hostPort;
    string host;
    int port;
    string domainName;
    int options[2];
  public:
    CParams(): port(-1) {options[0] = options[1] = 0;}
    void Help();
    void GetParams(int argc, char *argv[]);
    void Extract();
    friend class CSocket;
    friend class CMessage;
};

/* Metoda pro ziskani parametru. */
void CParams::GetParams(int argc, char *argv[]) {
  // Napoveda.
  if((argc == 2) && (strcmp(argv[1], "-h") == 0)) throw(runtime_error(helpMsg));
  // Prilis malo parmetru.
  if(argc < 4) throw(runtime_error("CHYBA: Nekorektne zadane parametry."));
  
  // Jmeno domeny a cislo portu.
  hostPort = argv[1];
  
  // volby -4, -6
  int i = 0;
  bool done[2] = {false};  // index 0 .. IPv4, index 1 .. IPv6
  int param;
  while((param = getopt(argc-1, &argv[1], ":46")) != -1) {
    switch(param) {
      case '4':
	if(done[0] == false) {options[i++] = IPV4; done[0] = true;}
	break;
      case '6':
	if(done[1] == false) {options[i++] = IPV6; done[1] = true;}
	break;
      case '?':
	throw(runtime_error("CHYBA: Nekorektne zadane parametry."));
	break;
      default: break;
    }
  }
  
  if(argc != optind + 2) throw(runtime_error("CHYBA: Nekorektne zadane parametry."));
  domainName = argv[optind+1];
}

/* Metoda pro ziskani jmena hosta a cisla portu. 
 */
void CParams::Extract() {
  // HOST a PORT oddelen znakem ':'
  size_t found;
  if((found = hostPort.find_last_of(":")) == hostPort.npos ||
      found + 1 == hostPort.length())
    throw(runtime_error("CHYBA: Chybne cislo portu."));
  
  // Jmeno hosta.
  host = hostPort.substr(0,found);
  if(host.length() > 255) throw(runtime_error("CHYBA: Chybne jmeno hosta."));
  
  // Cislo portu.
  char *end;
  if(((port = strtol((hostPort.substr(found + 1)).c_str(), &end, 10)) == -1) || 
      (*end != '\0') ||
      (port < PORT_MIN || port > PORT_MAX)) 
    throw(runtime_error("CHYBA: Chybne cislo portu."));    
}

// Trida implementuje metody, ktere server provadi nad soketem.
// =======================================================================================
class CSocket {
  private:
    int s;	// soket
    struct sockaddr_in sin; 
    struct hostent *hptr;
  public:
    CSocket(): s(-1), hptr(NULL) {};
    void CreateSocket();
    void Connect(CParams & params);
    void Close() {close(s);}
    friend class CMessage;
};

/* Metoda pro vytvoreni komunikacniho soketu. */
void CSocket::CreateSocket() {
  if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    throw(runtime_error("CHYBA(socket): Nelze otevrit soket."));
}

/* Metoda pro pripojeni k hostu. Provadi preklad zadaneho 
 * jmena hosta na IPv4 adresu. */
void CSocket::Connect(CParams & params) {
  sin.sin_family = AF_INET;
  sin.sin_port = htons(params.port);
  
  // Preklad jmena hosta na IPv4 adresu
  if((hptr = gethostbyname((params.host).c_str())) == NULL) 
    throw(runtime_error("CHYBA(gethostbyname): Nelze prelozit zdresu hosta."));
  memcpy(&sin.sin_addr, hptr->h_addr, hptr->h_length);  
  
  // pripojeni k hostu.
    if(connect(s,(struct sockaddr *) &sin, sizeof(sin)) < 0)
      throw(runtime_error("CHYBA(connect): Nelze se pripojit k hostu."));
}

// Trida pro komunikaci se serverem
// =======================================================================================
class CMessage {
  private:
    string msg;
    char received[MAX_MSG_LEN];
  public:
    CMessage() {memset(received, '\0', MAX_MSG_LEN);}
    void CreateMsg(CParams & params);
    void Send(CSocket & objSock);    
    void Read(CSocket & objSock);
    void ParseAndPrint(CParams & params);
};

/* Metoda pripravi zpravu pro hosta.*/
void CMessage::CreateMsg(CParams & params) {
  msg += "Host:";
  msg += params.domainName;
  msg += "\r\n";
  msg += "Type:";
  switch(params.options[0] + params.options[1]) {
    case 4:  msg += "4";  break;
    case 6:  msg += "6";  break;
    case 10: msg += "10"; break;
  }
  msg += "\r\n";
}

/* Metoda pro zaslani zpravy hostu.*/
void CMessage::Send(CSocket & objSock) {
  if(write(objSock.s, msg.c_str(), strlen(msg.c_str())) < 0) 
    throw(runtime_error("CHYBA(write): Nelze zaslat zpravu hostu."));
}

/* Metoda pro cteni odpovedi od hosta.*/
void CMessage::Read(CSocket & objSock) {
  if(read(objSock.s, received, sizeof(received)) < 0)
    throw(runtime_error("CHYBA(read): Nelze precist zpravu od hosta."));
}

/* Metoda pro zpracovani a tisk odpovedi od hosta.*/
void CMessage::ParseAndPrint(CParams & params) {
  char *ipv4start = NULL;
  char *ipv6start = NULL;
  
  ipv4start = strstr(received, "IPv4:") ;
  ipv6start = strstr(received, "IPv6:") ;
  if(ipv4start != NULL) {
    ipv4start += strlen("IPv4:");
    *(strchr(ipv4start, '\r')) = '\0';
  }
  if(ipv6start != NULL) {
    ipv6start += strlen("IPv6:");
    *(strchr(ipv6start, '\r')) = '\0';
  }
  
  for(int i = 0; i < 2; i++) {
    switch(params.options[i]) {
      case 0: break;
      
      case IPV4:
	if(ipv4start == NULL) throw(runtime_error("CHYBA: Neuplna odpoved od hosta."));
	if (*ipv4start == '-') cerr << "Err4: Nenalezeno." << endl;
	else cout << ipv4start << endl;
	break;
	
      case IPV6:
	if(ipv6start == NULL) throw(runtime_error("CHYBA: Neuplna odpoved od hosta."));
	if (*ipv6start == '-') cerr << "Err6: Nenalezeno." << endl;
	else cout << ipv6start << endl;
	break;
    }
  }
}

//--------------------------------------------------------------------------------------//
int main(int argc, char *argv[]) {
  try {
    // Zpracovani parametru.
    CParams params;
    params.GetParams(argc, argv);
    params.Extract();
    
    // Pripojeni k hostu.
    CSocket objSock;
    objSock.CreateSocket();
    objSock.Connect(params);
    
    // Zaslani zpravy a prijem, parsovani a tisk odpovedi.
    CMessage objMsg;
    objMsg.CreateMsg(params);
    objMsg.Send(objSock);
    objMsg.Read(objSock);
    objMsg.ParseAndPrint(params);
    
    objSock.Close();
    
  } catch(runtime_error &e) {
    cerr << e.what() << endl;
    exit(1);
  }
  return 0;
}
