#ifndef OUTPUT_H
#define OUTPUT_H

#include "parameters.h"
#include <QTextStream>
#include <QObject>
#include <QFile>

/**
 * @brief put a newline character in the stream that ignores the streams current FieldWidth
 * @param s stream
 * @return stream
 */
QTextStream & newline(QTextStream& s);

/**
 * @brief put a space in the stream that ignores the streams current FieldWidth
 * @param s stream
 * @return stream
 */
QTextStream & space(QTextStream& s);

namespace LangmuirCore
{

/**
 * @brief Back up a file
 * @param name a relative or absolute file name path
 *
 * Back up the file using the current time and a revision number.
 * The file is backed up as path/file.date.num, where num is
 * determined by examing existing files in path with a similiar
 * form (path/file.current_date.a_number).  The file is renamed,
 * not copied.
 *
 * @warning gives an error if a directory is passed instead of a file
 * @warning gives an error if the file can not be renamed
 */
void backupFile(const QString& name);

/**
 * @brief brief A class to generate file names using the SimulationParameters
 */
class OutputInfo : public QFileInfo
{
public:
    /**
     * @brief Generate file name according to SimulationParameters
     * @param name the file name desired.  The following substitutions can be made:
     *  - \b "%stub", substitutes in SimulationParameters::outputStub
     *  - \b "%step", substitutes in SimulationParameters::currentStep
     * @param par pointer to a SimulationParameters object
     *  - if 0 or NULL, then all substitutions become empty strings
     *
     * The constructor makes useful substitutions into the passed name (deatiled below)
     * as well as making sure the name generated is valid (according to the passed
     * SimulationParameters).  If the directory of the passed name doesn't exist, it
     * will be created.
     */
    OutputInfo(const QString &name, const SimulationParameters *par = 0);
};

/**
 * @brief A class to combine QFile, QTextStream and OutputInfo (QFileInfo).
 *
 * Only for used for output.  Derived from QObject so destruction
 * ensures streams are flushed and files are closed.
 */
class OutputStream : public QObject, public QTextStream
{
    Q_OBJECT
public:

    //! Setup the QTextStream, QFile, and OutputInfo
    /*!
      The parameters are the same as OutputInfo.  Opens the file as
      text and write only.  Will open with QIODevice::Append
      if Outout::Options::AppendMode is given.
      \sa OutputInfo::OutputInfo
     */

    /**
     * @brief Setup the QTextStream, QFile, and OutputInfo
     * @param name file name
     * @param par simulation parameters
     * @param parent QObject this belongs to
     *
     * The parameters are the same as OutputInfo.  Opens the file as
     * text and write only.  Will open with QIODevice::Append
     * if Outout::Options::AppendMode is given.
     *
     * @sa OutputInfo::OutputInfo
     */
    OutputStream(const QString &name, const SimulationParameters *par = 0, QObject *parent = 0);

    /**
      * @brief flush the stream and close the file
      */
   ~OutputStream();

    /**
     * @brief Get the info object to get things like file name and path
     * @return file info object
     */
    const OutputInfo& info();

    /**
     * @brief Get the file object, though you probably have no need for it
     * @return file object
     */
    const QFile& file();

private:
    //!< OutputInfo object that generated file name
    OutputInfo m_info;

    //!< QFile object, the device of this QTextStream
    QFile m_file;
};

}
#endif // OUTPUT_H
