#ifndef CLPARSER_H
#define CLPARSER_H

#include <QStringList>
#include <QString>
#include <QObject>
#include <QDebug>
#include <QMap>

namespace Langmuir {

/**
 * @brief A class to parse command line arguments.
 */
class CommandLineParser : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief create the CommandLineParser
     * @param parent QObject this belongs to
     */
    explicit CommandLineParser(QObject *parent = 0);

    /**
     * @brief set the program description
     * @param desc description string
     */
    void setDescription(const QString& desc);

    /**
     * @brief add a flag that has no argument
     * @param flag command line flag
     * @param dest internal key
     * @param help help message
     */
    void addBool(QString flag, QString dest, QString help);

    /**
     * @brief add an argument that has no flag
     * @param dest internal key
     * @param help help message
     */
    void addPositional(QString dest, QString help);

    /**
     * @brief add a flag that has an argument
     * @param flag command line flag
     * @param dest internal key
     * @param help help message
     */
    void add(QString flag, QString dest, QString help);

    /**
     * @brief get value by key and convert to type
     * @param dest internal key
     * @param default_value default value if key not present
     */
    template <typename T> T get(const QString& dest, T default_value);

    /**
     * @brief parse the command line arguments
     * @param args arguments as a list of strings
     */
    void parse(QStringList &args);

    /**
     * @brief get the help string
     */
    QString help();

protected:
    //! map dest->boolean
    QMap<QString,bool> m_isBool;

    //! map dest->boolean
    QMap<QString,int> m_isPositional;

    //! map dest->flag
    QMap<QString,QString> m_flags;

    //! map dest->help
    QMap<QString,QString> m_helps;

    //! map dest->value
    QMap<QString,QString> m_values;

    //! description string
    QString m_description;

    //! list of *remaining* command line arguments
    QStringList m_args;

    //! total number of positional arguments
    unsigned int m_numPositional;

    //! total number of arguments
    unsigned int m_numArguments;

    /**
     * @brief convert value to type
     * @param value value as string
     */
    template <typename T> T convert(const QString& value);
};

template <typename T> T CommandLineParser::get(const QString &dest, T default_value)
{
    if (!m_values.contains(dest)) {
        qDebug() << "langmuir: invalid parser key..." << qPrintable(dest);
        exit(-1);
    }
    if (m_values[dest].isEmpty()) {
        return default_value;
    }
    return convert<T>(m_values[dest]);
}

}
#endif // CLPARSER_H
