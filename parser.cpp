#include "parser.h"
#include <QDebug>

namespace Langmuir
{

Parser::Parser(QObject *parent) :
    QObject(parent),
    m_description_sim("Simulation Options"),
    m_description_gui       ("GUI Options"       ),
    m_description           ("Langmuir Options"  )
{
    describeParameters();
}

void Parser::parse(int &argc, char **argv)
{
    try
    {
        boost::program_options::command_line_parser parser(argc,argv);
        parser.positional(m_description_pos);
        parser.options(m_description);
        boost::program_options::store(parser.run(), m_variables);
        boost::program_options::notify(m_variables);
        if ( m_variables.count("help") )
        {
            std::cout << m_description;
            qFatal("exiting Langmuir");
        }
    }
    catch (std::exception& e)
    {
        qDebug() << e.what();
        qFatal("exit -1");
    }
}

void Parser::parse(const QString &fileName)
{
    try{
        const char *name = NULL;
        if ( ! fileName.isEmpty() )
        {
            name = fileName.toLocal8Bit().constData();
        }
        else if ( ! m_parameters.inputFile.isEmpty() )
        {
            name = m_parameters.inputFile.toLocal8Bit().constData();
        }
        if ( name != NULL )
        {
            boost::program_options::store(
                        boost::program_options::parse_config_file<char>(name, m_description),
                        m_variables);
            boost::program_options::notify(m_variables);
        }
        if ( m_variables.count("help") )
        {
            std::cout << m_description;
            qFatal("exiting Langmuir");
        }
    }
    catch (std::exception& e)
    {
        qDebug() << e.what();
        qFatal("exit -1");
    }
}

void Parser::describeParameters()
{
    try
    {
    m_description_sim.add_options()

            ("help",
             "produce help message")

            ("input.file", boost::program_options::value<QString>(&m_parameters.inputFile),
             "input file name")

            ("use.checkpoint", boost::program_options::value<QString>(&m_parameters.checkFile),
             "checkpoint file name")

            ("simulation.type",boost::program_options::value<QString>(&m_parameters.simulationType),
             "set up the sources and drains a certain way; transistor or solarcell")

            ("random.seed",boost::program_options::value<unsigned int>(&m_parameters.randomSeed),
             "if 0, use the current time; otherwise seed the random number generater to make it predictable")

            ("grid.z",boost::program_options::value<int>(&m_parameters.gridZ),
             "the height of the device")

            ("grid.y",boost::program_options::value<int>(&m_parameters.gridY),
             "the width of the device")

            ("grid.x",boost::program_options::value<int>(&m_parameters.gridX),
             "the length of the device")

            ("coulomb.carriers",boost::program_options::value<bool>(&m_parameters.coulombCarriers),
             "if true, calcualte Coulomb interactions between carriers")

            ("coulomb.defects",boost::program_options::value<bool>(&m_parameters.coulombDefects),
             "if true, calculate Coulomb interactions between defects and carriers")

            ("defects.charge",boost::program_options::value<int>(&m_parameters.defectsCharge),
             "the chrage of charged defects")

            ("output.xyz",boost::program_options::value<bool>(&m_parameters.outputXyz),
             "output trajector frames every iterations.print")

            ("output.ids.on.delete",boost::program_options::value<bool>(&m_parameters.outputIdsOnDelete),
             "output carrier lifetime and pathlength when carriers are deleted")

            ("output.coulomb",boost::program_options::value<bool>(&m_parameters.outputCoulomb),
             "output the coulomb potential every iterations.print (expensive and slow!)")

            ("output.potential",boost::program_options::value<bool>(&m_parameters.outputPotential),
             "output the potential of the grid (including traps)")

            ("image.defects",boost::program_options::value<bool>(&m_parameters.imageDefects),
             "output image of the defects")

            ("image.traps",boost::program_options::value<bool>(&m_parameters.imageTraps),
             "output image of the traps")

            ("output.is.on",boost::program_options::value<bool>(&m_parameters.outputIsOn),
             "if false, produce no output")

            ("output.append",boost::program_options::value<bool>(&m_parameters.outputAppend),
             "if true, append to data files when they already exist")

            ("output.backup",boost::program_options::value<bool>(&m_parameters.outputBackup),
             "if true, backup files if they already exist")

            ("iterations.print",boost::program_options::value<int>(&m_parameters.iterationsPrint),
             "how often to print to the screen")

            ("iterations.real",boost::program_options::value<int>(&m_parameters.iterationsReal),
             "the number of production steps")

            ("output.precision",boost::program_options::value<int>(&m_parameters.outputPrecision),
             "the precision of doubles in certain output files")

            ("output.width",boost::program_options::value<int>(&m_parameters.outputWidth),
             "the width of columns in certain output files")

            ("output.path",boost::program_options::value<QString>(&m_parameters.outputPath),
             "the default path for output files")

            ("output.stub",boost::program_options::value<QString>(&m_parameters.outputStub),
             "how to name output files (ex: out.dat, stub = out)")

            ("electron.percentage",boost::program_options::value<double>(&m_parameters.electronPercentage),
             "the total percentage of negative charges to place")

            ("hole.percentage",boost::program_options::value<double>(&m_parameters.holePercentage),
             "the total percentage of positive charges to place")

            ("seed.charges",boost::program_options::value<bool>(&m_parameters.seedCharges),
             "place charges randomly at start of the simulation")

            ("defect.percentage",boost::program_options::value<double>(&m_parameters.defectPercentage),
             "the total percentage of defects to palce")

            ("trap.percentage",boost::program_options::value<double>(&m_parameters.trapPercentage),
             "the total percentage of traps to place")

            ("trap.potential",boost::program_options::value<double>(&m_parameters.trapPotential),
             "the potential of trap sites")

            ("gaussian.stdev",boost::program_options::value<double>(&m_parameters.gaussianStdev),
             "add a gaussian spread in the potential of traps (trap.potential)")

            ("seed.percentage",boost::program_options::value<double>(&m_parameters.seedPercentage),
             "the percent of traps to place initially, to which the remaining traps will grow around")

            ("voltage.right",boost::program_options::value<double>(&m_parameters.voltageRight),
             "the potential of the right right (x=grid.x) side of the device")

            ("voltage.left",boost::program_options::value<double>(&m_parameters.voltageLeft),
             "the potential of the left (x=0) side of the device")

            ("temperature.kelvin",boost::program_options::value<double>(&m_parameters.temperatureKelvin),
             "the temperature used in the Boltzmann factor")

            ("source.rate",boost::program_options::value<double>(&m_parameters.sourceRate),
             "the probability at which all sources inject charges")

            ("drain.rate",boost::program_options::value<double>(&m_parameters.drainRate),
             "the probability at which all drains accept charges")

            ("use.opencl",boost::program_options::value<bool>(&m_parameters.useOpenCL),
             "turn on OpenCL to speed up Coulomb interactions")

            ("work.x",boost::program_options::value<int>(&m_parameters.workX),
             "size of work groups for OpenCL kernel 3D")

            ("work.y",boost::program_options::value<int>(&m_parameters.workY),
             "size of work groups for OpenCL kernel 3D")

            ("work.z",boost::program_options::value<int>(&m_parameters.workZ),
             "size of work groups for OpenCL kernel 3D")

            ("work.size",boost::program_options::value<int>(&m_parameters.workSize),
             "size of work groups for OpenCL kernel 1D")
            ;

    m_description_pos.add("input.file",1);

    m_description.add(m_description_sim).add(m_description_gui);
    }
    catch (std::exception& e)
    {
        qDebug() << e.what();
        qFatal("exit -1");
    }
}

SimulationParameters& Parser::parameters()
{
    check(m_parameters);
    return m_parameters;
}

}

inline void validate(boost::any& v, const std::vector<std::string>& values, QString*, int)
{
    // Make sure no previous assignment to 'a' was made.
    boost::program_options::validators::check_first_occurrence(v);

    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = boost::program_options::validators::get_single_string(values);

    v = boost::any(QString::fromLocal8Bit(s.c_str()));
}
