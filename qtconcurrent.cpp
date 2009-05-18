
#include <QtCore/QCoreApplication>
#include <QtCore/QFuture>
#include <QtCore/QtConcurrentMap>

#include <QtCore/QVector>
#include <QtCore/QString>

#include <cmath>

using std::cos;

void test(const QString &str)
{
  qDebug() << "String" << str << "in thread" << QThread::currentThread();
  for (int i = 0; i < 100000000; ++i)
    double f = cos(static_cast<double>(i));
}

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  qDebug() << "Ideal thread count:" << QThread::idealThreadCount();

  QVector<QString> strings(20);
  for (int i = 0; i < strings.size(); ++i)
    strings[i] = "Test string " + QString::number(i);

  qDebug() << "Parallel operations initiated.";
  QFuture<void> future = QtConcurrent::map(strings, test);
  qDebug() << "Calling future.waitForFinished()...";
  future.waitForFinished();
  qDebug() << "Parallel operations complete.";
}
