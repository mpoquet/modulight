#ifndef USERINTERFACE_HPP
#define USERINTERFACE_HPP

#include <QVector>
#include <modulight/common/modulightexception.hpp>

namespace modulight
{
    class Application;
/**
 * @brief Provides an interface to handle both processes and parallel processes
 */
namespace user_interface
{
   /**
    * \addtogroup groupApplication
    * @{
    */

    /**
     * @brief Process handle
     */
    class Process
    {
        friend class modulight::Application;
        Process(int id) :  _id(id) {}
        ~Process() {}

    public:
        /**
         * @brief Gets the process ID, which is unique amongst one application execution
         * @return The process ID
         */
        int id() const { return _id; }

    private:
        int _id;
    };

    /**
     * @brief Parallel process handle
     */
    class ParallelProcess
    {
        friend class modulight::Application;
        ParallelProcess(int size, const QVector<int> & ids, const QVector<user_interface::Process *> processes) :
            _size(size), _ids(ids), _processes(processes) {}
        ~ParallelProcess() {}

    public:
        /**
         * @brief Gets the number of processes which form the parallel process
         * @return  The number of processes which form the parallel process
         */
        int size() const { return _size; }

        /**
         * @brief Gets IDs of the various processes which form the parallel process
         * @return The IDs of the various processes which form the parallel process
         */
        QVector<int> ids() const { return _ids; }

        /**
         * @brief Gets the handles of the various processes which form the parallel process
         * @return The handles of the various processes which form the parallel process
         */
        QVector<user_interface::Process *> processes() const { return _processes; }

        /**
         * @brief Gets the handle of a process which is a part of the parallel process
         * @param index the process index, 0 <= index < the parallel process size
         * @return The handle of a process which is a part of the parallel process
         */
        user_interface::Process * process(int index)
        {
            if (index > _size)
                throw Exception("ParallelProcess::operator[] : index out of range");

            return _processes[index];
        }

    private:
        int _size;
        QVector<int> _ids;
        QVector<user_interface::Process*> _processes;
    };

    /// @}
}

}

#endif // USERINTERFACE_HPP
