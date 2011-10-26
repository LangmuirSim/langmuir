#ifndef POTENTIALNEW_H
#define POTENTIALNEW_H

#include <QtCore>

namespace Langmuir
{
  class World;
  class Potential
  {
  public:

      /**
       * @brief Create Potential
       *
       * This class is a holder of methods used to set site energies
       *  @param world address of the world object
       */
      Potential( World *world );

      /**
       * @brief Zero site energies
       *
       * Sets all site energies in the grid to zero
       * @note new site energy = 0; \b Sets the site energy
       */
      void setPotentialZero();

      /**
       * @brief Add Linear term to site energies
       *
       * Creates a potential from the width of the grid and source - drain potential difference
       * Assumes potential doesn't vary with height or depth
       * @note new site energy = old site energy + new term; \b Adds to the site energy
       *  - grid.width
       *  - potential.source
       *  - potential.drain
       */
      void setPotentialLinear();

      /**
       * @brief Add Trap term to site energies
       *
       * Add a certain percentage of traps to the grid.
       * The traps are first seeded and then grown around the seeds.
       * Finally, some gaussian disorder is added to the traps.
       * @note new site energy = old site energy + new term; \b Adds to the site energy
       *  - trap.percentage
       *  - seed.percentage
       *  - gaussian.stdev
       *  - gaussian.averg
       */
      void setPotentialTraps();

      /**
       * @brief Add to site energies from a file
       *
       * Read a file which contains a list of numbers and set the site energy
       * Assumes the numbers are site, energy, site, energy, ...
       * @note new site energy = old site energy + new term; \b Adds to the site energy
       */
      void setPotentialFromFile( QString filename );

      /**
       * @brief Add to site energies from a script
       *
       * Currently this function is not implemented.
       * @note new site energy = old site energy + new term; \b Adds to the site energy
       */
      void setPotentialFromScript( QString filename );

      /**
       * @brief Precalculate interaction energies.
       *
       * precalculate the interaction energies of charges at different sites.
       * These values reduce necessary calculations in tight loops when considering Coulomb interactions.
       */
      void updateInteractionEnergies();

  private:

      /**
       * @brief address of the world object
       */
      World *m_world;
  };
}
#endif // POTENTIALNEW_H
