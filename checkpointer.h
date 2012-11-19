#ifndef CHECKPOINTER_H
#define CHECKPOINTER_H

#include <QObject>
#include <QMap>

#include "parameters.h"

namespace Langmuir
{

class World;
class KeyValueParser;

/**
 * @brief A class to read and write checkpoint files
 *
 * Checkpoint files are essentially the same as input files
 */
class CheckPointer : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief A way to identify different sections in the input file
     */
    enum Section
    {
        Parameters,
        Electrons,
        Holes,
        Defects,
        Traps,
        TrapPotentials,
        RandomState,
        FluxState
    };
    Q_ENUMS(Section)

    /**
     * @brief Create the checkpointer object
     * @param world reference world object
     * @param parent parent QObject
     */
    explicit CheckPointer(World& world, QObject *parent = 0);

    /**
     * @brief load simulation information
     * @param fileName name of input file
     * @param configInfo temporary storage for electrons, holes, etc
     */
    void load(const QString& fileName, ConfigurationInfo &configInfo);

    /**
     * @brief save simulation information
     * @param fileName name of output file
     */
    void save(const QString& fileName = "%stub.chk");

    /**
     * @brief check to see if input stream has failed
     * @param stream input stream
     * @param message the error message to output if stream failed
     */
    void checkStream(std::istream& stream, const QString& message = "");

private:

    /**
     * @brief load electrons sites from input file
     * @param stream the input stream
     * @param configInfo temporary storage for site ids
     */
    std::istream& loadElectrons(std::istream &stream, ConfigurationInfo &configInfo);

    /**
     * @brief load hole sites from input file
     * @param stream the input stream
     * @param configInfo temporary storage for site ids
     */
    std::istream& loadHoles(std::istream &stream, ConfigurationInfo &configInfo);

    /**
     * @brief load defect sites from input file
     * @param stream the input stream
     * @param configInfo temporary storage for site ids
     */
    std::istream& loadDefects(std::istream &stream, ConfigurationInfo &configInfo);

    /**
     * @brief load trap sites from input file
     * @param stream the input stream
     * @param configInfo temporary storage for site ids
     */
    std::istream& loadTraps(std::istream &stream, ConfigurationInfo &configInfo);

    /**
     * @brief load trap energies from input file
     * @param stream the input stream
     * @param configInfo temporary storage for site energies
     */
    std::istream& loadTrapPotentials(std::istream &stream, ConfigurationInfo &configInfo);

    /**
     * @brief load flux state from input file
     * @param stream the input stream
     * @param configInfo temporary storage for flux state
     */
    std::istream& loadFluxState(std::istream &stream, ConfigurationInfo &configInfo);

    /**
     * @brief load parameter from input file
     * @param stream the input stream
     */
    std::istream& loadParameters(std::istream &stream);

    /**
     * @brief load random number generator state from input file
     * @param stream the input stream
     */
    std::istream& loadRandomState(std::istream &stream);

    /**
     * @brief save electron site ids to output file
     * @param stream output stream
     */
    std::ostream& saveElectrons(std::ostream &stream);

    /**
     * @brief save hole site ids to output file
     * @param stream output stream
     */
    std::ostream& saveHoles(std::ostream &stream);

    /**
     * @brief save defect site ids to output file
     * @param stream output stream
     */
    std::ostream& saveDefects(std::ostream &stream);

    /**
     * @brief save trap site ids to output file
     * @param stream output stream
     */
    std::ostream& saveTraps(std::ostream &stream);

    /**
     * @brief save trap energies to output file
     * @param stream output stream
     */
    std::ostream& saveTrapPotentials(std::ostream &stream);

    /**
     * @brief save flux states to output file
     * @param stream output stream
     */
    std::ostream& saveFluxState(std::ostream &stream);

    /**
     * @brief save parameters to output file
     * @param stream output stream
     */
    std::ostream& saveParameters(std::ostream &stream);

    /**
     * @brief save random number generator state to output file
     * @param stream output stream
     */
    std::ostream& saveRandomState(std::ostream &stream);

    /**
     * @brief reference to world object
     */
    World &m_world;
};

inline static std::ostream& operator<<(std::ostream& stream, QString& string)
{
    stream << string.toStdString();
    return stream;
}

inline static std::istream& operator>>(std::istream& stream, QString& string)
{
    std::string temporary;
    stream >> temporary;
    string = QString::fromStdString(temporary);
    return stream;
}

}
#endif // CHECKPOINTER_H
