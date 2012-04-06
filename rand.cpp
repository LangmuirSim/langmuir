#include "rand.h"
#include <fstream>
#include <sstream>

namespace Langmuir
{

Random::Random(quint64 seed, QObject *parent) : QObject(parent)
{
    m_seed = 0;
    if(seed == 0)
    {
        m_seed = static_cast < unsigned int >(time(0));
    }
    else
    {
        m_seed = static_cast < unsigned int >(seed);
    }
    twister = new boost::mt19937(m_seed);

    boost::uniform_01< double > distribution;
    generator01 = new boost::variate_generator < boost::mt19937 &, boost::uniform_01 < double > >(*twister, distribution);
}

Random::~Random()
{
    delete twister;
    delete generator01;
}

quint64 Random::seed()
{
    return m_seed;
}

void Random::seed(quint64 seed)
{
    twister->seed(seed);
    m_seed = seed;
}

double Random::random()
{
    return(*generator01)();
}

double Random::range(const double low, const double high)
{
    boost::uniform_real < double > distribution(low, high);
    boost::variate_generator < boost::mt19937 &, boost::uniform_real < double > >generator(*twister, distribution);
    return generator();
}

double Random::normal(const double mean, const double sigma)
{
    boost::normal_distribution<double> distribution(mean, sigma);
    boost::variate_generator < boost::mt19937 &, boost::normal_distribution < double > >  generator(*twister, distribution);
    return generator();
}

int Random::integer(const int low, const int high)
{
    boost::uniform_int < int > distribution(low, high);
    boost::variate_generator < boost::mt19937 &, boost::uniform_int < int > >generator(*twister, distribution);
    return generator();
}

void Random::testInteger(const int low, const int high, int tries)
{
    for(int i = 0; i < tries; i++)
    {
        int generated = this->integer(low, high);
        Q_ASSERT_X(generated >= low && generated <= high, "testInteger", qPrintable(QString("low: %1 high: %2 generated: %3").arg(low).arg(high).arg(generated)) );
    }
}

bool Random::randomlyChooseYesWithMetropolis(double energyChange, double inversekT)
{
    double randNumber = this->random();
    return randomlyChooseYesWithMetropolis(energyChange, inversekT, randNumber);
}

bool Random::randomlyChooseYesWithMetropolis(double energyChange, double inversekT, double randNumber)
{
    if(energyChange > 0.0)
    {
        if(exp(-energyChange * inversekT)> randNumber)
        {
            return true;
        }
    }
    else
    {
        return true;
    }
    return false;
}

bool Random::randomlyChooseYesWithMetropolisAndCoupling(double energyChange, double inversekT, double coupling)
{
    double randNumber = this->random();
    return randomlyChooseYesWithMetropolisAndCoupling(energyChange, inversekT, coupling, randNumber);
}

bool Random::randomlyChooseYesWithMetropolisAndCoupling(double energyChange, double inversekT, double coupling, double randNumber)
{
    if(energyChange > 0.0)
    {
        if(coupling * exp(-energyChange * inversekT)> randNumber)
        {
            return true;
        }
    }
    else if(coupling > randNumber)
    {
        return true;
    }
    return false;
}

bool Random::randomlyChooseNoWithPercent(double percent)
{
    double randNumber = this->random();
    return randomlyChooseNoWithPercent(percent, randNumber);
}

bool Random::randomlyChooseNoWithPercent(double percent, double randNumber)
{
    if(percent < randNumber)
    {
        return true;
    }
    return false;
}

bool Random::randomlyChooseYesWithPercent(double percent)
{
    double randNumber = this->random();
    return randomlyChooseYesWithPercent(percent, randNumber);
}

bool Random::randomlyChooseYesWithPercent(double percent, double randNumber)
{
    if(percent > randNumber)
    {
        return true;
    }
    return false;
}

QDataStream& operator<<(QDataStream& stream, Random& random)
{
    std::stringstream sstream;
    sstream << *random.twister;
    if (sstream.fail() || sstream.bad())
    {
        qFatal("can not save state of random number generator to QDataStream; "
               "std::stringstream has failed on write");
    }
    QList<quint64> state;
    quint64 value;
    sstream >> value;
    while(!sstream.eof())
    {
        if (sstream.fail() || sstream.bad())
        {
            qFatal("can not save state of random number generator to QDataStream; "
                   "std::stringstream has failed on read");
        }
        state.push_back(value);
        sstream >> value;
    }
    stream << quint64(random.m_seed);
    stream << state;
    return stream;
}

QDataStream& operator>>(QDataStream& stream, Random& random)
{
    stream >> random.m_seed;
    switch (stream.status())
    {
    case QDataStream::Ok:
    {
        break;
    }
    case QDataStream::ReadPastEnd:
    {
        qFatal("can not load state of random number generator from QDataStream, random.seed: "
               "QDataStream::ReadPastEnd");
        break;
    }
    case QDataStream::ReadCorruptData:
    {
        qFatal("can not load state of random number generator from QDataStream, random.seed: "
               "QDataStream::ReadCorruptData");
        break;
    }
    default:
    {
        qFatal("can not load state of random number generator from QDataStream, random.seed: "
               "QDataStream::Status unknown");
    break;
    }
    }
    QList<quint64> state;
    stream >> state;
    switch (stream.status())
    {
    case QDataStream::Ok:
    {
        break;
    }
    case QDataStream::ReadPastEnd:
    {
        qFatal("can not load state of random number generator from QDataStream: "
               "QDataStream::ReadPastEnd");
        break;
    }
    case QDataStream::ReadCorruptData:
    {
        qFatal("can not load state of random number generator from QDataStream: "
               "QDataStream::ReadCorruptData");
        break;
    }
    default:
    {
        qFatal("can not load state of random number generator from QDataStream: "
               "QDataStream::Status unknown");
    break;
    }
    }
    std::stringstream sstream;
    for (int i = 0; i < state.size(); i++)
    {
        sstream << state.at(i) << " ";
        if (sstream.fail() || sstream.bad())
        {
            qFatal("can not load state of random number generator from QDataStream; "
                   "std::stringstream has failed on write");
        }
    }
    sstream >> *random.twister;
    if (sstream.fail() || sstream.bad())
    {
        qFatal("can not load state of random number generator from QDataStream; "
               "std::stringstream has failed on read");
    }
    return stream;
}

QTextStream& operator<<(QTextStream& stream, Random& random)
{
    std::stringstream sstream;
    sstream << *random.twister;
    if (sstream.fail() || sstream.bad())
    {
        qFatal("can not save state of random number generator to QTextStream; "
               "std::stringstream has failed on write");
    }
    QList<quint64> state;
    quint64 value;
    sstream >> value;
    while(!sstream.eof())
    {
        if (sstream.fail() || sstream.bad())
        {
            qFatal("can not save state of random number generator to QTextStream; "
                   "std::stringstream has failed on read");
        }
        state.push_back(value);
        sstream >> value;
    }
    stream << quint64(random.m_seed);
    stream << " " << quint64(state.size());
    for (int i = 0; i < state.size(); i++)
    {
        stream << " " << quint64(state.at(i));
    }
    return stream;
}

QTextStream& operator>>(QTextStream& stream, Random& random)
{
    stream >> random.m_seed;
    switch (stream.status())
    {
    case QTextStream::Ok:
    {
        break;
    }
    case QTextStream::ReadPastEnd:
    {
        qFatal("can not load state of random number generator from QTextStream, random.seed: "
               "QTextStream::ReadPastEnd");
        break;
    }
    case QTextStream::ReadCorruptData:
    {
        qFatal("can not load state of random number generator from QTextStream, random.seed: "
               "QTextStream::ReadCorruptData");
        break;
    }
    default:
    {
        qFatal("can not load state of random number generator from QTextStream, random.seed: "
               "QTextStream::Status unknown");
    break;
    }
    }
    quint64 size;
    stream >> size;
    switch (stream.status())
    {
    case QTextStream::Ok:
    {
        break;
    }
    case QTextStream::ReadPastEnd:
    {
        qFatal("can not load state of random number generator from QTextStream, state.size(): "
               "QTextStream::ReadPastEnd");
        break;
    }
    case QTextStream::ReadCorruptData:
    {
        qFatal("can not load state of random number generator from QTextStream, state.size(): "
               "QTextStream::ReadCorruptData");
        break;
    }
    default:
    {
        qFatal("can not load state of random number generator from QTextStream, state.size(): "
               "QTextStream::Status unknown");
    break;
    }
    }
    std::stringstream sstream;
    for (int i = 0; i < int(size); i++)
    {
        quint64 value = 0;
        stream >> value;
        switch (stream.status())
        {
        case QTextStream::Ok:
        {
            break;
        }
        case QTextStream::ReadPastEnd:
        {
            qFatal("can not load state of random number generator from QTextStream: state.at(%d)"
                   "QTextStream::ReadPastEnd",i);
            break;
        }
        case QTextStream::ReadCorruptData:
        {
            qFatal("can not load state of random number generator from QTextStream: state.at(%d)"
                   "QTextStream::ReadCorruptData",i);
            break;
        }
        default:
        {
            qFatal("can not load state of random number generator from QTextStream: state.at(%d)"
                   "QTextStream::Status unknown",i);
        break;
        }
        }
        sstream << value << " ";
        if (sstream.fail() || sstream.bad())
        {
            qFatal("can not load state of random number generator from QTextStream; state.at(%d)"
                   "std::stringstream has failed on write", i);
        }
    }
    sstream >> *random.twister;
    if (sstream.fail() || sstream.bad())
    {
        qFatal("can not load state of random number generator from QTextStream; "
               "std::stringstream has failed on read");
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, Random& random)
{
    stream << random.m_seed << ' ';
    stream << *random.twister;
    return stream;
}

std::istream& operator>>(std::istream& stream, Random& random)
{
    stream >> random.m_seed;
    if (stream.fail() || stream.bad() || stream.eof())
    {
        qFatal("can not load state of random number generator; random.m_seed\n"
               "std::ifstream has failed on write");
    }
    stream >> *random.twister;
    if (stream.fail() || stream.bad() || stream.eof())
    {
        qFatal("can not load state of random number generator; twister\n"
               "std::ifstream has failed on write");
    }
    return stream;
}

}
