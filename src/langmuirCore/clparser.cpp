#include "clparser.h"

namespace Langmuir {

CommandLineParser::CommandLineParser(QObject *parent) :
    QObject(parent), m_numPositional(0), m_numArguments(0)
{
}

void CommandLineParser::setDescription(const QString& desc)
{
    m_description = desc;
}

void CommandLineParser::addBool(QString flag, QString dest, QString help)
{
    add(flag, dest, help);
    m_isBool[dest] = true;
    m_values[dest] = "0";
}

void CommandLineParser::addPositional(QString dest, QString help)
{
    add("", dest, help);
    m_isPositional[dest] = m_numPositional + 1;
    ++m_numPositional;
}

void CommandLineParser::add(QString flag, QString dest, QString help)
{
    m_isPositional[dest] = 0;
    m_isBool[dest] = false;
    m_values[dest] = "";
    m_flags[dest] = flag;
    m_helps[dest] = help;
    ++m_numArguments;
}

void CommandLineParser::parse(QStringList &args)
{
    // save args
    m_args = args;

    // check for help
    if (m_args.contains("-h") || m_args.contains("--help")) {
        qDebug() << qPrintable(help());
        exit(-1);
    }

    // calculate the total arguments possible
    int total_possible_args = 1;
    foreach (QString dest, m_flags.keys()) {
        if (m_isPositional[dest]) {
            total_possible_args += 1;
        }
        else {
            if (m_isBool[dest]) {
                total_possible_args += 1;
            }
            else {
                total_possible_args += 2;
            }
        }
    }

    // check the number of arguments passed
    if (m_args.size() > total_possible_args) {
        qDebug() << "langmuir: too many arguments..." <<
                    m_args.size() << ">" << total_possible_args;
        exit(-1);
    }

    // check for known flags
    foreach (QString flag, m_args) {
        if (flag.startsWith("--") && !m_flags.values().contains(flag)) {
            qDebug("langmuir: unknown option... %s", qPrintable(flag));
            exit(-1);
        }
    }

    // loop flags
    foreach (QString dest, m_flags.keys()) {
        QString& flag = m_flags[dest];
        // find index of flag
        int i = m_args.indexOf(flag);
        if (i >= 0) {
            // treat bool flags
            if (m_isBool[dest]) {
                m_values[dest] = "1";
                m_args.removeAt(i);
            }
            // treat key=value flags
            else {
                if (m_args.size() > i) {
                    m_values[dest] = m_args.at(i + 1);
                    m_args.removeAt(i);
                    m_args.removeAt(i);
                }
                else {
                    qDebug() << "langmuir: expected argument after" << qPrintable(flag);
                    exit(-1);
                }
            }
        }
    }

    // parse the positional arguments
    foreach (QString dest, m_flags.keys()) {
        int i = m_isPositional[dest];
        if (i > 0) {
            if (m_args.size() > i) {
                if (m_args.at(i).startsWith("--")) {
                    qDebug() << "langmuir: positional argument starts with --";
                }
                m_values[dest] = m_args.at(i);
                m_args.removeAt(i);
            }
        }
    }

    // check the number of arguments
    if (m_args.size() != 1) {
        qDebug() << "langmuir: can not parse remaining arguments" << m_args;
        exit(-1);
    }
}

QString CommandLineParser::help()
{
    QString helpString;
    if (!m_description.isEmpty()) {
        helpString += QString("%1\n\n").arg(m_description);
    }
    if (m_numPositional) {
        helpString += "positional:\n";
        foreach (QString dest, m_flags.keys()) {
            if (m_isPositional[dest]) {
                helpString += QString("  %1 %2\n").arg(m_isPositional[dest], -15).arg(m_helps[dest]);
            }
        }
    }
    if (m_numArguments - m_numPositional > 0) {
        if (m_numPositional > 0) {
            helpString += '\n';
        }
        helpString += "arguments:\n";
        foreach (QString dest, m_flags.keys()) {
            if (m_isPositional[dest] <= 0) {
                helpString += QString("  %1 %2\n").arg(m_flags[dest], -15).arg(m_helps[dest]);
            }
        }
    }
    return helpString.remove(helpString.size() - 1, 1);
}

template <> QString CommandLineParser::convert(const QString &value)
{
    return value;
}

template <> float CommandLineParser::convert(const QString& value)
{
    return value.toFloat();
}

template <> bool CommandLineParser::convert(const QString& value)
{
    if (value.toLower() == "false") {
        return false;
    }
    if (value.toLower() == "true") {
        return true;
    }
    int converted = value.toInt();
    return converted > 0;
}

template <> int CommandLineParser::convert(const QString& value)
{
    return value.toInt();
}

}
