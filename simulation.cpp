#include "simulation.h"
#include "openclhelper.h"
#include "inputparser.h"
#include "chargeagent.h"
#include "sourceagent.h"
#include "drainagent.h"
#include "potential.h"
#include "cubicgrid.h"
#include "logger.h"
#include "world.h"
#include "rand.h"
namespace Langmuir
{
    Simulation::Simulation (SimulationParameters * par, int id) :  m_world(0), m_tick(0), m_id(id)
    {
        // Careful changing the order of these function calls; you can cause memory problems
        // For example, setPotentialTraps assumes the source and drain have already been created

        // Create the World Object
        m_world = new World(par);

        // Tell World about this simulation
        m_world->setSimulation(this);

        // Place Defects
        this->createDefects();

        // Create Source ( assumes defects were placed because source neighbors can't be defects )
        this->createSource();

        // Create Drain ( assumes defects were placed because drain neighbors can't be defects )
        this->createDrain();

        // Setup Grid Potential
        // Zero potential
        m_world->potential()->setPotentialZero();

        // Add Linear Potential
        if ( m_world->parameters()->potentialLinear ) { m_world->potential()->setPotentialLinear(); }

        // Add Trap Potential ( assumes source and drain were created so that hetero traps don't start growing on the source / drain )
        m_world->potential()->setPotentialTraps();

        // precalculate and store coulomb interaction energies
        m_world->potential()->updateInteractionEnergies();

        // place charges on the grid randomly ( assumes source / drain were created )
        if (m_world->parameters()->gridCharge) seedCharges();

        // Generate grid image
        if (m_world->parameters()->outputGrid)
        {
            m_world->logger()->saveTrapImageToFile(QString("trap-%1-%2-%3.png")
                   .arg(m_world->parameters()->trapPercentage*100.0)
                   .arg(m_world->parameters()->seedPercentage*100.0)
                   .arg(m_id));
        }

        // Output Field Energy
        if (m_world->parameters()->outputFieldPotential)
        {
            m_world->logger()->saveFieldEnergyToFile(QString("field-%1.dat").arg(m_id));
        }

        // Output Defect IDs
        if (m_world->parameters()->outputDefectIDs)
        {
            m_world->logger()->saveDefectIDsToFile(QString("defectIDs-%1.dat").arg(m_id));
        }

        // Output Trap IDs
        if (m_world->parameters()->outputTrapIDs)
        {
            m_world->logger()->saveTrapIDsToFile(QString("trapIDs-%1.dat").arg(m_id));
        }

        // Initialize OpenCL
        m_world->opencl()->initializeOpenCL();
        if ( m_world->parameters()->useOpenCL )
        {
            m_world->opencl()->toggleOpenCL(true);
        }
        else
        {
            m_world->opencl()->toggleOpenCL(false);
        }
    }

    Simulation::~Simulation ()
    {
        if ( m_world != 0 )
        {
            delete m_world;
        }
    }

    void Simulation::createDefects()
    {
        for (int i = 0; i < m_world->grid()->volume (); ++i)
        {
            if (m_world->randomNumberGenerator()->random() < m_world->parameters()->defectPercentage)
            {
                m_world->grid()->setSiteID(i, 1);
                m_world->defectSiteIDs()->push_back(i);
            }
            else
            {
                m_world->grid()->setSiteID(i, 0);
            }
        }
    }

    void Simulation::createSource()
    {
        m_world->source()->setSite(m_world->grid()->volume());
        m_world->grid()->setAgent(m_world->grid()->volume(), m_world->source());
        m_world->grid()->setSiteID(m_world->grid()->volume(), 2);
        m_world->source()->setNeighbors(neighborsSource());
        m_world->source()->setMaxCharges( m_world->parameters()->chargePercentage * double (m_world->grid()->volume()) );
    }

    void Simulation::createDrain()
    {
        m_world->drain()->setSite(m_world->grid()->volume() + 1);
        m_world->grid()->setAgent(m_world->grid()->volume() + 1, m_world->drain());
        m_world->grid()->setSiteID(m_world->grid()->volume() + 1, 3);
        m_world->drain()->setNeighbors(neighborsDrain());
    }

    void Simulation::seedCharges()
    {
        for (int i = 0; i < m_world->source()->maxCharges();)
        {
            // Randomly select a site, ensure it is suitable, if so add a charge agent
            int site = m_world->randomNumberGenerator()->integer(0,m_world->grid()->volume()-1);
            if (m_world->grid()->siteID (site) == 0 && m_world->grid()->agent (site) == 0)
            {
                ChargeAgent *charge = new ChargeAgent (m_world, site);
                m_world->charges()->push_back (charge);
                m_world->source()->incrementCharge ();
                ++i;
            }
        }
    }

