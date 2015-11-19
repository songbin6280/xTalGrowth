/*
  Copyright (C) 2012,2013
      Max Planck Institute for Polymer Research
  Copyright (C) 2008,2009,2010,2011
      Max-Planck-Institute for Polymer Research & Fraunhofer SCAI
  
  This file is part of ESPResSo++.
  
  ESPResSo++ is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  ESPResSo++ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

// ESPP_CLASS
#ifndef _INTERACTION_LJCOS_HPP
#define _INTERACTION_LJCOS_HPP

#include "Potential.hpp"

#ifndef M_PIl
#define M_PIl 3.1415926535897932384626433832795029L
#endif

namespace espressopp {
  namespace interaction {
    /** This class provides methods to compute forces and energies of
		 the modified Lennard Jones potential (WCA) combined with the cos-contribution that 
		 smoothly reaches zero. It has cutoff 1.5 LJ units. There are two main application 
		 possibilities:
		 1. Controlling the depth of the resulted potential mimic solvents of various qualities;
		 2. With some modification cos->cos^2 model biomembranes.
		 
		 The approach 1. can be followes in detail in M. Steinhauser, JCP, 2005 (Eq. 8)
		 The approach 2. is the paper of Markus Deserno in Nature from 2007

    */
    class LJcos : public PotentialTemplate< LJcos > {
    private:
      real phi;
      
      real pot_border, sqr_pot_border;		// border of the WCA (repulsive LJ) potential
			real epsilon = 1.;
			real alpha, beta, gamma;
      real one_phi, half_phi, alpha_phi, gamma_phi;
      real sqr_cutoff;

      real auxCoef; // This is temporary solution for empty potential. This problem
                    // appears when there is a potential for, for example,  00 and
                    // 01, but no potential for 11. The loop in interaction
                    // template will call potential(1, 1) anyway if these particles are
                    // within the range. Thus one have to set coefficient to 0,
                    // in order to get zero forces. 
    public:
      static void registerPython();

			// constructor without the cos-part
			LJcos(): phi(0.0){
        setShift(0.0);
        autoShift = false;
        setCutoff(1.5);
//				setEpsilon(1.0);
        preset();
        auxCoef = 0.0;
      }

			// constructor with arbitrary phi
      LJcos(real _phi): phi(_phi){	
        setShift(0.0);
        autoShift = false;
        setCutoff(1.5);
//				setEpsilon(1.0);
        preset();
        auxCoef = 1.0;
      }

      virtual ~LJcos() {};

//			void setEpsilon(real _epsilon) {
//				epsilon = _epsilon;
//			}
			
//			real getEpsilon() const { return epsilon; }
			
      void preset() {
        pot_border = pow(2.0, 1.0/6.0);
        sqr_pot_border = pot_border * pot_border;
				sqr_cutoff = getCutoff() * getCutoff();
        alpha = M_PIl / (sqr_cutoff - sqr_pot_border);
        beta = M_PIl - sqr_pot_border*alpha;
				gamma = -0.5;

				real _epsilon = 1.;
        one_phi = (1.0 - phi) * _epsilon;
        half_phi = 0.5 * phi * _epsilon;
				gamma_phi = gamma * phi * _epsilon;
        alpha_phi = alpha * phi * _epsilon;
      }
      
      // Setter and getter phi
      void setPhi(real _phi) {
        phi = _phi;
        preset();
      }
      real getPhi() const { return phi; }

      real _computeEnergySqrRaw(real distSqr) const {
        real energy;
        if(distSqr<=sqr_pot_border){
          real frac2 = auxCoef / distSqr;
          real frac6 = frac2 * frac2 * frac2;
          energy = 4.0 * (frac6 * frac6 - frac6) + one_phi;
        }
        else{
          energy = half_phi * cos(alpha*distSqr+beta) + gamma_phi;
        }
        
        return energy;
      }

      bool _computeForceRaw(Real3D& force,
                            const Real3D& r21,
                            real distSqr) const {
        real ffactor;
        if(distSqr<=sqr_pot_border){
          real frac2 = auxCoef / distSqr;
          real frac6 = frac2 * frac2 * frac2;
          ffactor = frac6 * ( 48.0 * frac6 - 24.0 ) * frac2;
        }
        else{
          ffactor = alpha_phi * sin( alpha * distSqr + beta );
        }
        force = r21 * ffactor;
        return true;
      }
    };
    // provide pickle support
    struct LJcos_pickle : boost::python::pickle_suite
    {
      static
      boost::python::tuple
      getinitargs(LJcos const& pot)
      {
          real p;
          p=pot.getPhi();
          return boost::python::make_tuple(p);
      }
    };

  }
}

#endif
