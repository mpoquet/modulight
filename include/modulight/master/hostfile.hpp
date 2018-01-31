#ifndef HOSTFILE_HPP
#define HOSTFILE_HPP

#include <QString>
#include <QMap>

namespace modulight
{
/**
 * \addtogroup groupApplication
 * @{
 */

/**
 * @brief The HostFile class handles host files.
 *
 * It allows to read them and provide a simple API to find where a process should be launched.<br/>
 * <br/>
 * Example of the used file format (which supports multilines comments too):
 * @verbatim
// The first line is the default host
localhost

// All instances of process whose executable filename is foo will be launched on 192.168.1.42
foo * 192.168.1.42

bar 0 host1   // Instance 0 of bar will be launched on host1
bar 1-3 host2 // Those from 1 to 3 on host2
bar 4-* host3 // Others on host3
@endverbatim

 * Please note that you have to specify a MPI hostfile to use these.<br/>
 * You can specify an hostfile to MPI thanks to the mpirun command:
@verbatim
mpirun -np 1 --hostfile myMPIHostfile myApplication
@endverbatim
 * In such a file, you can specify what your node names are and tell how many cores they have.<br/>
 * More information can be found on MPI implementations websites, such as <a href=http://www.open-mpi.org/faq/?category=running#simple-spmd-run>OpenMPI</a>.<br/>
 * <br/>
 * Please note that you can also use the --host option. Further information can be found <a href=http://www.open-mpi.org/faq/?category=running#mpirun-host>here</a>.
 */
class HostFile
{
    struct Set
    {
        int inf;
        int sup;

        Set(int i) : inf(i), sup(i) {}
        Set(int i, int s) : inf(i), sup(s) {}

        bool contains(int i) const { return i >= inf && i <= sup; }

        bool operator<(const Set & oth) const // Required to compare keys in a map
        {
            if (inf == oth.inf)
                return sup < oth.sup;

            return inf < oth.inf;
        }

        bool operator!=(const Set & oth) const { return ((inf != oth.inf) || (sup != oth.sup)); }
        bool isDisjointTo(const Set & oth) const { return ((inf > oth.sup) || (oth.inf > sup)); }
    };

public:
    /**
     * @brief Constructor
     */
    HostFile();

    /**
     * @brief Constructs a HostFile by loading a file
     * @param filename The host file to load
     */
    HostFile(const QString & filename);

    /**
     * @brief loads a host file
     * @param filename The filename
     */
    void loadFilename(const QString & filename);

    /**
     * @brief Allows to get the host of a process
     * @param command The command
     * @param instance The instance number
     * @return The corresponding host, as a QString
     */
    QString hostOf(const QString & command, int instance);

private:
    QString readFile(const QString & filename);
    QString uncommented(const QString & string);
    bool isValid();

private:
    QString _default; //! The default hostname
    QMap<QString, QMap<Set, QString> > _hosts; //! Command -> Set -> Hostname
};

/// @}
}

#endif // HOSTFILE_HPP
