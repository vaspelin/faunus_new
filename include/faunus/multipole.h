#ifndef FAUNUS_MULTIPOLE_H
#define FAUNUS_MULTIPOLE_H

namespace Faunus {

  /**
   * @brief Approximation of erfc-function
   * @param x Value for which erfc should be calculated 
   * @details Reference for this approximation is found in Abramowitz and Stegun, Handbook of mathematical functions, eq. 7.1.26
   *
   * @f[
   *     erf(x) = 1 - (a_1t + a_2t^2 + a_3t^3 + a_4t^4 + a_5t^5)e^{-x^2} + \epsilon(x)
   * @f]
   * @f[
   *     t = \frac{1}{1 + px}
   * @f]
   * @f[
   *     |\epsilon(x)| \le 1.5\times 10^{-7}
   * @f]
   */
  double erfc_x(double x) {
    //
    // |error| <= 1.5*10^-7
    double p = 0.3275911;
    double t = 1.0/(1.0+p*x);
    double x2 = x*x;
    double a1 = 0.254829592;
    double a2 = -0.284496736;
    double a3 = 1.421413741;
    double a4 = -1.453152027;
    double a5 = 1.061405429;
    double tp = t*(a1+t*(a2+t*(a3+t*(a4+t*a5))));
    return tp*exp(-x2);
  }

  /**
   * @brief Returns NemoType1-interaction (Exponential Repulsion)                         Needs to be checked!
   * @param vec Vector with parameters. Form: \f$ [a_{ab} b_{ab} c_{ab} d_{ab}] \f$  
   * @param r Vector between particles
   * @param expmax Maximum exponential coefficient (optional)
   */
  template<class Tvec>
    double nemo1(Eigen::VectorXd &vec, const Tvec &r, double expmax=80.0) {
      double asw = 1.2;
      double nsw = 4;
      double r1i = 1/r.norm();
      double r2i = r1i*r1i;
      double r6i = r2i*r2i*r2i;
      double ss = 1 - pow(2.71828,-std::min(expmax,pow((1/(asw*r1i)),nsw)));
      
      double uexp  = vec[0]*pow(2.71828,-std::min(expmax,vec[1]/r1i));
      double ur20  = vec[2]*r6i*r6i*r6i*r2i;
      double udis  = vec[3]*ss*r6i;
      return (uexp  + ur20 + udis);
    }
    
  /**
   * @brief Returns NemoType2-interaction (r-7 Repulsion)                         Needs to be checked!
   * @param vec Vector with parameters. Form: \f$ [a_{ab} b_{ab} c_{ab} d_{ab}] \f$  
   * @param r Vector between particles
   * @param expmax Maximum exponential coefficient (optional)
   */
  template<class Tvec>
    double nemo2(Eigen::VectorXd &vec, const Tvec &r, double expmax=80.0) {
      double asw = 1.2;
      double nsw = 4;
      double r1i = 1/r.norm();
      double r2i = r1i*r1i;
      double r6i = r2i*r2i*r2i;
      double ss = 1 - pow(2.71828,-std::min(expmax,pow((1/(asw*r1i)),nsw)));
      
      double uexp  = vec[0]*r1i*r6i;
      double udis  = vec[3]*ss*r6i;
      return (uexp + udis);
    }
    
  /**
   * @brief Returns NemoType3-interaction (Modified Interactions)                         Needs to be checked!
   * @param vec Vector with parameters. Form: \f$ [a_{ab} b_{ab} c_{ab} d_{ab} n_{ab}] \f$  
   * @param r Vector between particles
   */
  template<class Tvec>
    double nemo3(Eigen::VectorXd &vec, const Tvec &r) {
      double r1i = 1/r.norm();
      double r2i = r1i*r1i;
      double r6i = r2i*r2i*r2i;
      
      double uexp  = vec[3]*pow(r1i,vec[4]);          // vec[4] = nab   <------------------------
      double udis1  = -vec[2]*r6i;
      double udis2  = vec[0]*pow(2.71828,-vec[1]/r1i);
      return (uexp  + udis1 + udis2);
    }
  
