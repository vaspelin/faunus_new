#pragma once

#include "core.h"
#include "energy.h"
#include "average.h"
#include "mpi.h"

namespace Faunus {
namespace Move {

class Movebase {
  private:
    virtual void _move(Change &) = 0;                          //!< Perform move and modify change object
    virtual void _accept(Change &);                            //!< Call after move is accepted
    virtual void _reject(Change &);                            //!< Call after move is rejected
    virtual void _to_json(json &j) const = 0;                  //!< Extra info for report if needed
    virtual void _from_json(const json &j) = 0;                //!< Extra info for report if needed
    TimeRelativeOfTotal<std::chrono::microseconds> timer;      //!< Timer for whole move
    TimeRelativeOfTotal<std::chrono::microseconds> timer_move; //!< Timer for _move() only
  protected:
    unsigned long cnt = 0;
    unsigned long accepted = 0;
    unsigned long rejected = 0;

  public:
    static Random slump; //!< Shared for all moves
    std::string name;    //!< Name of move
    std::string cite;    //!< Reference
    int repeat = 1;      //!< How many times the move should be repeated per sweep

    void from_json(const json &j);
    void to_json(json &j) const; //!< JSON report w. statistics, output etc.
    void move(Change &change);   //!< Perform move and modify given change object
    void accept(Change &c);
    void reject(Change &c);
    virtual double bias(Change &, double uold,
                        double unew); //!< adds extra energy change not captured by the Hamiltonian
    inline virtual ~Movebase() = default;
};

void from_json(const json &j, Movebase &m); //!< Configure any move via json
void to_json(json &j, const Movebase &m);

/**
 * @brief Swap the charge of a single atom
 */
class AtomicSwapCharge : public Movebase {
  private:
    typedef typename Space::Tpvec Tpvec;
    typedef typename Space::Tparticle Tparticle;
    Space &spc; // Space to operate on
    int molid = -1;
    double ln10 = log(10);
    double pKa, pH;
    Average<double> msqd; // mean squared displacement
    double _sqd, _bias;   // squared displament
    std::string molname;  // name of molecule to operate on
    Change::data cdata;

    void _to_json(json &j) const override;
    void _from_json(const json &j) override; //!< Configure via json object
    typename Tpvec::iterator randomAtom();
    void _move(Change &change) override;
    double bias(Change &, double, double) override; //!< adds extra energy change not captured by the Hamiltonian
    void _accept(Change &) override;
    void _reject(Change &) override;

  public:
    AtomicSwapCharge(Space &spc);
};

/**
 * @brief Translate and rotate a molecular group
 */
class AtomicTranslateRotate : public Movebase {
  protected:
    typedef typename Space::Tpvec Tpvec;
    Space &spc; // Space to operate on
    int molid = -1;
    Point dir = {1, 1, 1};
    Average<double> msqd; // mean squared displacement
    double _sqd;          // squared displament
    std::string molname;  // name of molecule to operate on
    Change::data cdata;

    void _to_json(json &j) const override;
    void _from_json(const json &j) override; //!< Configure via json object
    std::vector<Particle>::iterator randomAtom();

    /**
     * @brief translates a single particle.
     */
    virtual void translateParticle(typename Tpvec::iterator p, double dp);
    void _move(Change &change) override;
    void _accept(Change &) override;
    void _reject(Change &) override;

