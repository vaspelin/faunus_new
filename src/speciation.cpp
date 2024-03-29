#include "speciation.h"

namespace Faunus {
namespace Move {

void SpeciationMove::_to_json(json &j) const {
    j = {
        // { "replicas", mpi.nproc() },
        // { "datasize", pt.getFormat() }
    };
    json &_j = j["reactions"];
    _j = json::object();
    for (auto &m : accmap)
        _j[m.first] = {{"attempts", m.second.cnt}, {"acceptance", m.second.avg()}};
    Faunus::_roundjson(_j, 3);
}
void SpeciationMove::setOther(Tspace &ospc) { otherspc = &ospc; }
void SpeciationMove::_move(Change &change) {
    if (reactions.size() > 0) {
        auto rit = slump.sample(reactions.begin(), reactions.end());
        lnK = rit->lnK;
        forward = (bool)slump.range(0, 1); // random boolean
        trialprocess = &(*rit);
        if (rit->empty(forward)) // Enforce canonic constraint if invoked
            return;              // Out of material, slip out the back door

        for (auto &m : rit->Molecules2Add(not forward)) { // Delete checks
            auto mollist = spc.findMolecules(m.first, Tspace::ALL);
            if (molecules[m.first].atomic) {
                if (size(mollist) != 1) // There can be only one
                    throw std::runtime_error("Bad definition: One group per atomic molecule!");
                auto git = mollist.begin();
                if (git->size() < m.second) // Assure that there are atoms enough in the group
                    return;                 // Slip out the back door
            } else {
                mollist = spc.findMolecules(m.first, Tspace::ACTIVE);
                if (size(mollist) < m.second)
                    return; // Not possible to perform change, escape through the back door
            }
        }
        for (auto &m : rit->Molecules2Add(forward)) { // Addition checks
            auto mollist = spc.findMolecules(m.first, Tspace::ALL);
            if (molecules[m.first].atomic) {
                if (size(mollist) != 1) // There can be only one
                    throw std::runtime_error("Bad definition: One group per atomic molecule!");
                auto git = mollist.begin();
                if ((git->size() + m.second) > git->capacity()) { // Assure that there are atoms enough in the group
                    return;                                       // Slip out the back door
                }
            } else {
                mollist = spc.findMolecules(m.first, Tspace::INACTIVE);
                if (size(mollist) < m.second) {
                    return; // Not possible to perform change, escape through the back door
                }
            }
        }

        if (rit->swap == true) {
            auto m1 = rit->Atoms2Add(not forward); // Swap checks
            auto m2 = rit->Atoms2Add(forward);
            assert((m1.size() == 1) and (m2.size() == 1) &&
                   "Bad definition: Only 2 explicit atoms per reaction!"); // Swap A = B
            auto atomlist = spc.findAtoms(m1.begin()->first);
            if (size(atomlist) < 1) // Make sure that there are any active atoms to swap
                return;             // Slip out the back door
            auto ait = slump.sample(atomlist.begin(), atomlist.end()); // Random particle iterator
            auto git = spc.findGroupContaining(*ait);
            Change::data d;
            d.atoms.push_back(Faunus::distance(git->begin(), ait)); // Index of particle rel. to group
            d.index = Faunus::distance(spc.groups.begin(), git);
            d.internal = true;
            d.dNswap = true;
            change.groups.push_back(d); // Add to list of moved groups
            Particle p = atoms.at(m2.begin()->first);
            p.pos = ait->pos;
            *ait = p;
            assert(ait->id == m2.begin()->first);
        }

        bondenergy = 0;

        change.dN = true;                                 // Attempting to change the number of atoms / molecules
        for (auto &m : rit->Molecules2Add(not forward)) { // Delete
            auto mollist = spc.findMolecules(m.first, Tspace::ALL);
            if (molecules[m.first].atomic) {
                auto git = mollist.begin();
                auto othermollist =
                    otherspc->findMolecules(m.first, Tspace::ALL); // implies that new and old are in sync
                auto othergit = othermollist.begin();
                Change::data d;
                d.index = Faunus::distance(spc.groups.begin(), git); // integer *index* of moved group
                d.internal = true;
                d.dNatomic = true;
                for (int N = 0; N < m.second; N++) {                   // deactivate m.second m.first atoms
                    auto ait = slump.sample(git->begin(), git->end()); // iterator to random atom
                    // Shuffle back to end, both in trial and new
                    auto nait = git->end() - 1;                   // iterator to last atom
                    int dist = Faunus::distance(ait, git->end()); // distance to random atom from end
                    if (Faunus::distance(ait, nait) > 1) {
                        std::iter_swap(ait, nait);
                        std::iter_swap(othergit->end() - dist - N, othergit->end() - (1 + N));
                    }
                    d.atoms.push_back(Faunus::distance(git->begin(), nait));
                    git->deactivate(nait, git->end());
                }
                std::sort(d.atoms.begin(), d.atoms.end());
                change.groups.push_back(d); // add to list of moved groups
            } else {
                mollist = spc.findMolecules(m.first, Tspace::ACTIVE);
                for (int N = 0; N < m.second; N++) {
                    auto git = slump.sample(mollist.begin(), mollist.end());
                    for (auto &bond : molecules.at(m.first).bonds) {
                        auto bondclone = bond->clone();
                        bondclone->shift(std::distance(spc.p.begin(), git->begin()));
                        Potential::setBondEnergyFunction(bondclone, spc.p);
                        bondenergy += bondclone->energy(spc.geo.getDistanceFunc());
                    }
                    git->deactivate(git->begin(), git->end());
                    Change::data d;
                    d.index = Faunus::distance(spc.groups.begin(), git); // integer *index* of moved group
                    d.all = true;                                        // *all* atoms in group were moved
                    d.internal = true;
                    for (int i = 0; i < git->capacity(); i++)
                        d.atoms.push_back(i);
                    change.groups.push_back(d); // add to list of moved groups
                }
            }
        }

        for (auto &m : rit->Molecules2Add(forward)) { // Add
            auto mollist = spc.findMolecules(m.first, Tspace::ALL);
            if (molecules[m.first].atomic) {
                auto git = mollist.begin();
                Change::data d;
                d.index = Faunus::distance(spc.groups.begin(), git);
                d.internal = true;
                d.dNatomic = true;
                for (int N = 0; N < m.second; N++) { // Activate m.second m.first atoms
                    git->activate(git->end(), git->end() + 1);
                    auto ait = git->end() - 1;
                    spc.geo.randompos(ait->pos, slump);
                    spc.geo.getBoundaryFunc()(ait->pos);
                    d.atoms.push_back(Faunus::distance(git->begin(), ait)); // Index of particle rel. to group
                }
                std::sort(d.atoms.begin(), d.atoms.end());
                change.groups.push_back(d); // Add to list of moved groups
            } else {
                mollist = spc.findMolecules(m.first, Tspace::INACTIVE);
                for (int N = 0; N < m.second; N++) {
                    auto git = slump.sample(mollist.begin(), mollist.end());
                    git->activate(git->inactive().begin(), git->inactive().end());
                    Point cm = git->cm;
                    git->translate(-cm, spc.geo.getBoundaryFunc());
                    spc.geo.randompos(cm, slump);
                    git->translate(cm, spc.geo.getBoundaryFunc());
                    Point u = ranunit(slump);
                    Eigen::Quaterniond Q(Eigen::AngleAxisd(2 * pc::pi * (slump() - 0.5), u));
                    git->rotate(Q, spc.geo.getBoundaryFunc());
                    for (auto &bond : molecules.at(m.first).bonds) {
                        auto bondclone = bond->clone();
                        bondclone->shift(std::distance(spc.p.begin(), git->begin()));
                        Potential::setBondEnergyFunction(bondclone, spc.p);
                        bondenergy -= bondclone->energy(spc.geo.getDistanceFunc());
                    }
                    Change::data d;
                    d.index = Faunus::distance(spc.groups.begin(), git); // Integer *index* of moved group
                    d.all = true;                                        // All atoms in group were moved
                    d.internal = true;
                    for (int i = 0; i < git->capacity(); i++)
                        d.atoms.push_back(i);
                    change.groups.push_back(d); // Add to list of moved groups
                    assert(spc.geo.sqdist(git->cm, Geometry::massCenter(git->begin(), git->end(),
                                                                        spc.geo.getBoundaryFunc(), -git->cm)) < 1e-9);
                }
            }
        }

        std::sort(change.groups.begin(), change.groups.end());
    } else
        throw std::runtime_error("No reactions in list, disable rcmc or add reactions");
}
double SpeciationMove::bias(Change &, double, double) {
    if (forward)
        return -lnK + bondenergy;
    return lnK + bondenergy;
}
void SpeciationMove::_accept(Change &) {
    accmap[trialprocess->name] += 1;
    trialprocess->N_reservoir += (forward == true) ? -1 : 1;
    if (trialprocess->N_reservoir < 0 && trialprocess->canonic)
        throw std::runtime_error("There are no negative number of molecules");
}
void SpeciationMove::_reject(Change &) { accmap[trialprocess->name] += 0; }

SpeciationMove::SpeciationMove(Tspace &spc) : spc(spc) {
    name = "rcmc";
    cite = "doi:10/fqcpg3";
}

} // end of namespace Move
} // end of namespace Faunus