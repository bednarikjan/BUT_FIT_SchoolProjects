/* Projekt: Klient/server
 * Predmet: IPK
 * Program: server
 * Autor: Jan Bednarik
 * Login: xbedna45
 * Datum: 7.4.2012
 */

#include <stdexcept>	// Standardni vyjimky
#include <string>
#include <cstring>
#include <iostream>
#include <cstring>	// strcmp()
#include <cstdlib>	// exit(), strtol()
#include <sys/socket.h> // socket(), bind(), listen(), accetp()
#include <netinet/in.h>	// struct sockaddr_in, inet_ntoa()
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>	// struct adrrinfo


#define MAX_MSG_LEN 200
#define MAX_HOST_LEN 300
#define MAX_IP4_LEN 16
#define MAX_IP6_LEN 40
#define PORT_MIN 0
#define PORT_MAX 65535

#define IPV4 4
#define IPV6 6
#define IPV4_AND_IPV6 10

using namespace std;

const char *helpMsg = ""
"Klient/server DNS rezoluce - server\n"
"-----------------------------------\n\n"
"Synopsis:\n"
"server -p PORT\n\n"
"Modifikacni parametry:\n"
"-h - napoveda\n";

class CCommunication;

// Trida implementuje metody, ktere server provadi nad soketem.
// =======================================================================================
class CSocket {
  private:
    struct sockaddr_in sin;  // info o delce struktury, adresni rodine, portu, IP adrese (pri naslouchani jakakoliv)
    struct sockaddr_in cin;  // info o pripojenem klientovi
    socklen_t cin_len;	     
    int ws;		     // welcome socket
    int comms;		     // komunikacni soket. 
  
  public:
    CSocket();		      // konstruktor
    void SetPort(int port) {sin.sin_port = htons(port);}
    void GetSocket();
    void Bind();
    void Listen();
    int Accept();
    void CloseWs() {close(ws);}
    void CloseComms() {close(comms);}
    void SendMsg();
    friend class CCommunication;
};

// Konstruktor
CSocket::CSocket() {
  cin_len = sizeof(cin);
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
}

