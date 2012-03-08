#include "parser.h"
#include "output.h"

namespace Langmuir
{

Parser::Parser(QObject *parent) :
    QObject(parent),
    m_description("Langmuir Options")
{
    describeParameters();
}

Parser::~Parser()
{
    foreach(AbstractVariable *var, m_printable)
    {
        delete var;
    }
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
            m_description.add_options()
                ("help",
                 "produce help message");

            createVariable("input.file", m_parameters.inputFile,
             "input file name");

            createVariable("use.checkpoint", m_parameters.checkFile,
             "checkpoint file name");

            createVariable("simulation.type",m_parameters.simulationType,
             "set up the sources and drains a certain way; transistor or "
             "solarcell");

            createVariable("random.seed",m_parameters.randomSeed,
             "if 0, use the current time; otherwise seed the random number "
             "generater to make it predictable");

            createVariable("grid.z",m_parameters.gridZ,
             "the height of the device");

            createVariable("grid.y",m_parameters.gridY,
             "the width of the device");

            createVariable("grid.x",m_parameters.gridX,
             "the length of the device");

            createVariable("coulomb.carriers",m_parameters.coulombCarriers,
             "if true, calcualte Coulomb interactions between carriers");

            createVariable("coulomb.defects",m_parameters.coulombDefects,
             "if true, calculate Coulomb interactions between defects and "
             "carriers");

            createVariable("defects.charge",m_parameters.defectsCharge,
             "the chrage of charged defects");

            createVariable("output.xyz",m_parameters.outputXyz,
             "output trajector frames every iterations.print");

            createVariable("output.ids.on.delete",m_parameters.outputIdsOnDelete,
             "output carrier lifetime and pathlength when carriers are deleted");

            createVariable("output.coulomb",m_parameters.outputCoulomb,
             "output the coulomb potential every iterations.print "
             "(expensive and slow!)");

            createVariable("output.potential",m_parameters.outputPotential,
             "output the potential of the grid including traps");

            createVariable("image.defects",m_parameters.imageDefects,
             "output image of the defects");

            createVariable("image.traps",m_parameters.imageTraps,
             "output image of the traps");

            createVariable("output.is.on",m_parameters.outputIsOn,
             "if false, produce no output");

            createVariable("output.append",m_parameters.outputAppend,
             "if true, append to data files when they already exist");

            createVariable("output.backup",m_parameters.outputBackup,
             "if true, backup files if they already exist");

            createVariable("iterations.print",m_parameters.iterationsPrint,
             "how often to print to the screen");

            createVariable("iterations.real",m_parameters.iterationsReal,
             "the number of production steps");

            createVariable("output.precision",m_parameters.outputPrecision,
             "the precision of doubles in certain output files");

            createVariable("output.width",m_parameters.outputWidth,
             "the width of columns in certain output files");

            createVariable("output.path",m_parameters.outputPath,
             "the default path for output files");

            createVariable("output.stub",m_parameters.outputStub,
             "how to name output files, ex: out.dat, stub = out");

            createVariable("electron.percentage",m_parameters.electronPercentage,
             "the total percentage of negative charges to place");

            createVariable("hole.percentage",m_parameters.holePercentage,
             "the total percentage of positive charges to place");

            createVariable("seed.charges",m_parameters.seedCharges,
             "place charges randomly at start of the simulation");

            createVariable("defect.percentage",m_parameters.defectPercentage,
             "the total percentage of defects to palce");

            createVariable("trap.percentage",m_parameters.trapPercentage,
             "the total percentage of traps to place");

            createVariable("trap.potential",m_parameters.trapPotential,
             "the potential of trap sites");

            createVariable("gaussian.stdev",m_parameters.gaussianStdev,
             "add a gaussian spread in the potential of traps");

            createVariable("seed.percentage",m_parameters.seedPercentage,
             "the percent of traps to place initially, to which the remaining "
             "traps will grow around");

            createVariable("voltage.right",m_parameters.voltageRight,
             "the potential of the right right x=grid.x side of"
             " the device");

            createVariable("voltage.left",m_parameters.voltageLeft,
             "the potential of the left x=0 side of the device");

            createVariable("temperature.kelvin",m_parameters.temperatureKelvin,
             "the temperature used in the Boltzmann factor");

            createVariable("source.rate",m_parameters.sourceRate,
             "the probability at which all sources inject charges");

            createVariable("drain.rate",m_parameters.drainRate,
             "the probability at which all drains accept charges");

            createVariable("use.opencl",m_parameters.useOpenCL,
             "turn on OpenCL to speed up Coulomb interactions");

            createVariable("work.x",m_parameters.workX,
             "size of work groups for OpenCL kernel 3D");

            createVariable("work.y",m_parameters.workY,
             "size of work groups for OpenCL kernel 3D");

            createVariable("work.z",m_parameters.workZ,
             "size of work groups for OpenCL kernel 3D");

            createVariable("work.size",m_parameters.workSize,
             "size of work groups for OpenCL kernel 1D");

    m_description_pos.add("input.file",1);
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

void Parser::saveParameters()
{
    if (m_parameters.outputIsOn)
    {
        OutputStream stream("%path/%stub.inp",&m_parameters);
        foreach(AbstractVariable *var, m_printable)
        {
            stream << *var;
        }
    }
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
