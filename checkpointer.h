#ifndef CHECKPOINTER_H
#define CHECKPOINTER_H

#include <QObject>
#include <QDataStream>

namespace Langmuir
{

class World;

class Checkpointer : public QObject
{
    Q_OBJECT
public:
    explicit Checkpointer(World& world, QObject *parent = 0);

    void load(const QString& fileName);
    void save(const QString& fileName);
    void check(QDataStream& stream);

private:
    World &m_world;
};

}
#endif // CHECKPOINTER_H
