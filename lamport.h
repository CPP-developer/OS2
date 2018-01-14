#include "funkcije.h"


using namespace std;

int main(int argc, char **argv)
{
    bool res;

    bool cekanjeNaKO, dogadjaj;

    unsigned int i, procesId;
    unsigned char znamenka = 1;

    unsigned short brojProcesa=0;
    bool dretvePetlje=true;
    unsigned int lokalniSat=0;

    chrono::milliseconds vrijeme_staro, vrijeme_trenutno;

    queue<Poruka> redPoruka;
    deque<Poruka> redZahtijeva;
    deque<Poruka>::iterator it;

    Poruka poruka;
    unsigned short brojOdgovora;

    char ispis[80];

    int uticnica;
    if(argc<2)
    {
        cout << "Koristenje: " << basename(argv[0]) << " T10 T20 T30 .. .@ T11 T12 @  T21 T22 @ ..." << endl;
        return 0;
    }
    vector< deque<unsigned int> > kriticnaVremena = parsirajArgumente(argv+1, argc-1);
    for(i=0; i< kriticnaVremena.size(); i++)
        if(kriticnaVremena[i].size()>2)
            sort(kriticnaVremena[i].begin()+1,kriticnaVremena[i].end());
    fflush(stdout);

    procesId=getpid();
    for(i=0; i<kriticnaVremena.size()-1&&procesId>0; i++, brojProcesa++)
        procesId=fork();
    if(procesId>0)
        brojProcesa=0;
    procesId=getpid();
    res = kreirajUticnicu(htons(10000+znamenka*10+brojProcesa), uticnica);
    if(res==false || uticnica<0)
    {
        cerr << "pid: " << getpid() << ", rb: " << brojProcesa << "; ne mogu otvoriti uticnicu na portu " << 10000+znamenka*10+brojProcesa << "." << endl;
        return 0;
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        cerr << "Ne mogu pokrenuti hvatanje signala (CTRL/C)." << endl;
        close(uticnica);
        return 0;
    }

    lokalniSat = kriticnaVremena[brojProcesa].front();
    if(!kriticnaVremena[brojProcesa].empty())
        kriticnaVremena[brojProcesa].pop_front();

    ispisiDogadjaj(procesId, brojProcesa, lokalniSat, "Pokrenut");
    thread primanjePorukaDretva([&redPoruka, brojProcesa, uticnica, &dretvePetlje]()
                                {citajPoruke(redPoruka, brojProcesa, uticnica, dretvePetlje);});


    if(brojProcesa==kriticnaVremena.size()-1)
    {
        this_thread::sleep_for(chrono::milliseconds(100));
        cout << "=== Svi procesi miruju 2 sekunde ===" << endl;
    }
    this_thread::sleep_for(chrono::milliseconds(2000));

    /* glavna petlja programa */
    signalizirano = false;
    cekanjeNaKO=false;
    brojOdgovora = 0;
    vrijeme_trenutno=chrono::duration_cast< chrono::milliseconds >(chrono::system_clock::now().time_since_epoch());
    vrijeme_staro=vrijeme_trenutno;
    srand(time(0)); rand(); rand(); rand();
    while(!signalizirano)
    {
        dogadjaj=false;
        /* pracenje liste vremena za ulazak u KO */
        if(!cekanjeNaKO&&!kriticnaVremena[brojProcesa].empty()&&lokalniSat>=kriticnaVremena[brojProcesa].front())
        {
            sprintf(ispis, "Vrijeme KO T[KO]=%d",kriticnaVremena[brojProcesa].front());
            ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis, BojaIspisa::ZUTO);
            kriticnaVremena[brojProcesa].pop_front();
            brojOdgovora = 0;
            poruka.tip = TipPoruke::ZAHTIJEV;
            poruka.i = brojProcesa;
            poruka.T = lokalniSat;
            redZahtijeva.push_back(poruka);
            sort(redZahtijeva.begin(), redZahtijeva.end(), usporedba);
            posaljiSvima(uticnica,brojProcesa,poruka,kriticnaVremena.size(),znamenka);
            cekanjeNaKO=true;

            dogadjaj=true;
            sprintf(ispis, "Zahtijev svima: Z(%d,T[%d]=%d)",poruka.i,poruka.i,poruka.T);
            ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis);
        }
        /* obrada pristiglih poruka */
        while(!redPoruka.empty())
        {
            poruka = redPoruka.front();
            redPoruka.pop();
            dogadjaj = true;
            switch(poruka.tip)
            {
                case TipPoruke::ZAHTIJEV:
                    sprintf(ispis, "Primljen zahtijev: Z(%d,T[%d]=%d)",poruka.i,poruka.i,poruka.T);
                    ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis);

                    lokalniSat=(lokalniSat>=poruka.T)?lokalniSat+1:poruka.T+1;
                    redZahtijeva.push_back(poruka);
                    sort(redZahtijeva.begin(), redZahtijeva.end(), usporedba);
                    poruka.T=lokalniSat;
                    poruka.tip=TipPoruke::ODGOVOR;
                    posaljiPoruku(uticnica,poruka.i,poruka,znamenka);
                    sprintf(ispis, "Poslan odgovor: O(%d,T[%d]=%d)",poruka.i,poruka.i,poruka.T);
                    ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis);
                    break;
                case TipPoruke::IZLAZAK:
                    sprintf(ispis, "Primljen izlazak: I(%d,T[%d]=%d)",poruka.i,poruka.i,poruka.T);
                    ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis);
                    for(it=redZahtijeva.begin(); it!=redZahtijeva.end(); ++it)
                        if(it->i==poruka.i)
                        {
                            redZahtijeva.erase(it);
                            break;
                        }
                    break;

                case TipPoruke::ODGOVOR:
                    sprintf(ispis, "Primljen odgovor: O(%d,T[%d]=%d)",poruka.i,poruka.i,poruka.T);
                    ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis);

                    lokalniSat=(lokalniSat>=poruka.T)?lokalniSat+1:poruka.T+1;
                    brojOdgovora++;
                    break;
                default:;
            }
        }

        /* provjera uvjeta za ulazak u KO */
        if(cekanjeNaKO)
        {
            if((brojOdgovora==kriticnaVremena.size()-1)&&(redZahtijeva.front().i==brojProcesa)&&!signalizirano)
            {
                sprintf(ispis, "Ulazak KO \t\t\t--- %d",brojProcesa);
                ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis, BojaIspisa::ZELENO);
                for(int j=0; j<3; j++)
                {
                    sprintf(ispis, "Rad u KO - dovrseno %d%%",(j+1)*25);
                    ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis, BojaIspisa::CRVENO);
                    this_thread::sleep_for(chrono::milliseconds(1000));
                }
                sprintf(ispis, "Izlazak KO\t\t\t--- %d",brojProcesa);
                ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis, BojaIspisa::PLAVO);
                dogadjaj = true;

                cekanjeNaKO = false;
                poruka.i=redZahtijeva.front().i;
                poruka.T=redZahtijeva.front().T;
                poruka.tip=TipPoruke::IZLAZAK;
                redZahtijeva.pop_front();
                posaljiSvima(uticnica,brojProcesa,poruka,kriticnaVremena.size(),znamenka);

                sprintf(ispis, "Izlazak svima I(%d,T[%d]=%d)",poruka.i,poruka.i,poruka.T);
                ispisiDogadjaj(procesId, brojProcesa, lokalniSat, ispis);
            }
        }
        vrijeme_trenutno=chrono::duration_cast< chrono::milliseconds >(chrono::system_clock::now().time_since_epoch());
        if(!dogadjaj&&(vrijeme_trenutno-vrijeme_staro).count()>=1000)
        {
            lokalniSat++;
            ispisiDogadjaj(procesId, brojProcesa, lokalniSat, "Interni dogadjaj");
            vrijeme_staro=vrijeme_trenutno;
        }
        this_thread::sleep_for(chrono::milliseconds(100+rand()%100));
        //ispisiDogadjaj(procesId, brojProcesa, lokalniSat, "Lokalni dogadjaj");
    }
    /* salje poruku prekida glavnom procesu */

    if(brojProcesa>0)
    {
        ispisiDogadjaj(procesId, brojProcesa, lokalniSat, "Signal prekida",BojaIspisa::PLAVO);
        poruka.i=brojProcesa;
        poruka.T = lokalniSat;
        poruka.tip = TipPoruke::SIGNAL;
        posaljiPoruku(uticnica,0,poruka,znamenka);
    }
    else
    {
        ispisiDogadjaj(procesId, brojProcesa, lokalniSat, "Signal prekida - ceka ostale",BojaIspisa::PLAVO);
        brojOdgovora=0;
        while(brojOdgovora<kriticnaVremena.size()-2)
        {
            while(!redPoruka.empty())
            {
                if(redPoruka.front().tip==TipPoruke::SIGNAL)
                    brojOdgovora++;
                redPoruka.pop();
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
    }
    dretvePetlje=false;
    poruka.i=brojProcesa;
    poruka.T=lokalniSat;
    poruka.tip = TipPoruke::PREKID;
    if(brojProcesa==0)
        posaljiSvima(uticnica,-1,poruka,kriticnaVremena.size(),znamenka);
    primanjePorukaDretva.join();
    unistiUticnicu(uticnica);

    if(brojProcesa>0)
        cout << "Proces dijete, pid: " << procesId << ", rb: " << brojProcesa << " je zavrsio." << endl;
    else
    {
        this_thread::sleep_for(chrono::milliseconds(2000));
        cout << "Glavni proces, pid: " << procesId << ", rb: " << brojProcesa << " je zavrsio." << endl;
        cout << "=== KRAJ PROGRAMA ===" << endl;
    }
    return 0;
}
