#ifndef CHECKPOINTER_H
#define CHECKPOINTER_H

#include <QObject>
#include <QMap>

#include "parameters.h"

namespace Langmuir
{

class World;
class KeyValueParser;

class CheckPointer : public QObject
{
    Q_OBJECT
public:

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

    explicit CheckPointer(World& world, QObject *parent = 0);
    void load(const QString& fileName, ConfigurationInfo &configInfo);
    void save(const QString& fileName = "%stub.chk");
    void checkStream(std::istream& stream, const QString& message = "");

private:

    std::istream& loadElectrons(std::istream &stream, ConfigurationInfo &configInfo);
    std::istream& loadHoles(std::istream &stream, ConfigurationInfo &configInfo);
    std::istream& loadDefects(std::istream &stream, ConfigurationInfo &configInfo);
    std::istream& loadTraps(std::istream &stream, ConfigurationInfo &configInfo);
    std::istream& loadTrapPotentials(std::istream &stream, ConfigurationInfo &configInfo);
    std::istream& loadFluxState(std::istream &stream, ConfigurationInfo &configInfo);
    std::istream& loadParameters(std::istream &stream);
    std::istream& loadRandomState(std::istream &stream);

    std::ostream& saveElectrons(std::ostream &stream);
    std::ostream& saveHoles(std::ostream &stream);
    std::ostream& saveDefects(std::ostream &stream);
    std::ostream& saveTraps(std::ostream &stream);
    std::ostream& saveTrapPotentials(std::ostream &stream);
    std::ostream& saveFluxState(std::ostream &stream);
    std::ostream& saveParameters(std::ostream &stream);
    std::ostream& saveRandomState(std::ostream &stream);

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