// Tvorba soketu.
// AF_INET - komuniakcni domena, rodina protokolu (dle manualu AF_* == PF_*)
// SOCK_STREAM - odpovida TCP transport. protokolu
// 0 - oznaceni IP protokolu
inline void CSocket::GetSocket() {
  if((ws = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    throw(runtime_error("CHYBA(socket): Nelze otevrit soket."));
}

// Navazani socketu na zvoleny port.
// pretypovani struct sockaddr * - je to obecna struktura, musi se na ni pretypovat,
// protoze tento typ ockeva funkce. viz bind(2).
inline void CSocket::Bind() {
  if(bind(ws, reinterpret_cast<struct sockaddr*>(&sin), sizeof(sin)) < 0)
    throw(runtime_error("CHYBA(bind): Nelze svazat soket s portem."));
}

// Prepnuti soketu do pasivniho cekani, server ceka na spojeni
// 1 - delka fornty uplnych (klienti zadajici o TCP pripojnei) a neuplnych (klienti 
// s uz ustavenym TCP spojenim) spojeni
//TODO jaka je vhodna velikost fronty v pripde konkurentiho serveru? (protoze i fork neco trva)
inline void CSocket::Listen() {
  if(listen(ws, 10) < 0)
    throw(runtime_error("CHYBA(listen): Server nemuze naslouchat."));
}

// prijeti spojeni, ustaveni komunikace
inline int CSocket::Accept() {
  comms = accept(ws, reinterpret_cast<struct sockaddr*>(&cin), &cin_len);
  return comms;
}

// Trida pro komunikaci s klientem
// =======================================================================================
class CCommunication {
  private:
    char msgRcv[MAX_MSG_LEN];
    char *hostPtr;
    char ip4[MAX_IP4_LEN];
    char ip6[MAX_IP6_LEN];
    int options[2];		// volba rezoluce IPv4 a IPv6 adresy
    string msgSnd;
  public:
    CCommunication();		//konstruktor
    void Read(CSocket & objSock);
    void Parse();
    void DNSresol();
    void Send(CSocket & objSock);
};

/* Konstruktor */
CCommunication::CCommunication() {
  hostPtr = NULL;
  options[0] = options[1] = 0;
  memset(msgRcv, '\0', sizeof(msgRcv));
  memset(ip4, '\0', sizeof(ip4));
  memset(ip6, '\0', sizeof(ip6));
}

/* Metoda precte zpravu od klienta. Zprava je zapsana v mnou navrzenem
 * komunikacnim protkolou */
void CCommunication::Read(CSocket & objSock) {
  int n;
  
  if((n = read(objSock.comms, msgRcv, MAX_MSG_LEN-1)) < 0) 
    throw(runtime_error("CHYBA(read): Nastala chyba pri cteni zpravy od klienta."));
}

/* Metoda z prijate zpravy zjisti nazev hosta a volby pro preklad adres. */
void CCommunication::Parse() {
  char *aux = NULL;
  char *end;
  int type = -1;
  
  // typ adres(y) (4 - IPv4, 6 - IPv6, 10 - IPv4 i IPv6)
  if((aux = strstr(msgRcv, "Type:")) == NULL) throw(runtime_error("CHYBA: Komunikacni protokol"));
  aux += strlen("Type:");
  if(((type = strtol(aux, &end, 10)) == -1) || (*end != '\r')) throw(runtime_error("CHYBA: Komunikacni protokol"));
  
  if(type == 4 || type == 10) options[0] = IPV4;
  if(type == 6 || type == 10) options[1] = IPV6;
  
  // ukazatel na retezec se jmenem hosta
  aux = NULL;
  if((hostPtr = strstr(msgRcv, "Host:")) == NULL) throw(runtime_error("CHYBA: Komunikacni protokol"));
  hostPtr += strlen("Host:");
  if((aux = strchr(hostPtr, '\r')) == NULL) throw(runtime_error("CHYBA: Komunikacni protokol"));
  *aux = '\0';
}

/* Metoda provede rezoluci domenoveho jmena na IPv4 a IPv6 adresu. */
void CCommunication::DNSresol() {
  int s;
  struct addrinfo hints;
  struct addrinfo *res;
  char buf[MAX_IP6_LEN];
  void *ptr = NULL;
  
  for(int i = 0; i < 2; i++) {
    if(options[i] == 0) continue;
    
    memset (&hints, 0, sizeof (hints));
    memset (&buf, '\0', sizeof (buf));
    hints.ai_socktype = SOCK_STREAM;
    options[i] == IPV4 ?  (msgSnd += "IPv4:", hints.ai_family = AF_INET) : 
			  (msgSnd += "IPv6:", hints.ai_family = AF_INET6);

    
    // reoluce se nezdarila
    if((s = getaddrinfo(hostPtr, NULL, &hints, &res)) != 0) {
      msgSnd += "-";
    // rezoluce se zdarila
    } else {
      options[i] == IPV4 ?  (ptr = &(((struct sockaddr_in *)res->ai_addr)->sin_addr)) :
			    (ptr = &(((struct sockaddr_in6 *)res->ai_addr)->sin6_addr));
      inet_ntop(res->ai_family, ptr, buf, sizeof(buf));
      freeaddrinfo(res); res = NULL;
      msgSnd += buf;
    }
    msgSnd += "\r\n";
  }
}

/* Metoda zasle vysledek rezoluce klientovi*/
void CCommunication::Send(CSocket & objSock) {
  if(write(objSock.comms, msgSnd.c_str(), msgSnd.length()) <= 0) 
    throw(runtime_error("CHYBA(write): Nelze zaslat zpravu klientovi."));
}

// Trida pro zracovani parametru.
// =======================================================================================
class CParams {
  private:
    long port;
  public:
    CParams(): port(-1) {};	// konstruktor
    void ProcessParams(int argc, char **argv);
    long GetPort() {return port;}
};

/* Metoda pro zpracovani parametru 
 */
void CParams::ProcessParams(int argc, char **argv) {
  char *end;
  
  if(argc == 2 && strcmp(argv[1], "-h") == 0) throw(runtime_error(helpMsg));
  
  if((argc != 3) || strcmp(argv[1], "-p")) 
    throw(runtime_error("CHYBA: Nespravne zadane parametry."));
    
  if(((port = strtol(argv[2], &end, 10)) == -1) || (*end != '\0')) 
    throw(runtime_error("CHYBA: Nespravne zadane parametry."));
  
  if(port < PORT_MIN || port > PORT_MAX)
    throw(runtime_error("CHYBA: Nespravne zadane parametry."));
}

void myExit(int s) {
  s == SIGINT ? exit(0) : exit(s);
}

//--------------------------------------------------------------------------------------//
int main(int argc, char **argv) {
  try {
    // Ziskani cisla portu.
    CParams params;
    params.ProcessParams(argc, argv);
  
    CSocket objSock;
    objSock.SetPort(params.GetPort());
  
    // Odebirani navratovych kodu potomku
    // masku zrejme nepotrebuji, ale valgrindu se nelibi, kdyz ji nevyplnim.
    sigset_t mask;
    sigemptyset(&mask);
    struct sigaction actionChld;
    actionChld.sa_handler = NULL;
    actionChld.sa_flags = SA_NOCLDWAIT;	// z ukoncenych dcerinnych procesu nebudou zombie.
    actionChld.sa_mask = mask;
    sigaction(SIGCHLD, &actionChld, NULL);
    // Ukonceni serveru pres SIGINT (Ctrl-C)
    struct sigaction actionInt;
    actionInt.sa_handler = myExit;
    actionInt.sa_flags = 0;
    actionInt.sa_mask = mask;
    sigaction(SIGINT, &actionInt, NULL);

    objSock.GetSocket();	// Tvorba welcome soketu.
    objSock.Bind();		// Navazani oketu an port.
    objSock.Listen();		// Nastaveni naslouchani na danem portu.
  
    // Opakovane vyrizovani pozadavku klientu
    while(1) {    
      if(objSock.Accept() < 0) continue;
    
      // Tvorba potomku
      int pid = fork();
      // chyba
      if(pid < 0) {throw(runtime_error("CHYBA(fork): Nelze vytvorit novy proces."));}
      // proces potomka
      if(pid == 0) {
	objSock.CloseWs();
	CCommunication objCom;

	objCom.Read(objSock);	// Cteni zpravy od klienta.
	objCom.Parse();		// Ziskani jmena hosta a voleb.
	objCom.DNSresol();	// DNS rezoluce.
	objCom.Send(objSock);	// Zaslani zpravy s vysledkem klientovi.
	
	objSock.CloseComms();
	exit(0);
      }
      // puvodni rodicovsky proces
      else {
	objSock.CloseComms();
      }
    }
  } catch(runtime_error &e) {
    cerr << e.what() << endl;
    exit(1);
  }

  return 0;
}
