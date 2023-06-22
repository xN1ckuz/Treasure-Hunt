#pragma once
#ifndef __ALBEROKD_H__
#define __ALBEROKD_H__

#include <vector>
#include <numeric>
#include <algorithm>
#include <exception>
#include <functional>

namespace kd
{
    template <class PuntoT>
    class AlberoKD
    {
    public:
        AlberoKD() : radice_(nullptr) {};
        AlberoKD(const std::vector<PuntoT>& punti) : radice_(nullptr) { costruisciAlbero(punti); }

        ~AlberoKD() { eliminaAlbero(); }

        void creaAlbero(const std::vector<PuntoT>& punti) {
            radice_ = nullptr;
            costruisciAlbero(punti);
        }

        void costruisciAlbero(const std::vector<PuntoT>& punti)
        {
            eliminaAlbero();

            punti_ = punti;

            std::vector<int> indici(punti.size());
            std::iota(std::begin(indici), std::end(indici), 0);

            radice_ = costruisciAlberoRicorsivo(indici.data(), (int)punti.size(), 0);
        }

        void eliminaAlbero()
        {
            eliminaAlberoRicorsivo(radice_);
            radice_ = nullptr;
            punti_.clear();
        }

        bool validaAlbero() const
        {
            try
            {
                validaAlberoRicorsivo(radice_, 0);
            }
            catch (const Eccezione&)
            {
                return false;
            }

            return true;
        }

        int trovaVicinoPiuProssimo(const PuntoT& query, double* distMin = nullptr) const
        {
            int ipotesi;
            double _distMin = std::numeric_limits<double>::max();

            trovaVicinoPiuProssimoRicorsivo(query, radice_, &ipotesi, &_distMin);

            if (distMin)
                *distMin = _distMin;

            return ipotesi;
        }

        std::vector<int> trovaKViciniPiuProssimi(const PuntoT& query, int k) const
        {
            CodaViciniPiuProssimi coda(k);
            trovaKViciniPiuProssimiRicorsivo(query, radice_, coda, k);

            std::vector<int> indici(coda.size());
            for (size_t i = 0; i < coda.size(); i++)
                indici[i] = coda[i].second;

            return indici;
        }

        std::vector<int> trovaPuntiEntroRaggio(const PuntoT& query, double raggio) const
        {
            std::vector<int> indici;
            trovaPuntiEntroRaggioRicorsivo(query, radice_, indici, raggio);
            return indici;
        }

        PuntoT getPuntoVicino(const PuntoT& query, double* distMin = nullptr) {
            int indice = trovaVicinoPiuProssimo(query, distMin);
            return punti_[indice];
        }

        vector<PuntoT> getKPuntiPiùVicini(const PuntoT& query, int k) {
            vector <PuntoT> listaPunti;
            vector<int> indici = trovaKViciniPiuProssimi(query, k);
            for (int i = 0; i < indici.size(); i++) {
                listaPunti.push_back(punti_[indici[i]]);
            }
            return listaPunti;
        }

        vector<PuntoT> getPuntiEntroRaggio(const PuntoT& query, double raggio) {
            vector <PuntoT> listaPunti;
            vector<int> indici = trovaPuntiEntroRaggio(query, raggio);
            for (int i = 0; i < indici.size(); i++) {
                listaPunti.push_back(punti_[indici[i]]);
            }
            return listaPunti;
        }

    private:

        struct Nodo
        {
            int idx;       //!< indice del punto originale
            Nodo* sinistro;    //!< puntatore al nodo figlio sinistro
            Nodo* destro;   //!< puntatore al nodo figlio destro
            int asse;      //!< asse della dimensione

            Nodo() : idx(-1), sinistro(nullptr), destro(nullptr), asse(-1) {}
        };

        class Eccezione : public std::exception { using std::exception::exception; };

        template <class T, class Comparatore = std::less<T>>
        class CodaConPrioritaLimitata
        {
        public:

            CodaConPrioritaLimitata() = delete;
            CodaConPrioritaLimitata(size_t limite) : limite_(limite) { elementi_.reserve(limite + 1); };

            void inserisci(const T& valore)
            {
                auto it = std::find_if(std::begin(elementi_), std::end(elementi_),
                    [&](const T& elemento) { return Comparatore()(valore, elemento); });
                elementi_.insert(it, valore);

                if (elementi_.size() > limite_)
                    elementi_.resize(limite_);
            }

            const T& ultima() const { return elementi_.back(); };
            const T& operator[](size_t indice) const { return elementi_[indice]; }
            size_t size() const { return elementi_.size(); }

        private:
            size_t limite_;
            std::vector<T> elementi_;
        };

        using CodaViciniPiuProssimi = CodaConPrioritaLimitata<std::pair<double, int>>;

