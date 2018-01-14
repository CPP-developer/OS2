#ifndef FUNKCIJE_H_INCLUDED
#define FUNKCIJE_H_INCLUDED

#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <queue>
#include <signal.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <mutex>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

extern bool signalizirano;

enum TipPoruke:unsigned char{PREKID=0, ZAHTIJEV=1, ODGOVOR=2, IZLAZAK=3, SIGNAL=4};
enum BojaIspisa:unsigned char{NORMALNO=0, CRVENO=1, ZUTO=2, ZELENO=3, PLAVO=4};

struct Poruka
{
    TipPoruke tip;
    unsigned short i;
    unsigned int T;
};

std::vector< std::deque<unsigned int> > parsirajArgumente(char **arg, unsigned int velicina);

bool usporedba(Poruka p1, Poruka p2);
bool kreirajUticnicu(in_port_t port, int& uticnica);
void unistiUticnicu(int uticnica);

void citajPoruke(std::queue<Poruka>& redPoruka, unsigned short rb, int uticnica, bool& vrtnjaPetlje);
void posaljiPoruku(int uticnica, unsigned short dest, const Poruka& p, unsigned char znamenka);
void posaljiSvima(int uticnica, unsigned short rb, const Poruka& p, unsigned short ukupnoProcesa, unsigned char znamenka);

void ispisiDogadjaj(unsigned int pid, unsigned short rb, unsigned int lokSat, const char poruka[], BojaIspisa boja=BojaIspisa::NORMALNO);
void ispisiInfoUticnice(int uticnica, unsigned short rb);
void sig_handler(int signo);
#endif // LAMPORT_H_INCLUDED