  /**
   * @brief Returns NemoType4-interaction (Damping Exponential)
   * @param vec Vector with parameters. Form: \f$ [a_{ab} b_{ab} c_{ab} d_{ab} e_{ab} f_{ab} n_{ab}] \f$  
   * @param r Vector between particles
   * @param expmax Maximum exponential coefficient (optional)
   */
  template<class Tvec>
    double nemo4(Eigen::VectorXd &vec, const Tvec &r, double expmax=80.0) {
      double r1i = 1/r.norm();
      double r2i = r1i*r1i;
      double r6i = r2i*r2i*r2i;
      
      double uexp1  = vec[4]*pow(2.71828,-std::min(expmax,vec[5]/r1i));
      double uexp2  = 0;
      if(vec[6] != 0) uexp2 = vec[3]*pow(r1i,vec[6]);               // vec[6] = nab   <------------------------
      double udis1  =-vec[2]*r6i;
      double udis2  = vec[0]*pow(2.71828,-std::min(expmax,vec[1]/r1i));
      return (uexp1 + uexp2  + udis1 + udis2);
    }
    
  /**
   * @brief Returns NemoType5-interaction (Full Damping)                         Needs to be checked!
   * @param vec Vector with parameters. Form: \f$ [a_{ab} b_{ab} c_{ab} d_{ab} e_{ab} f_{ab} n_{ab}] \f$  
   * @param r Vector between particles
   * @param expmax Maximum exponential coefficient (optional)
   */
  template<class Tvec>
    double nemo5(Eigen::VectorXd &vec, const Tvec &r, double expmax=80.0) {
      double r1i = 1/r.norm();
      double r2i = r1i*r1i;
      double r6i = r2i*r2i*r2i;
      double bri = r1i/vec[1];
      double ud1 = 6*bri;
      double ud2 = 5*bri*ud1;
      double ud3 = 4*bri*ud2;
      double ud4 = 3*bri*ud3;
      double ud5 = 2*bri*ud4;
      double ud6 = bri*ud5;
      
      double uexp1  = vec[4]*pow(2.71828,-std::min(expmax,vec[5]/r1i));
      double uexp2  = 0;
      if(vec[6] != 0) uexp2 = vec[3]*pow(r1i,vec[6]);       // vec[6] = nab   <------------------------
      double udis1  =-vec[2]*r6i;
      double udd = 1 + ud1 + ud2 + ud3 + ud4 + ud5 + ud6;
      double udis2  = vec[0]*pow(2.71828,-std::min(expmax,1/bri));
      return (uexp1 + uexp2  + udis1 + udd*udis2);
    }
    
  /**
   * @brief Returns NemoType6-interaction (Full Damping chtr)                         Needs to be checked!
   * @param vec Vector with parameters. Form: \f$ [a_{ab} b_{ab} c_{ab} d_{ab} e_{ab} f_{ab} n_{ab}] \f$  
   * @param r Vector between particles
   * @param expmax Maximum exponential coefficient (optional)
   */
  template<class Tvec>
    double nemo6(Eigen::VectorXd &vec, const Tvec &r, double expmax=80.0) {
      double r1i = 1/r.norm();
      double r2i = r1i*r1i;
      double r6i = r2i*r2i*r2i;
      double bri = r1i/vec[1];
      double ud1 = 6*bri;
      double ud2 = 5*bri*ud1;
      double ud3 = 4*bri*ud2;
      double ud4 = 3*bri*ud3;
      double ud5 = 2*bri*ud4;
      double ud6 = bri*ud5;

      double uexp1  = vec[4]*pow(2.71828,-std::min(expmax,vec[5]/r1i));
      double uexp2  = 0;
      if(vec[6] != 0) uexp2 = vec[3]*pow(r1i,vec[6]);                     // vec[6] = nab   <------------------------
      double udis1  =-vec[2]*r6i;
      double udd = 1 + ud1 + ud2 + ud3 + ud4 + ud5 + ud6;
      double udis2  = vec[0]*pow(2.71828,-std::min(expmax,1/bri));
      double uchtexp = -vec[8]*pow(2.71828,-std::min(expmax,vec[7]/r1i));  // vec[7] = acht, vec[8] = kcht    <------------------------
      return (uexp1 + uexp2  + udis1 + udd*udis2 + uchtexp);
    }
    