  public:
    AtomicTranslateRotate(Space &spc);
};

/**
 * @brief Translate and rotate an atom on a 2D hypersphere-surface
 * @todo under construction
 */
/*
template<typename Tspace>
   class Atomic2dTranslateRotate : public AtomicTranslateRotate {
       protected:
           typedef AtomicTranslateRotate base;
           using base::spc;

           void translateParticle(Space::Tpvec::iterator p, double dp) override {
               auto &g = spc.groups[base::cdata.index];
               Point oldpos = p->pos;

               Point rtp = xyz2rtp(p->pos); // Get the spherical coordinates of the particle
               double slump_theta = dp * (base::slump() - 0.5); // Get random theta-move
               double slump_phi = dp * (base::slump() - 0.5);   // Get random phi-move

               double scalefactor_theta = spc.geo.getRadius() * sin(rtp.z()); // Scale-factor for theta
               double scalefactor_phi = spc.geo.getRadius();                  // Scale-factor for phi

               Point theta_dir = Point(-sin(rtp.y()), cos(rtp.y()), 0); // Unit-vector in theta-direction
               Point phi_dir = Point(cos(rtp.y()) * cos(rtp.z()), sin(rtp.y()) * cos(rtp.z()),
                                     -sin(rtp.z())); // Unit-vector in phi-direction
               Point xyz = oldpos + scalefactor_theta * theta_dir * slump_theta +
                           scalefactor_phi * phi_dir * slump_phi; // New position
               p->pos = spc.geo.getRadius() * xyz / xyz.norm();   // Convert to cartesian coordinates

               spc.geo.boundary(p->pos);
               base::_sqd = spc.geo.sqdist(oldpos, p->pos); // squared displacement
               if (not g.atomic) {                          // recalc mass-center for non-molecular groups
                   g.cm = Geometry::massCenter(g.begin(), g.end(), spc.geo.getBoundaryFunc(), -g.cm);
#ifndef NDEBUG
                   Point cmbak = g.cm;                             // backup mass center
                   g.translate(-cmbak, spc.geo.getBoundaryFunc()); // translate to {0,0,0}
                   double should_be_zero = spc.geo.sqdist({0, 0, 0}, Geometry::massCenter(g.begin(),
g.end())); if (should_be_zero > 1e-6) throw std::runtime_error("atomic move too large"); else g.translate(cmbak,
spc.geo.getBoundaryFunc()); #endif
               }
           }

       public:
           Atomic2dTranslateRotate(Tspace &spc) : base(spc) {
               base::name = "transrot 2d";
           }
   };*/

/**
 * @brief Translate and rotate a molecular group
 */
class TranslateRotate : public Movebase {
  protected:
    typedef typename Space::Tpvec Tpvec;
    Space &spc; // Space to operate on
    int molid = -1;
    double dptrans = 0;
    double dprot = 0;
    Point dir = {1, 1, 1};
    double _sqd;          // squared displacement
    Average<double> msqd; // mean squared displacement

    void _to_json(json &j) const override;
    void _from_json(const json &j) override; //!< Configure via json object
    void _move(Change &change) override;
    void _accept(Change &) override { msqd += _sqd; }
    void _reject(Change &) override { msqd += 0; }

  public:
    TranslateRotate(Space &spc);
};

/**
 * @brief Translate and rotate molecular groups preferentially, depending on wether they are inside or outside geometry (e.g. ellipsoid)
 */
class BiasedTranslateRotate : public Movebase {
  protected:
    typedef typename Space::Tpvec Tpvec;
    Space &spc;

    int molid = -1, refid1 = -1, refid2 = -1;
    unsigned long cnt;
    double dptrans = 0, dprot = 0;
    double p = 1;
    double apad = 0, a = 0, b = 0, c = 0;
    double _sqd; // squared displacement
    Average<double> msqd, countNin_avg, countNin_avgBlocks, countNout_avg, countNout_avgBlocks; // mean squared displacement and particle counters 

    double cosTheta, theta;
    double x, y;
    double coord, coordNew, coordTemp;
    double randNbr;
    double _bias = 0, rsd = 0.01, Nin, countNin, countNout, Ntot = 0, cntInner = 0;

    Point dir = {1,1,1};
    Point cylAxis = {0,0,0};
    Point origo = {0,0,0};
    Point molV = {0,0,0};

    bool findBias = true;
    
    void _to_json(json &j) const override;
    void _from_json(const json &j) override; //!< Configure via json object
    void _move(Change &change) override;
    double bias(Change&, double, double) override;
    void _accept(Change &) override { msqd += _sqd; }
    void _reject(Change &) override { msqd += 0; }

