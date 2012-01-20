#ifndef INPUTPARSER_H
#define INPUTPARSER_H

#include <QVariant>
#include <QObject>
#include <QVector>
#include <QMap>

namespace Langmuir
{

struct SimulationParameters
{
    int randomSeed;

    int gridZ;
    int gridY;
    int gridX;

    bool coulombCarriers;
    bool coulombDefects;
    int defectsCharge;

    bool outputIdsOnIteration;
    bool outputIdsAtStart;
    bool outputIdsOnDelete;

    bool outputCoulomb;
    bool outputPotential;
    bool outputImage;

    int iterationsPrint;
    int iterationsReal;
    int iterationsWarmup;
    int outputPrecision;
    int outputWidth;
    QString outputPath;

    double electronPercentage;
    double holePercentage;
    bool seedCharges;
    double defectPercentage;
    double trapPercentage;
    double trapPotential;
    double gaussianAverg;
    double gaussianStdev;
    double seedPercentage;

    double voltageDrain;
    double voltageSource;
    double temperatureKelvin;

    bool useOpenCL;
    int workX;
    int workY;
    int workZ;
    int workSize;
    QString kernelsPath;

    double boltzmannConstant;
    double dielectricConstant;
    double elementaryCharge;
    double permittivitySpace;
    double gridFactor;
    double electrostaticPrefactor;
    int electrostaticCutoff;
    double inverseKT;
    bool okCL;
    unsigned int currentStep;
    unsigned int currentSimulation;
    QString outputStub;
    int iterationsTotal;
    int simulationSteps;

    SimulationParameters();
    void check();
};

class InputParser : public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(InputParser)

public:
    InputParser(const QString &fileName, QObject *parent = 0);
    SimulationParameters& getParameters(int step = 0);
    QString mapToQString();
    int steps();

private:
    SimulationParameters m_parameters;
    QMap<QString, QVector<QVariant> > m_map;
    void parseFile(const QString &fileName);
    template <typename T> void convert( QString key );
    template <typename T> void parameter( QString key, T &parameter, int step, QList<T> validList = QList<T>() );
};

template <> inline void InputParser::convert<int>( QString key )
{
    if ( ! m_map.contains(key) ) { qFatal("error: map does not key %s",qPrintable(key)); }
    for ( int i = 0; i < m_map[key].size(); i++ )
    {
        if ( !( m_map[key][i].canConvert<int>() &&
                m_map[key][i].convert(QVariant::Int)) )
        {
            qFatal("error: can not convert value to %s",
                   QVariant::typeToName(QVariant::Int));
        }
    }
}

template <> inline void InputParser::convert<bool>( QString key )
{
    if ( ! m_map.contains(key) ) { qFatal("error: map does not key %s",qPrintable(key)); }
    for ( int i = 0; i < m_map[key].size(); i++ )
    {
        if ( m_map[key][i].value<QString>().toLower() == "true" )
        {
            m_map[key][i].setValue(true);
        }
        else if ( m_map[key][i].value<QString>().toLower() == "false" )
        {
            m_map[key][i].setValue(false);
        }
        else
        {
            qFatal("error: can not convert value to %s",
                   QVariant::typeToName(QVariant::Bool));
        }
    }
}

template <> inline void InputParser::convert<double>( QString key )
{
    if ( ! m_map.contains(key) ) { qFatal("error: map does not key %s",qPrintable(key)); }
    for ( int i = 0; i < m_map[key].size(); i++ )
    {
        if ( !( m_map[key][i].canConvert<double>() &&
                m_map[key][i].convert(QVariant::Double)) )
        {
            qFatal("error: can not convert value to %s",
                   QVariant::typeToName(QVariant::Double));
        }
    }
}

template <> inline void InputParser::convert<QString>( QString key )
{
    if ( ! m_map.contains(key) ) { qFatal("error: map does not key %s",qPrintable(key)); }
    for ( int i = 0; i < m_map[key].size(); i++ )
    {
        if ( !( m_map[key][i].canConvert<QString>() &&
                m_map[key][i].convert(QVariant::String)) )
        {
            qFatal("error: can not convert value to %s",
                   QVariant::typeToName(QVariant::String));
        }
    }
}

template <typename T> void InputParser::parameter( QString key, T &parameter, int step, QList<T> validList )
{
    if ( step < 0 )
    {
        if ( ! m_map.contains(key) )
        {
            m_map[key] = QVector< QVariant >() << QVariant(parameter);
        }

        convert<T>(key);
        for ( int i = 0; i < m_map[key].size(); i++ )
        {
            T converted = m_map[key].at(i).value<T>();
            if ( validList.size() > 0 && ! ( validList.contains(converted) ) )
            {
                qFatal("invalid value found for key %s",qPrintable(key));
            }
        }
        return;
    }

    if ( m_map.contains(key) )
    {
        QVector< QVariant > &valueList = m_map[key];
        if ( step >= valueList.size() )
        {
            parameter = valueList.last().value<T>();
        }
        else
        {
            parameter = valueList[step].value<T>();
        }
    }
    else
    {
        qFatal("invalid key: %s",qPrintable(key));
    }
}

}
#endif // INPUTPARSER_H
