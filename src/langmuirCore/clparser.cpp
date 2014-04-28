#include "clparser.h"

namespace Langmuir {

CommandLineParser::CommandLineParser(QObject *parent) :
    QObject(parent)
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

void CommandLineParser::add(QString flag, QString dest, QString help)
{
    m_isBool[dest] = false;
    m_values[dest] = "";
    m_flags[dest] = flag;
    m_helps[dest] = help;
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

    // check for known flags
    foreach (QString flag, m_args) {
        if (flag.startsWith("--") && !m_flags.values().contains(flag)) {
            qDebug() << qPrintable(help());
            qDebug("unknown option: %s", qPrintable(flag));
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
                    qFatal("expected argument after %s", qPrintable(flag));
                }
            }
        }
    }
}

QString CommandLineParser::help()
{
    QString helpString;
    if (!m_description.isEmpty()) {
        helpString += QString("%1\n").arg(m_description);
    }
    foreach (QString dest, m_flags.keys()) {
        helpString += QString("  %1 %2\n").arg(m_flags[dest], -15).arg(m_helps[dest]);
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
