#include "particle.h"

namespace Faunus {
void ParticlePropertyBase::rotate(const Eigen::Quaterniond &, const Eigen::Matrix3d &) {}

void Radius::to_json(json &j) const { j["r"] = radius; }

void Radius::from_json(const json &j) { radius = j.value("r", 0.0); }

void Charge::to_json(json &j) const { j["q"] = charge; }

void Charge::from_json(const json &j) { charge = j.value("q", 0.0); }

void Dipole::rotate(const Eigen::Quaterniond &q, const Eigen::Matrix3d &) { mu = q * mu; }

void Dipole::to_json(json &j) const {
    j["mu"] = mu;
    j["mulen"] = mulen;
}

void Dipole::from_json(const json &j) {
    mu = j.value("mu", Point(1, 0, 0));
    mulen = j.value("mulen", mulen);
}

void Polarizable::rotate(const Eigen::Quaterniond &q, const Eigen::Matrix3d &m) {
    mui = q * mui;
    alpha.rotate(m);
}

void Polarizable::to_json(json &j) const {
    j["alpha"] = alpha;
    j["mui"] = mui;
    j["muilen"] = muilen;
}

void Polarizable::from_json(const json &j) {
    alpha = j.value("alpha", alpha);
    mui = j.value("mui", Point(1, 0, 0));
    muilen = j.value("mulen", muilen);
}

void Quadrupole::rotate(const Eigen::Quaterniond &, const Eigen::Matrix3d &m) { Q.rotate(m); }

void Quadrupole::to_json(json &j) const { j["Q"] = Q; }

void Quadrupole::from_json(const json &j) { Q = j.value("Q", Q); }

void Cigar::rotate(const Eigen::Quaterniond &q, const Eigen::Matrix3d &) { scdir = q * scdir; }

void Cigar::to_json(json &j) const {
    j["scdir"] = scdir;
    j["sclen"] = sclen;
}
void Cigar::from_json(const json &j) {
    scdir = j.value("scdir", scdir);
    sclen = j.value("sclen", sclen);
}

const AtomData &PositionAndID::traits() {
    assert(id >= 0 and id < atoms.size());
    return atoms.at(id);
}
void PositionAndID::to_json(json &j) const { j = {{"id", id}, {"pos", pos}}; }
void PositionAndID::from_json(const json &j) {
    id = j.value("id", id);
    pos = j.value("pos", pos);
}
const AtomData &Particle::traits() {
    assert(id >= 0 and id < atoms.size());
    return atoms.at(id);
}
Particle::Particle(const AtomData &a) { *this = json(a).front(); }
void Particle::rotate(const Eigen::Quaterniond &q, const Eigen::Matrix3d &m) {
    if (shape != nullptr)
        shape->rotate(q, m);
}
void from_json(const json &j, Particle &p) {
    p.id = j.value("id", -1);
    p.pos = j.value("pos", Point(0, 0, 0));
    p.charge = j.value("q", 0.0);
    if (p.shape != nullptr)
        from_json(j, *p.shape);
}
void to_json(json &j, const Particle &p) {
    if (p.shape != nullptr)
        to_json(j, *p.shape);
    j["id"] = p.id;
    j["pos"] = p.pos;
    j["q"] = p.charge;
}
} // namespace Faunus
