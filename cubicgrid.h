#ifndef CUBICGRID_H
#define CUBICGRID_H

#include "agent.h"

#include <QTextStream>
#include <QVector>
#include <QString>
#include <QObject>
#include <QDebug>

namespace Langmuir
{

class World;

/**
 * @brief A class to hold Agents, calculate their positions, and store the background potential
 *
 * The x-direction
 *   - perpendicular to the electrodes
 *   - runs from left to right
 *   - corresponds to the dimension called \b length.
 *
 * The y-direction
 *   - parallel to the electrodes
 *   - runs from bottom to top
 *   - corresponds to the dimension called \b width.
 *
 * The z-direction
 *   - parallel to the electrodes
 *   - runs from back to front
 *   - corresponds to the dimension called \b height.
 */
class Grid : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Grid)
    Q_ENUMS(CubeFace)

public:
    /**
     * @brief A way to indicate the faces of a cube
     */
    enum CubeFace
    {
        //! x =  0, yz plane
        Left       =   0,

        //! x = lx, yz plane
        Right      =   1,

        //! z =  0, xy plane
        Top        =   2,

        //! z = lz, xy plane
        Bottom     =   3,

        //! y =  0, xz plane
        Front      =   4,

        //! y = ly, xz plane
        Back       =   5,

        //! undefined face
        NoFace     =   6
    };
    static QString toQString(const Grid::CubeFace e);

    /**
     * @brief Create a grid
     * @param world reference to the world object
     * @param parent QObject this belongs to
     */
    Grid(World &world, QObject *parent = 0);

    /**
     * @brief Destroy the grid
     */
    ~Grid();

    /**
     * @brief Get the number of sites along the x-direction
     */
    int xSize();

    /**
     * @brief Get the number of sites along the y-direction
     */
    int ySize();

    /**
     * @brief Get the number of sites along the z-direction
     */
    int zSize();

    /**
     * @brief Get the number of sites in the xy-plane
     */
    int xyPlaneArea();

    /**
     * @brief Get the total number of sites
     */
    int volume();

    /**
     * @brief Get the distance between two sites
     * @param site1 the first site
     * @param site2 the second site
     */
    double totalDistance(int site1, int site2);

    /**
     * @brief Get the distance along the x-direction between two sites
     * @param site1 the first site
     * @param site2 the second site
     */
    double xDistance(int site1, int site2);

    /**
     * @brief Get the distance along the y-direction between two sites
     * @param site1 the first site
     * @param site2 the second site
     */
    double yDistance(int site1, int site2);

    /**
     * @brief Get the distance along the z-direction between two sites
     * @param site1 the first site
     * @param site2 the second site
     */
    double zDistance(int site1, int site2);

    /**
     * @brief Get the image distance along the x-direction between two sites
     * @param site1 the first site
     * @param site2 the second site (reflected)
     *
     * The second site's x-position is taken to be the negative of its x-value
     * (i.e., the particle is reflected through the yz-plane).
     */
    double xImageDistance(int site1, int site2);

    /**
     * @brief Get the image distance along the y-direction between two sites
     * @param site1 the first site
     * @param site2 the second site (reflected)
     *
     * The second site's y-position is taken to be the negative of its y-value
     * (i.e., the particle is reflected through the xz-plane).
     */
    double yImageDistance(int site1, int site2);

    /**
     * @brief Get the image distance along the z-direction between two sites
     * @param site1 the first site
     * @param site2 the second site (reflected)
     *
     * The second site's z-position is taken to be the negative of its z-value
     * (i.e., the particle is reflected through the xy-plane).
     */
    double zImageDistance(int site1, int site2);

    /**
     * @brief Get the \b integer distance along the x-direction between two sites
     * @param site1 the first site
     * @param site2 the second site
     */
    int xDistancei(int site1, int site2);

    /**
     * @brief Get the \b integer distance along the y-direction between two sites
     * @param site1 the first site
     * @param site2 the second site
     */
    int yDistancei(int site1, int site2);

    /**
     * @brief Get the \b integer distance along the z-direction between two sites
     * @param site1 the first site
     * @param site2 the second site
     */
    int zDistancei(int site1, int site2);

    /**
     * @brief Get the \b integer image distance along the x-direction between two sites
     * @param site1 the first site
     * @param site2 the second site (reflected)
     *
     * The second site's x-position is taken to be the negative of its x-value
     * (i.e., the particle is reflected through the yz-plane).
     */
    int xImageDistancei(int site1, int site2);

    /**
     * @brief Get the \b integer image distance along the y-direction between two sites
     * @param site1 the first site
     * @param site2 the second site (reflected)
     *
     * The second site's y-position is taken to be the negative of its y-value
     * (i.e., the particle is reflected through the xz-plane).
     */
    int yImageDistancei(int site1, int site2);

    /**
     * @brief Get the \b integer image distance along the z-direction between two sites
     * @param site1 the first site
     * @param site2 the second site (reflected)
     *
     * The second site's z-position is taken to be the negative of its z-value
     * (i.e., the particle is reflected through the xy-plane).
     */
    int zImageDistancei(int site1, int site2);

    /**
     * @brief Get the serial site ID
     * @param xIndex x site ID
     * @param yIndex y site ID
     * @param zIndex z site ID
     *
     * The position of a particle in the Grid can be thought of as a 3-tuple of (x, y, z)
     * site IDs.  However, this 3-tuple can be mapped/hashed into a single number using
     * the dimension of the grid, called the "serial site ID", the "s-site ID",
     * or just the "site".
     */
    int getIndexS(int xIndex, int yIndex, int zIndex = 0);

    /**
     * @brief Get the "y-site ID" from the "s-site ID"
     * @param site the "s-site ID"
     *
     * The y-site ID can be thought of as the y-value of the cornor of a Grid site.
     * @see getIndexS
     */
    int getIndexY(int site);

    /**
     * @brief Get the "x-site ID" from the "s-site ID"
     * @param site the "s-site ID"
     *
     * The y-site ID can be thought of as the x-value of the cornor of a Grid site.
     * @see getIndexS
     */
    int getIndexX(int site);

    /**
     * @brief Get the "z-site ID" from the "s-site ID"
     * @param site the "s-site ID"
     *
     * The y-site ID can be thought of as the z-value of the cornor of a Grid site.
     * @see getIndexS
     */
    int getIndexZ(int site);

    /**
     * @brief Get the y-position from the "s-site ID"
     * @param site the "s-site ID"
     *
     * Particles are considered to reside in the "center" of Grid sites.
     * The y-position is therefore the "y-site ID" plus 0.5 in reduced
     * units.
     */
    double getPositionY(int site);

    /**
     * @brief Get the x-position from the "s-site ID"
     * @param site the "s-site ID"
     *
     * Particles are considered to reside in the "center" of Grid sites.
     * The x-position is therefore the "x-site ID" plus 0.5 in reduced
     * units.
     */
    double getPositionX(int site);

    /**
     * @brief Get the z-position from the "s-site ID"
     * @param site the "s-site ID"
     *
     * Particles are considered to reside in the "center" of Grid sites.
     * The z-position is therefore the "z-site ID" plus 0.5 in reduced
     * units.
     */
    double getPositionZ(int site);

    /**
     * @brief Get a pointer to the Agent at a site
     * @param site the "s-site ID"
     * @warning may be NULL if there is no Agent
     */
    Agent * agentAddress(int site);

    /**
     * @brief Get the type of Agent at a site
     * @param site the "s-site ID"
     * @warning if there is no Agent, it should be Agent::Empty
     */
    Agent::Type agentType(int site);

    /**
     * @brief Add some value to the background potential at a site
     * @param site the "s-site ID"
     * @param potential the value to add
     */
    void addToPotential(int site, double potential);

    /**
     * @brief Set the background potential at a site to some value
     * @param site the "s-site ID"
     * @param potential the value to set
     */
    void setPotential(int site, double potential);

    /**
     * @brief Get the background potential at some site
     * @param site the "s-site ID"
     */
    double potential(int site);

    /**
     * @brief Calculate the neighboring sites of a given site
     * @param site the "s-site ID"
     * @param hoppingRange the number of adjacent sites to consider in the calculation
     */
    QVector<int> neighborsSite(int site, int hoppingRange = 1);

    /**
     * @brief Calculate the neighboring sites of a given face of the Grid
     * @param cubeFace the face of the Grid to consider
     */
    QVector<int> neighborsFace(Grid::CubeFace cubeFace);

    /**
     * @brief Calculate the list of sites occupying a given range
     * @param xi starting x-site ID
     * @param xf stopping x-site ID
     * @param yi starting y-site ID
     * @param yf stopping y-site ID
     * @param zi starting z-site ID
     * @param zf stopping z-site ID
     */
    QVector<int> sliceIndex(int xi, int xf, int yi, int yf, int zi, int zf);

    /**
     * @brief Assign an Agent to a site in the Grid
     * @param agent a pointer to the Agent
     * @warning uses Agent::getCurrentSite()
     * @warning site must be Agent::Empty
     *
     * Makes sure the site is empty first.  After assigning the Agent to the site,
     * calculates and assigns the neighbors to the Agent.
     */
    void registerAgent(Agent *agent);

    /**
     * @brief Assign an Agent to a special location
     * @param agent a pointer to the Agent
     * @param cubeFace the face of the Grid
     *
     * Agents such as Sources and Drains do not occupy a site in the Grid, and
     * so must be stored in a special location.
     */
    void registerSpecialAgent(Agent *agent, Grid::CubeFace cubeFace);

    /**
     * @brief Remove an Agent from the Grid
     * @param agent a pointer to the Agent
     */
    void unregisterAgent(Agent *agent);

    /**
     * @brief Remove an Agent from the special list of Agents in the Grid
     * @param agent a pointer to the Agent
     * @param cubeFace the face of the Grid
     */
    void unregisterSpecialAgent(Agent *agent, Grid::CubeFace cubeFace);

    /**
     * @brief Remove a defect from the Grid
     * @param site the "s-site ID"
     */
    void unregisterDefect(int site);

    /**
     * @brief Assign a site to be Agent::Defect.
     * @param site
     */
    void registerDefect(int site);

    /**
     * @brief The total number of special Agents
     */
    int specialAgentCount();

    /**
     * @brief Get a list of special Agents assigned to a specific Grid::CubeFace
     * @param cubeFace the face of the Grid
     */
    QList<Agent *>& getSpecialAgentList(Grid::CubeFace cubeFace);

