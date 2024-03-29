#pragma once

#include "move.h"

namespace Faunus {
namespace Move {

/*
 * @brief Establishes equilibrium of matter
 * Establishes equilibrium of matter between all species
 *
 * Consider the dissociation process AX=A+X. This class will locate
 * all species of type AX and A and make a MC swap move between them.
 * X can be implicit, meaning that it enters only with its chemical potential
 * (activity). The reacting species, the equilibrium constant,
 * and the activities are read from the JSON input file.
 *
 */
class SpeciationMove : public Movebase {
  private:
    typedef typename Tspace::Tpvec Tpvec;

    Tspace &spc;
    Tspace *otherspc;
    ReactionData *trialprocess;
    std::map<std::string, Average<double>> accmap;

    double lnK;
    double bondenergy;
    bool forward;
    std::vector<int> molDel;  // index of groups to delete
    std::vector<int> atomDel; // atom index to delete
    std::map<int, int> molcnt_ins, atomcnt_ins, molcnt_del, atomcnt_del, molcnt,
        atomcnt;                    // id's and number of inserted/deleted mols and atoms
    std::multimap<int, Tpvec> pmap; // coordinates of mols and atoms to be inserted
    // unsigned int Ndeleted, Ninserted; // Number of accepted deletions and insertions

    void _to_json(json &j) const override;

    void _from_json(const json &) override{};

  public:
    SpeciationMove(Tspace &spc);

    void setOther(Tspace &ospc);

    void _move(Change &change) override;

    double bias(Change &, double, double) override; //!< adds extra energy change not captured by the Hamiltonian

    void _accept(Change &) override;

    void _reject(Change &) override;

}; // End of class SpeciationMove

} // namespace Move
} // namespace Faunus