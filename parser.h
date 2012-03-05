#ifndef PARSER_H
#define PARSER_H

#include <QObject>

#include "boost/program_options.hpp"

#include "parameters.h"

namespace Langmuir
{

class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = 0);
    void parse(const QString& fileName = "");
    void parse(int &argc, char **argv);
    SimulationParameters& parameters();

private:

    void describeParameters();

    boost::program_options::positional_options_description m_description_pos;
    boost::program_options::options_description            m_description_sim;
    boost::program_options::options_description            m_description_gui;
    boost::program_options::options_description            m_description;
    boost::program_options::variables_map                  m_variables;

    SimulationParameters m_parameters;
};

//declare validate functions for custom types
void validate(boost::any& v, const std::vector<std::string>& values, QString*, int);

}
#endif // PARSER_H