  /**
   * @brief Returns NemoType7-interaction (Full Damping chtr gaussian)                         Needs to be checked!
   * @param vec Vector with parameters. Form: \f$ [a_{ab} b_{ab} c_{ab} d_{ab} e_{ab} f_{ab} n_{ab}  a_{cht} k_{cht}] \f$  
   * @param r Vector between particles
   * @param expmax Maximum exponential coefficient (optional)
   */
  template<class Tvec>
    double nemo7(Eigen::VectorXd &vec, const Tvec &r, double expmax=80.0) {
      double r1i = 1/r.norm();
      double r2i = r1i*r1i;
      double r6i = r2i*r2i*r2i;
      double bri = r1i/vec[1];
      double ud1 = 6*bri;
      double ud2 = 5*bri*ud1;
      double ud3 = 4*bri*ud2;
      double ud4 = 3*bri*ud3;
      double ud5 = 2*bri*ud4;
      double ud6 = bri*ud5;
      
      double uchtexp = -vec[8]*pow(2.71828,-std::min(expmax,vec[7]*(pow((r.norm()-vec[3]),2))));  // vec[7] = acht, vec[8] = kcht    <------------------------
      double uexp  = vec[4]*pow(2.71828,-std::min(expmax,vec[5]/r1i));
      double udis1  =-vec[2]*r6i;
      double udd = 1 + ud1 + ud2 + ud3 + ud4 + ud5 + ud6;
      double udis2  = vec[0]*pow(2.71828,-std::min(expmax,1/bri));
      return (uexp + udis1 + udd*udis2 + uchtexp);
    }

  /**
   * @brief Returns ion-dipole interaction, Needs to be checked!
   * @param QxMu Product of ion charge and dipole scalar
   * @param mu Unit dipole moment vector
   * @param r Direction \f$ r_Mu - r_Q \f$  
   *
   */
  template<class Tvec>
    double q2mu(double QxMu, const Tvec &mu, const Tvec &r) {
      double R2 = 1/r.squaredNorm();
      double R1 = sqrt(R2);
      double R3 = R1*R2;
      double W = QxMu*mu.dot(r)*R3;
      return W;  // Beware of r_Mu - r_Q = -r according to Israelachvili p.36, i.e. minus becomes plus
    }

  /**
   * @brief Returns dipole-dipole interaction
   *
   * @param muA Unit dipole moment vector of particle A
   * @param muB Unit dipole moment vector of particle B
   * @param muAxmuB Product of dipole scalars
   * @param r Vector \f$ r_{AB} \f$
   */
  template<class Tvec>
    double mu2mu(const Tvec &muA, const Tvec &muB, double muAxmuB, const Tvec &r) {
      double R2 = 1/r.squaredNorm();
      double R1 = sqrt(R2);
      double R3 = R1*R2;
      double R5 = R3*R2;
      //Eigen::Matrix3d T = 3*R5*r*r.transpose() - R3*Matrix3d::Identity();
      //double W = -muA.transpose()*T*muB;                       // Buckingham    Å^-3
      double W = muA.dot(muB)*R3-3*muA.dot(r)*muB.dot(r)*R5; // J&K
      return W*muAxmuB;  // e^2 Å^2 Å ^-3 = e^2 /A
    }

  class WolfDipoleDipole {
    private:
      double rc1i_d, rc3i_d, rc4i_d, rc1i, rc2i, expKc, kappa, kappa2;
    public:
      WolfDipoleDipole(double alpha, double rcut) {
        kappa = alpha;
        kappa2 = kappa*kappa;
        rc1i = 1/rcut;
        rc2i = rc1i*rc1i;
        expKc = 2*kappa*exp(-kappa2/rc2i)/sqrt(pc::pi);
        rc1i_d = erfc_x(kappa/rc1i)*rc1i;
        rc3i_d = expKc*(kappa2 + rc2i) + rc1i_d*rc2i;
        rc4i_d = 2*expKc*((kappa2*kappa2/(3*rc1i)) + (kappa2*rc1i/3) + (rc1i*rc2i/2)) + rc1i_d*rc1i*rc2i;
      }

      template<class Tvec>
        double energy(const Tvec &muA, const Tvec &muB, double muAxmuB, const Tvec &r_ab, double r1i, double r2i) const {
          if(r2i < rc2i)
            return 0;
          Point r = r_ab*r1i;
          double r1i_d = erfc_x(kappa/r1i)*r1i;
          double expK = 2*kappa*exp(-kappa2/r2i)/sqrt(pc::pi);
          double r3i_d = expK*(kappa2 + r2i) + r1i_d*r2i;
          Eigen::Matrix3d T = 3*r*r.transpose() - Matrix3d::Identity();
          double W = muA.transpose()*T*muB;  
          return W*muAxmuB*(r3i_d-rc3i_d + 3*rc4i_d*(r1i_d - rc1i_d));
        }