    QVector<int> Simulation::neighborsDrain()
    {
        // Now to assign nearest neighbours
        QVector<int>neighbors(0, 0);
        // Loop over layers
        for (int layer = 0; layer < m_world->grid()->depth (); layer++)
        {
            // Get column in this layer that is closest to the source
            QVector < int >column_neighbors_in_layer =
                m_world->grid()->col (m_world->grid()->width () - 1, layer);
            // Loop over this column
            for (int column_site = 0;
                    column_site < column_neighbors_in_layer.size (); column_site++)
            {
                neighbors.push_back (column_neighbors_in_layer[column_site]);
            }
        }
        // Remove defects as possible neighbors
        for ( int i = 0; i < m_world->defectSiteIDs()->size(); i++ )
        {
            int j = m_world->defectSiteIDs()->at(i);
            int k = neighbors.indexOf(j);
            if ( k != -1 ) { neighbors.remove(k); }
        }
        return neighbors;
    }

    QVector<int> Simulation::neighborsSource()
    {
        // Now to assign nearest neighbours
        QVector < int >neighbors(0, 0);
        // Loop over layers
        for (int layer = 0; layer < m_world->grid()->depth (); layer++)
        {
            // Get column in this layer that is closest to the source
            QVector < int >column_neighbors_in_layer =
                m_world->grid()->col (0, layer);
            // Loop over this column
            for (int column_site = 0;
                    column_site < column_neighbors_in_layer.size (); column_site++)
            {
                neighbors.push_back (column_neighbors_in_layer[column_site]);
            }
        }
        // Remove defects as possible neighbors
        for ( int i = 0; i < m_world->defectSiteIDs()->size(); i++ )
        {
            int j = m_world->defectSiteIDs()->at(i);
            int k = neighbors.indexOf(j);
            if ( k != -1 ) { neighbors.remove(k); }
        }
        return neighbors;
    }

