#ifndef POTENTIALNEW_H
#define POTENTIALNEW_H

#include <QtCore>

namespace Langmuir
{
  class World;
  class Potential
  {
  public:
      Potential( World *world );
      void setPotentialZero();
      void setPotentialLinear();
      void setPotentialTraps();
      void setPotentialFromFile( QString filename );
      void setPotentialFromScript( QString filename );

      /**
       * @brief Precalculate interaction energies.
       *
       * precalculate the interaction energies of charges at different sites.
       * These values reduce necessary calculations in tight loops when considering Coulomb interactions.
       */
      void updateInteractionEnergies();

  private:
      World *m_world;
  };
}
#endif // POTENTIALNEW_H