      double getKappa() { return kappa; }
      double getCutoff() { return (1/rc1i); }
  };

  /**
   * @brief Returns ion-quadrupole interaction
   */
  template<class Tvec, class Tmat>
    double q2quad(double q, const Tmat &quad, const Tvec &r) {
      double R2 = 1/r.squaredNorm();
      double R1 = sqrt(R2);
      double R3 = R1*R2;
      double R5 = R3*R2;
      double W = r.transpose()*quad*r;
      W = W*R5  - quad.trace()*(R3/3); // e / Å
      return q*W; // e^2 / Å
    }

  namespace Potential {

    class NemoRepulsion : public PairPotentialBase {
      private:
        string _brief() { return "NemoRepulsion"; }
      protected:
        typedef Eigen::VectorXd Tvec;
        typedef opair<int> Tpair;
        std::map<Tpair,Tvec> pairMap;
        double expmax;
        double scaling;

      public:
        NemoRepulsion(InputMap &in) {
          name="Nemo repulsion";
          pc::setT ( in.get<double>("temperature", 298.15, "Absolute temperature (K)") );
          expmax = in.get<double>("expmax", 80, "Maximum repulsion exponent");
          scaling = 1000/(pc::Nav*pc::kT());  // Converts from kJ/mol to kT
          pairMap = json::atomPairMap("water2.json","pairproperties","nemorep");
        }

        /**
         * @brief NemoRepulsion
         * @param a Dipole particle A
         * @param b Dipole particle B
         * @param r Direction \f$ r_A - r_B \f$  
         */
        template<class Tparticle>
          double operator()(const Tparticle &a, const Tparticle &b, const Point &r) const {
            Tpair pair(a.id,b.id);
            Tvec vec;
            auto it = pairMap.find(pair);
            if (it!=pairMap.end()) { 
              vec = it->second;
              return nemo4(vec, r,expmax)*scaling;
            }
            assert(!"No pair data defined");
            return 0;
          }

        string info(char w) { return _brief(); }
    };

    /**
     * @brief Ion-dipole interaction, 
     *
     * More info...
     */
    class IonDipole : public PairPotentialBase {
      private:
        string _brief() { return "Ion-dipole"; }
      protected:
        double _lB;
      public:
        IonDipole(InputMap &in) {
          pc::setT ( in.get<double>("temperature", 298.15, "Absolute temperature (K)") );
          double epsilon_r = in.get<double>("epsilon_r",80., "Dielectric constant");
          _lB=pc::lB( epsilon_r );
        }
        /**
         * @brief Ion-dipole
         * @param a Dipole particle A
         * @param b Dipole particle B
         * @param r Direction \f$ r_A - r_B \f$  
         */
        template<class Tparticle> // q2mu(1->2,r) + q2mu(2->1,-r) = q2mu(1->2,r) - q2mu(2->1,r)
          double operator()(const Tparticle &a, const Tparticle &b, const Point &r) const {
            return _lB*(q2mu(a.charge*b.muscalar,b.mu,r) - q2mu(b.charge*a.muscalar,a.mu,r));
          }

        string info(char w) { return _brief(); }
    };

    /**
     * @brief Dipole-dipole interaction
     *
     * More info...
     */
    class DipoleDipole : public PairPotentialBase {
      private:
        string _brief() {
          std::ostringstream o;
          o << "Dipole-dipole, lB=" << _lB << textio::_angstrom;
          return o.str();          
        }
      protected:
        double _lB;
      public:
        DipoleDipole(InputMap &in) {
          name="Dipole-dipole";
          pc::setT ( in.get<double>("temperature", 298.15,
                "Absolute temperature (K)") );
          double epsilon_r = in.get<double>("epsilon_r",80.,
              "Dielectric constant");
          _lB = pc::lB(epsilon_r);
        }
        template<class Tparticle>
          double operator()(const Tparticle &a, const Tparticle &b, const Point &r) const {
            return _lB*mu2mu(a.mu, b.mu, a.muscalar*b.muscalar, r);
          }

        /** @brief Dipole field at `r` due to dipole `p` 
         *  Gets returned in [e/Å] (\f$\beta eE \f$)
         */
        template<class Tparticle>
          Point field(const Tparticle &p, const Point &r) const {
            double R2 = 1.0/r.squaredNorm();
            double R1 = sqrt(R2);
            Point r_n = r*R1;
            return ((3.0*p.mu.dot(r_n)*r_n - p.mu)*R2*R1)*p.muscalar*_lB; // \beta e E
          }