        Nodo* costruisciAlberoRicorsivo(int* indici, int npunti, int profondita)
        {
            if (npunti <= 0)
                return nullptr;

            const int asse = profondita % PuntoT::DIM;
            const int medio = (npunti - 1) / 2;

            std::nth_element(indici, indici + medio, indici + npunti, [&](int sinistro, int destro)
                {
                    return punti_[sinistro][asse] < punti_[destro][asse];
                });

            Nodo* nodo = new Nodo();
            nodo->idx = indici[medio];
            nodo->asse = asse;

            nodo->sinistro = costruisciAlberoRicorsivo(indici, medio, profondita + 1);
            nodo->destro = costruisciAlberoRicorsivo(indici + medio + 1, npunti - medio - 1, profondita + 1);

            return nodo;
        }

        void eliminaAlberoRicorsivo(Nodo* nodo)
        {
            if (nodo == nullptr)
                return;

            if (nodo->sinistro)
                eliminaAlberoRicorsivo(nodo->sinistro);

            if (nodo->destro)
                eliminaAlberoRicorsivo(nodo->destro);

            delete nodo;
        }

        void validaAlberoRicorsivo(const Nodo* nodo, int profondita) const
        {
            if (nodo == nullptr)
                return;

            const int asse = nodo->asse;
            const Nodo* nodoSinistro = nodo->sinistro;
            const Nodo* nodoDestro = nodo->destro;

            if (nodoSinistro && nodoDestro)
            {
                if (punti_[nodo->idx][asse] < punti_[nodoSinistro->idx][asse])
                    throw Eccezione();

                if (punti_[nodo->idx][asse] > punti_[nodoDestro->idx][asse])
                    throw Eccezione();
            }

            if (nodoSinistro)
                validaAlberoRicorsivo(nodoSinistro, profondita + 1);

            if (nodoDestro)
                validaAlberoRicorsivo(nodoDestro, profondita + 1);
        }

        static double calcolaDistanza(const PuntoT& p, const PuntoT& q)
        {
            double distanza = 0;
            for (size_t i = 0; i < PuntoT::DIM; i++)
                distanza += ((p[i] - q[i]) * (p[i] - q[i]));
            return sqrt(distanza);
        }

        void trovaVicinoPiuProssimoRicorsivo(const PuntoT& query, const Nodo* nodo, int* supposizione, double* distanzaMinima) const
        {
            if (nodo == nullptr)
                return;

            const PuntoT& allenamento = punti_[nodo->idx];

            const double distanza = calcolaDistanza(query, allenamento);
            if (distanza < *distanzaMinima)
            {
                *distanzaMinima = distanza;
                *supposizione = nodo->idx;
            }

            const int asse = nodo->asse;
            const int dir = query[asse] < allenamento[asse] ? 0 : 1;
            trovaVicinoPiuProssimoRicorsivo(query, dir == 0 ? nodo->sinistro : nodo->destro, supposizione, distanzaMinima);

            const double differenza = fabs(query[asse] - allenamento[asse]);
            if (differenza < *distanzaMinima)
                trovaVicinoPiuProssimoRicorsivo(query, dir == 0 ? nodo->destro : nodo->sinistro, supposizione, distanzaMinima);
        }

        void trovaKViciniPiuProssimiRicorsivo(const PuntoT& query, const Nodo* nodo, CodaViciniPiuProssimi& coda, int k) const
        {
            if (nodo == nullptr)
                return;

            const PuntoT& allenamento = punti_[nodo->idx];

            const double distanza = calcolaDistanza(query, allenamento);
            coda.inserisci(std::make_pair(distanza, nodo->idx));

            const int asse = nodo->asse;
            const int dir = query[asse] < allenamento[asse] ? 0 : 1;
            trovaKViciniPiuProssimiRicorsivo(query, dir == 0 ? nodo->sinistro : nodo->destro, coda, k);

            const double differenza = fabs(query[asse] - allenamento[asse]);
            if (differenza < coda.ultima().first)
                trovaKViciniPiuProssimiRicorsivo(query, dir == 0 ? nodo->destro : nodo->sinistro, coda, k);
        }

        void trovaPuntiEntroRaggioRicorsivo(const PuntoT& query, const Nodo* nodo, std::vector<int>& indici, double raggio) const
        {
            if (nodo == nullptr)
                return;

            const PuntoT& allenamento = punti_[nodo->idx];

            const double distanza = calcolaDistanza(query, allenamento);
            if (distanza <= raggio)
                indici.push_back(nodo->idx);

            const int asse = nodo->asse;
            const int dir = query[asse] < allenamento[asse] ? 0 : 1;
            trovaPuntiEntroRaggioRicorsivo(query, dir == 0 ? nodo->sinistro : nodo->destro, indici, raggio);

            const double differenza = fabs(query[asse] - allenamento[asse]);
            if (differenza <= raggio)
                trovaPuntiEntroRaggioRicorsivo(query, dir == 0 ? nodo->destro : nodo->sinistro, indici, raggio);
        }

        Nodo* radice_;
        std::vector<PuntoT> punti_;
    };
}

#endif  // __ALBEROKD_H__
