#ifndef KEYVALUEPARSER_H
#define KEYVALUEPARSER_H

#include <QObject>

#include "variable.h"
#include "parameters.h"
#include "world.h"

namespace Langmuir
{

/**
 * @brief A class to read the parameters and store them in the correct place
 *
 * The location of the SimulationParameters object for the entire simulation
 * is a private variable of this class.
 *
 * To add new variables, follow these steps:
 *   - declare the new variable in the SimulationParameters struct (parameters.h)
 *   - assign the default value of the new variable in the SimulationParameters constructor (parameters.h)
 *   - implement validity checking for the variable in the checkSimulationParameters() function (parameters.h)
 *   - register the variable in the KeyValueParser constructor using the registerVariable() function (keyvalueparser.h)
 *   - to use non-standard types, you must overload certain template functions in variable.h.  See, for example,
 *     overloads for QDateTime in variable.h.
 */
class KeyValueParser : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Create a KeyValueParser
     * @param world reference to World Object
     * @param parent QObject this belongs to
     *
     * Add calls to registerVariable() to add new variables to the simulation.
     */
    KeyValueParser(World& world, QObject *parent = 0);

    /**
     * @brief Destroy the KeyValueParser
     */
   ~KeyValueParser();

    /**
     * @brief Get the SimulationParameters
     */
    SimulationParameters& parameters();

    /**
     * @brief Parse a string and assign a value to the correct parameter
     */
    void parse(const QString& line);

    /**
     * @brief Write the parameters to a file in a "key=value" fashion
     */
    void save(const QString& fileName = "%stub.parm");

    /**
     * @brief Get a reference to a variable by name
     * @param name the name of the variable
     */
    Variable& getVariable(const QString& key);

    /**
     * @brief Write the parameters to a std::ostream in a "key=value" fashion
     */
    friend std::ostream& operator<<(std::ostream &stream, const KeyValueParser &keyValueParser);

private:
    /**
     * @brief A map between variable names and variables
     */
    QMap<QString,Variable*> m_variableMap;

    /**
     * @brief A list of variable names in a specific order
     */
    QStringList m_orderedNames;

    /**
     * @brief The simulation parameters.
     */
    SimulationParameters m_parameters;

    /**
     * @brief Reference to World object
     */
    World& m_world;

    /**
     * @brief Register an allowed variable with the parser
     */
    template <typename T> void registerVariable(
        const QString &key,
        T &value,
        Variable::VariableMode mode = 0);
};

template <typename T> void KeyValueParser::registerVariable(
    const QString &key,
    T &value,
    Variable::VariableMode mode)
{
    if (m_variableMap.find(key) != m_variableMap.end())
    {
        qFatal("langmuir: variable already registered; key %s",
               qPrintable(key));
    }
    m_variableMap.insert(key, new TypedVariable<T>(key,value,mode,this));
    m_orderedNames.push_back(key);
}

}

#endif // KEYVALUEPARSER_H