        /**
         * @brief Interaction of dipole `p` with field `E`
         * @todo unfinished
         */
        template<class Tparticle>
          double fieldEnergy(const Tparticle &p, const Point &E) {
            return 0; // implement!
          }

        string info(char w) {
          using namespace textio;
          std::ostringstream o;
          o << pad(SUB,w,"Temperature") << pc::T() << " K" << endl
            << pad(SUB,w,"Bjerrum length") << _lB << " "+angstrom << endl;
          return o.str();
        }
    };

    /**
     * @brief Dipole-dipole interaction w. spherical cutoff and reaction field
     *
     * More info...
     */
    class DipoleDipoleRF : public DipoleDipole {
      private:
        string _brief() { return "Dipole-dipole (RF)"; }
        double rc2,eps,eps_rf;
      public:
        DipoleDipoleRF(InputMap &in) : DipoleDipole(in) {
          name+=" Reaction Field";
          rc2 = pow(in.get<double>("dipdip_cutoff",pc::infty), 2);
          eps_rf = in.get<double>("epsilon_rf",80.);
          eps = _lB*(2*(eps_rf-1)/(2*eps_rf+1))/pow(rc2,1.5);
        }
        template<class Tparticle>
          double operator()(const Tparticle &a, const Tparticle &b, const Point &r) const {
            if (r.squaredNorm() < rc2)
              return (DipoleDipole::operator()(a,b,r) - eps*a.mu.dot(b.mu)*a.muscalar*b.muscalar);
            return 0;
          }

        void updateDiel(double er) {
          eps = _lB*(2*(er-1)/(er+1))/pow(rc2,1.5);
        }  

        string info(char w) {
          using namespace textio;
          std::ostringstream o;
          o << DipoleDipole::info(w)
            << pad(SUB,w,"Cutoff") << sqrt(rc2) << " "+angstrom << endl
            << pad(SUB,w,"epsilon_rf") << eps_rf << endl;
          return o.str();
        }
    };

    /**
     * @brief Ion-dipole interaction
     *
     * More info...
     */
    class IonQuad : public PairPotentialBase {
      private:
        string _brief() { return "Ion-quadrupole"; }
      protected:
        double _lB;
      public:
        IonQuad(InputMap &in) {
          pc::setT ( in.get<double>("temperature", 298.15, "Absolute temperature (K)") );
          double epsilon_r = in.get<double>("epsilon_r",80., "Dielectric constant");
          _lB=pc::lB( epsilon_r );
        }
        template<class Tparticle>
          double operator()(const Tparticle &a, const Tparticle &b, const Point &r) const {
            return _lB*(q2quad(a.charge, b.theta,r)+q2quad(b.charge, a.theta,r));
          }

        string info(char w) { return _brief(); }
    };

    class DipoleDipoleWolf : public DipoleDipole {
      private:
        string _brief() { return "Dipole-dipole Wolf"; }
      protected:
        double _lB, rc2i;
        WolfDipoleDipole wmu2mu;
      public:
        DipoleDipoleWolf(InputMap &in) : DipoleDipole(in),
        wmu2mu(in.get<double>("kappa",1.8, "Kappa-damping"), in.get<double>("dipdip_cutoff",in.get<double>("cuboid_len",pc::infty)/2)) {
          name+=" Wolf";
          rc2i = 1/in.get<double>("dipdip_cutoff",in.get<double>("cuboid_len",pc::infty)/2);
          rc2i = rc2i*rc2i;
        }

        template<class Tparticle>
          double operator()(const Tparticle &a, const Tparticle &b, const Point &r) const {
            double r2i = 1/r.squaredNorm();
            double r1i = std::sqrt(r2i);
            return _lB*wmu2mu.energy(a.mu,b.mu, a.muscalar*b.muscalar, r, r1i, r2i);
          }

        string info(char w) {
          using namespace textio;
          std::ostringstream o;
          o << DipoleDipole::info(w)
            << pad(SUB,w,"Cutoff") << wmu2mu.getCutoff() << " "+angstrom << endl
            << pad(SUB,w,"Kappa") << wmu2mu.getKappa() << " "+angstrom+"^-1" << endl;
          return o.str();
        }
    };
  }
}
#endif