  public:
    BiasedTranslateRotate(Space &spc);
};


#ifdef DOCTEST_LIBRARY_INCLUDED
TEST_CASE("[Faunus] TranslateRotate") {
    typedef typename Space::Tpvec Tpvec;

    CHECK(!atoms.empty());     // set in a previous test
    CHECK(!molecules.empty()); // set in a previous test

    Space spc;
    TranslateRotate mv(spc);
    json j = R"( {"molecule":"B", "dp":1.0, "dprot":0.5, "dir":[0,1,0], "repeat":2 })"_json;
    mv.from_json(j);

    j = json(mv).at(mv.name);
    CHECK(j.at("molecule") == "B");
    CHECK(j.at("dir") == Point(0, 1, 0));
    CHECK(j.at("dp") == 1.0);
    CHECK(j.at("repeat") == 2);
    CHECK(j.at("dprot") == 0.5);
}
#endif

/**
 * @brief Move that will swap conformation of a molecule
 *
 * This will swap between different molecular conformations
 * as defined in `MoleculeData` with `traj` and `weight`.
 * If defined, the weight
 * distribution is respected, otherwise all conformations
 * have equal intrinsic weight. Upon insertion, the new conformation
 * is randomly oriented and placed on top of the mass-center of
 * an exising molecule. That is, there is no mass center movement.
 *
 * @todo Add feature to align molecule on top of an exiting one
 * @todo Expand `_info()` to show number of conformations
 * @warning Weighted distributions untested and not verified for correctness
 * @date Malmo, November 2016
 */
class ConformationSwap : public Movebase {
  private:
    typedef typename Space::Tpvec Tpvec;
    typedef MoleculeData Tmoldata;
    RandomInserter inserter;
    Space &spc; // Space to operate on
    int molid = -1;
    int newconfid = -1;

    void _to_json(json &j) const override;
    void _from_json(const json &j) override; //!< Configure via json object
    void _move(Change &change) override;
    void _accept(Change &change) override;

  public:
    ConformationSwap(Space &spc);

}; // end of conformation swap move

/**
 * @brief Sketch for MD move
 */
class ForceMove : public Movebase {
  private:
    typedef typename Space::Tpvec Tpvec;
    void _to_json(json &) const override{};
    void _from_json(const json &) override{};
    std::vector<Point> forces, velocities;

  public:
    ForceMove();
}; // end of forcemove

class VolumeMove : public Movebase {
  private:
    const std::map<std::string, Geometry::VolumeMethod> methods = {
        {"xy", Geometry::XY}, {"isotropic", Geometry::ISOTROPIC}, {"isochoric", Geometry::ISOCHORIC}};
    typename decltype(methods)::const_iterator method;
    typedef typename Space::Tpvec Tpvec;
    Space &spc;
    Average<double> msqd, Vavg; // mean squared displacement
    double dV = 0, deltaV = 0, Vnew = 0, Vold = 0;

    void _to_json(json &j) const override;
    void _from_json(const json &j) override;
    void _move(Change &change) override;
    void _accept(Change &) override;
    void _reject(Change &) override;

  public:
    VolumeMove(Space &spc);
}; // end of VolumeMove

/**
 * @brief Displaces charge on a single atom
 */
class ChargeMove : public Movebase {
  private:
    typedef typename Space::Tpvec Tpvec;
    Space &spc;           // Space to operate on
    Average<double> msqd; // mean squared displacement
    double dq = 0, deltaq = 0;
    int atomIndex;
    Change::data cdata;

    void _to_json(json &j) const override;
    void _from_json(const json &j) override;
    void _move(Change &change) override;
    void _accept(Change &) override;
    void _reject(Change &) override;

  public:
    ChargeMove(Tspace &spc);
};

/**
 * @brief Transfers charge between two atoms
 */
class ChargeTransfer : public Movebase {
  private:
    typedef typename Space::Tpvec Tpvec;
    Space &spc;           // Space to operate on
    Average<double> msqd; // mean squared displacement
    double dq = 0, deltaq = 0;
    double sumCharges = 0;
    double sumChanges = 0;
    int atomIndex = 0;
    int  molid = -1;
    int numOfAtoms = 0;
    int i = 0;
    std::vector<double> range, totrange, min, max, changeQ;
    Change::data cdata1, cdata2;

    void _to_json(json &j) const override;
    void _from_json(const json &j) override;
    void _move(Change &change) override;
    void _accept(Change &) override;
    void _reject(Change &) override;

  public:
    ChargeTransfer(Tspace &spc);
};


/**
 * @brief QuadrantJump translates a molecule to another quadrant
 * considering as the origin the center of the box or the center of mass
 * of a range of atomic indexes specified by "index": [start:stop].
 */
class QuadrantJump : public Movebase {
  private:
    typedef typename Space::Tpvec Tpvec;
    Space &spc; // Space to operate on
    int molid = -1;
    Point dir = {1, 1, 1};
    std::vector<size_t> index;
    double _sqd;          // squared displacement
    Average<double> msqd; // mean squared displacement

    void _to_json(json &j) const override;
    void _from_json(const json &j) override; //!< Configure via json object
    void _move(Change &change) override;
    void _accept(Change &) override { msqd += _sqd; }
    void _reject(Change &) override { msqd += 0; }

