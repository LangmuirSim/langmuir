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
    boost::program_options::command_line_parser parser(argc,argv);
    parser.positional(m_description_pos);
    parser.options(m_description);
    boost::program_options::store(parser.run(), m_variables);
    boost::program_options::notify(m_variables);
}

void Parser::parse(const QString &fileName)
{
    const char *name = NULL;

    if ( ! fileName.isEmpty() )
    {
        name = fileName.toLocal8Bit().constData();
    }
    //else if ( ! m_parameters.inputFile.isEmpty() )
    //{
    //    name = m_parameters.inputFile.toLocal8Bit().constData();
    //}

    if ( name != NULL )
    {
        boost::program_options::store(
                    boost::program_options::parse_config_file<char>(name, m_description),
                    m_variables);
        boost::program_options::notify(m_variables);
    }
}

void Parser::describeParameters()
{
//    m_description_sim.add_options()

//            ("help",
//             "produce help message")

//            ("input.file", boost::program_options::value<QString>(&m_parameters.inputFile),
//             "input file name")

//            ("grid.z", boost::program_options::value<int>(&m_parameters.gridZ),
//             "size of grid (z direction)");

    m_description_pos.add("input.file",1);

    m_description.add(m_description_sim).add(m_description_gui);
}

const Parameters& Parser::parameters()
{
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
