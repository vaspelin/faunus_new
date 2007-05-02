#ifndef _markovmove_h
#define _markovmove_h

#include "container.h"
#include "potentials.h"
#include "ensemble.h"
#include "titrate.h"
#include "slump.h"
typedef pot_coulomb T_pairpot;

/*! \brief Base class for MC moves
 *  \author Mikael Lund
 *  \todo Perhaps the P_pairpot could be made more elegant?
 *
 *  This class will keep track of the returned energy, if the
 *  move was successful of not and can provide statistics about the
 *  accepted and rejected moves.
 *
 *  Each derived class should provide a simple move() function that will
 *  perform a trial move, calculate the energy and either accept or reject
 *  the move. Unsuccessful moves are automatically undone.
 *
 *  The used pair-potential is identified using the
 *  type \verb T_pairpot \endverbatim that must be
 *  defined before processing the source code. For example,
 *
 *  \code
 *  #include "potentials.h"
 *  typedef pot_coulomb T_pairpot
 *  #include "markovmove.C"
 *  \endcode
 */
class markovmove {
  private:
    slump slp;
  protected:
    double uold, unew, deltadp;
    unsigned long long int cnt, naccept;
    string name;
    container *con;
    ensemble *ens;
    interaction<T_pairpot> *pot;
  public:
    enum keys {OK, ENERGY, HC};
    keys rc;                            //!< Return code from move() functions
    double dp,                          //!< Displacement parameter
           du,                          //!< Energy change of last move
           utot;                        //!< Sum of energy changes for all moves
    float accepted();                   //!< Return fraction of accepted moves
    bool run(float);                    //!< Probability
    void adjust_dp(float=30, float=40); //!< Adjust displacement parameter
    virtual string info();              //!< Show info about group 
    markovmove(ensemble &e, container &c, interaction<T_pairpot> &inter) {
      du=utot=dp=deltadp=0;
      cnt=naccept=0;
      ens=&e;
      con=&c;
      pot=&inter;
    }
};
string markovmove::info() {
  ostringstream o;
  o << "# " << name << ":" << endl
    << "#   Acceptance          = " << accepted()*100 << endl
    << "#   Number of trials    = " << cnt << endl
    << "#   Displacement param. = " << dp << endl
    << "#   Total energy change = " << utot << endl;
  return o.str();
}

/*! \brief Move salt particles
 *  \author Mikael Lund
 */
class saltmove : public markovmove {
  public:
    saltmove( ensemble &, container&, interaction<T_pairpot>& );
    void move(short);           //!< Move a single particle
    void move(group &);         //!< Loop over group particles (randomly)
};

/*! \brief Symmetrically move two groups along z-axis
 *  \author Mikael Lund
 */
class dualzmove : public markovmove {
  public:
    float z;    //!< Distance between CM's of the groups
    void move(group &, group &);
};

/*! \brief Rotate group around mass-center.
*/
class rotate : public markovmove { 
};

/*! \brief Titrate all titrateable sites
 *  \author Mikael Lund
 */
class chargereg : public markovmove, private titrate {
  public:
    chargereg( ensemble&, container&, interaction<T_pairpot>&, group&, float);
    void titrateall();
    string info();
};

#endif