    QVector<int> Simulation::neighborsSite(int site)
    {
        // Return the indexes of all nearest neighbours
        QVector<int>     nList(0);
        int col = m_world->grid()->getColumn(site);
        int row = m_world->grid()->getRow(site);
        int lay = m_world->grid()->getLayer(site);
        int hoppingRange = 1;

        switch ( hoppingRange )
        {
         case 1:
         {
          // To the west
          if (col > 0)
            nList.push_back(m_world->grid()->getIndex(col-1,row,lay));
          // To the east
          if (col < m_world->grid()->width() - 1)
            nList.push_back(m_world->grid()->getIndex(col+1,row,lay));
          // To the south
          if (row > 0)
            nList.push_back(m_world->grid()->getIndex(col,row-1,lay));
          // To the north
          if (row < m_world->grid()->height() - 1)
            nList.push_back(m_world->grid()->getIndex(col,row+1,lay));
          // Below
          if (lay > 0)
            nList.push_back(m_world->grid()->getIndex(col,row,lay-1));
          // Above
          if (lay < m_world->grid()->depth() - 1)
            nList.push_back(m_world->grid()->getIndex(col,row,lay+1));
          // Now for the source and the drain....
          if (col == 0)
            nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 1);
          if (col == m_world->grid()->width() - 1)
            nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 2);
          // Now we have all the nearest neighbours - between 4 and 6 in this case
          return nList;
          break;
         }
         case 2:
         {
           if (col > 0)
            {
             nList.push_back( m_world->grid()->getIndex(col-1,row,lay));
             if (row < m_world->grid()->height() -1)
              nList.push_back( m_world->grid()->getIndex(col-1,row+1,lay));
             if (row > 0)
              nList.push_back( m_world->grid()->getIndex(col-1,row-1,lay));
             if (col > 1)
              nList.push_back( m_world->grid()->getIndex(col-2,row,lay));
            }
           if (col < m_world->grid()->width() - 1)
            {
             nList.push_back( m_world->grid()->getIndex(col+1,row,lay));
             if (row < m_world->grid()->height() - 1)
              nList.push_back( m_world->grid()->getIndex(col+1,row+1,lay));
             if (row > 0)
              nList.push_back( m_world->grid()->getIndex(col+1,row-1,lay));
             if (col < m_world->grid()->width() - 2)
              nList.push_back( m_world->grid()->getIndex(col+2,row,lay));
            }
           if (row > 0)
            {
             nList.push_back( m_world->grid()->getIndex(col,row-1,lay));
             if (row > 1)
              nList.push_back( m_world->grid()->getIndex(col,row-2,lay));
            }
           if (row < m_world->grid()->height() - 1)
            {
             nList.push_back( m_world->grid()->getIndex(col,row+1,lay));
             if (row < m_world->grid()->height() - 2)
              nList.push_back( m_world->grid()->getIndex(col,row+2,lay));
            }
           if (lay > 0)
            {
             nList.push_back( m_world->grid()->getIndex(col,row,lay-1));
             if (lay > 1)
              nList.push_back( m_world->grid()->getIndex(col,row,lay-2));
             if (col > 0)
              {
               nList.push_back( m_world->grid()->getIndex(col-1,row,lay-1));
               if (row < m_world->grid()->height() -1)
                nList.push_back( m_world->grid()->getIndex(col-1,row+1,lay-1));
               if (row > 0)
                nList.push_back( m_world->grid()->getIndex(col-1,row-1,lay-1));
              }
             if (col < m_world->grid()->width() - 1)
              {
               nList.push_back( m_world->grid()->getIndex(col+1,row,lay-1));
               if (row < m_world->grid()->height() - 1)
                nList.push_back( m_world->grid()->getIndex(col+1,row+1,lay-1));
               if (row > 0)
                nList.push_back( m_world->grid()->getIndex(col+1,row-1,lay-1));
              }
             if (row > 0)
              nList.push_back( m_world->grid()->getIndex(col,row-1,lay-1));
             if (row < m_world->grid()->height() - 1)
              nList.push_back( m_world->grid()->getIndex(col,row+1,lay-1));
            }
           if (lay < m_world->grid()->depth()-1)
            {
             nList.push_back( m_world->grid()->getIndex(col,row,lay+1));
             if (lay < m_world->grid()->depth()-2)
              nList.push_back( m_world->grid()->getIndex(col,row,lay+2));
             if (col > 0)
              {
               nList.push_back( m_world->grid()->getIndex(col-1,row,lay+1));
               if (row < m_world->grid()->height() -1)
                nList.push_back( m_world->grid()->getIndex(col-1,row+1,lay+1));
               if (row > 0)
                nList.push_back( m_world->grid()->getIndex(col-1,row-1,lay+1));
              }
             if (col < m_world->grid()->width() - 1)
              {
               nList.push_back( m_world->grid()->getIndex(col+1,row,lay+1));
               if (row < m_world->grid()->height() - 1)
                nList.push_back( m_world->grid()->getIndex(col+1,row+1,lay+1));
               if (row > 0)
                nList.push_back( m_world->grid()->getIndex(col+1,row-1,lay+1));
              }
             if (row > 0)
              nList.push_back( m_world->grid()->getIndex(col,row-1,lay+1));
             if (row < m_world->grid()->height() - 1)
              nList.push_back( m_world->grid()->getIndex(col,row+1,lay+1));
            }
          if (col == 0)
            nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 1);
          if (col == m_world->grid()->width() - 1)
            nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 2);
          return nList;
          break;
         }
         default:
         {
          int cutoff = hoppingRange * hoppingRange;
          for ( int z = (lay - hoppingRange); z < int(lay + hoppingRange + 1); z += 1 )
          {
           if ( z >= 0 && z < int(m_world->grid()->depth()) )
           {
            for ( int x = (col - hoppingRange); x < int(col + hoppingRange + 1); x += 1 )
            {
             if ( x >= 0 && x < int(m_world->grid()->width()) )
             {
              for ( int y = (row - hoppingRange); y < int(row + hoppingRange + 1); y += 1 )
              {
               if ( y >= 0 && y < int(m_world->grid()->height()) )
               {
                if ( !(x == int(col) && y == int(row) && z == int(lay)) )
                {
                 int r2 = ( col - x ) * ( col - x ) + ( row - y ) * ( row - y ) + ( lay - z ) * ( lay - z );
                 if ( r2 <= cutoff )
                 {
                  nList.push_back(m_world->grid()->getIndex(x,y,z));
                 }
                }
               }
              }
             }
            }
           }
          }
          if (col == 0)
            nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 1);
          if (col == m_world->grid()->width() - 1)
            nList.push_back(m_world->grid()->getIndex(m_world->grid()->width()-1,m_world->grid()->height()-1,m_world->grid()->depth()-1) + 2);
          return nList;
          break;
         }
        }
    }

} // End namespace Langmuir