protected:
    /**
     * @brief Reference to the World object
     */
    World &m_world;

    /**
     * @brief 1D list of Agent pointers, the size of which is the volume of the Grid + the max number of special Agents.
     * @warning some of these may be NULL
     *
     * Each position in the list is mapped to a position in the Grid.  Use getIndexS()
     * to calculate the serial site ID needed to index this list.
     */
    QVector<Agent *> m_agents;

    /**
     * @brief 1D list of site potentials, the size of which is the volume of the Grid + the max number of special Agents.
     *
     * Each position in the list is mapped to a position in the Grid.  Use getIndexS()
     * to calculate the serial site ID needed to index this list.
     */
    QVector<double> m_potentials;

    /**
     * @brief 1D list of Agent types, the size of which is the volume of the Grid + the max number of special Agents.
     * @warning some of these may be Agent::Empty
     *
     * Each position in the list is mapped to a position in the Grid.  Use getIndexS()
     * to calculate the serial site ID needed to index this list.
     */
    QVector<Agent::Type> m_agentType;

    /**
     * @brief A list of lists of special agents, where each sub-list is for a different Grid::CubeFace
     */
    QList< QList<Agent *> > m_specialAgents;

    /**
     * @brief The max number of special Agents allowed
     */
    int m_specialAgentReserve;

    /**
     * @brief The current number of special Agents registered with the Grid
     */
    int m_specialAgentCount;

    /**
     * @brief The number of sites along the x-direction
     */
    int m_xSize;

    /**
     * @brief The number of sites along the y-direction
     */
    int m_ySize;

    /**
     * @brief The number of sites along the z-direction
     */
    int m_zSize;

    /**
     * @brief The number of sites in the xy-plane
     */
    int m_xyPlaneArea;

    /**
     * @brief The number of sites in the yz-plane
     */
    int m_yzPlaneArea;

    /**
     * @brief The number of sites in the xz-plane
     */
    int m_xzPlaneArea;

    /**
     * @brief The total number of sites
     */
    int m_volume;
};

/**
 * @brief Overload QTextStream for the Grid::CubeFace Enum
 */
QTextStream &operator<<(QTextStream &stream,const Grid::CubeFace e);

/**
 * @brief Overload QDebug for the Grid::CubeFace Enum
 */
QDebug operator<<(QDebug dbg,const Grid::CubeFace e);

}
#endif