  public:
    QuadrantJump(Space &spc);
};

#ifdef ENABLE_MPI
/**
 * @brief Class for parallel tempering (aka replica exchange) using MPI
 *
 * Although not completely correct, the recommended way of performing a temper move
 * is to do `N` Monte Carlo passes with regular moves and then do a tempering move.
 * This is because the MPI nodes must be in sync and if you have a system where
 * the random number generator calls are influenced by the Hamiltonian we could
 * end up in a deadlock.
 *
 * @date Lund 2012, 2018
 */
class ParallelTempering : public Movebase {
  private:
    typedef typename Tspace::Tpvec Tpvec;

    Tspace &spc; // Space to operate on
    MPI::MPIController &mpi;

    int partner;                   //!< Exchange replica (partner)
    enum extradata { VOLUME = 0 }; //!< Structure of extra data to send
    std::map<std::string, Average<double>> accmap;

    MPI::FloatTransmitter ft;           //!< Class for transmitting floats over MPI
    MPI::ParticleTransmitter<Tpvec> pt; //!< Class for transmitting particles over MPI

    void findPartner(); //!< Find replica to exchange with
    bool goodPartner(); //!< Is partner valid?
    void _to_json(json &j) const override;
    void _move(Change &change) override;
    double exchangeEnergy(double mydu); //!< Exchange energy with partner
    double bias(Change &, double uold, double unew) override;
    std::string id(); //!< Unique string to identify set of partners
    void _accept(Change &) override;
    void _reject(Change &) override;
    void _from_json(const json &j) override;

  public:
    ParallelTempering(Tspace &spc, MPI::MPIController &mpi);
};
#endif

class Propagator : public BasePointerVector<Movebase> {
  private:
    int _repeat;
    std::discrete_distribution<> dist;
    std::vector<double> w; // list of weights for each move
    void addWeight(double weight = 1);

  public:
    using BasePointerVector<Movebase>::vec;
    Propagator() = default;
    Propagator(const json &j, Tspace &spc, MPI::MPIController &mpi);
    int repeat() { return _repeat; }
    auto sample() {
        if (!vec.empty()) {
            assert(w.size() == vec.size());
            return vec.begin() + dist(Move::Movebase::slump.engine);
        }
        return vec.end();
    } //!< Pick move from a weighted, random distribution
};

} // namespace Move

class MCSimulation {
  private:
    typedef typename Space::Tpvec Tpvec;

    std::string lastMoveName; //!< name of latest move

    bool metropolis(double du) const; //!< Metropolis criterion (true=accept)

    struct State {
        Space spc;
        Energy::Hamiltonian pot;
        State(const json &j);

        void sync(State &other, Change &change);
    }; //!< Contains everything to describe a state

    State state1, // old state (accepted)
        state2;   // new state (trial)
    double uinit = 0, dusum = 0;
    Average<double> uavg;

    void init();

  public:
    Move::Propagator moves;

    auto &pot() { return state1.pot; }
    auto &space() { return state1.spc; }
    const auto &pot() const { return state1.pot; }
    const auto &space() const { return state1.spc; }
    const auto &geometry() const { return state1.spc.geo; }
    const auto &particles() const { return state1.spc.p; }

    MCSimulation(const json &j, MPI::MPIController &mpi);
    double drift(); //!< Calculates the relative energy drift from initial configuration

    /* currently unused -- see Analysis::SaveState.
                    void store(json &j) const {
                        j = state1.spc;
                        j["random-move"] = Move::Movebase::slump;
                        j["random-global"] = Faunus::random;
                    } // store system to json object
    */
    void restore(const json &j); //!< restore system from previously store json object
    void move();
    void to_json(json &j);
};

void to_json(json &j, MCSimulation &mc);

/**
 * @brief Ideal energy contribution of a speciation move
 * This funciton calculates the contribution to the energy change arising from the
 * change in concentration of reactant and products in the current and in the trial state.
 *
 * @f[
 *     \beta \Delta U = - \sum \ln ( N_o!/N_n! V^{N_n - N_o} )
 * @f]
 *
 * where the sum runs over all products and reactants.
 *
 * @todo
 * - use exception message to suggest how to fix the problem
 * - seems out of place; move to another file?
 * - find a better name
 */
double IdealTerm(Space &spc_n, Space &spc_o, const Change &change);

} // namespace Faunus
