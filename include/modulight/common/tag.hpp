#ifndef TAG_HPP
#define TAG_HPP

namespace modulight
{
/**
 * @brief Contains all MPI tags used within a standard Modulight application
 */
namespace tag
{
    /**
     * @brief The Tag enum contains all MPI tags used within a standard Modulight application
     */
    enum Tag
    {
        ARGUMENTS,          //!< The master gives its arguments to the module
        MODULE_DESCRIPTION, //!< The module sends its description to the master
        INSTANCE_INFO,      //!< The master tells the module about its instance number and the number count
        SEQUENCE,           //!< The master gives a sequence of connections to the module
        SEQUENCE_DONE,      //!< The module tells the master it finished its sequence
        SPLIT,              //!< The master tells the module to split
        START_ORDER,        //!< The master tells the module that initial connections are done
        ABORT_LAUNCH,       //!< The master tells the module to don't start
        MODULE_FINISHED,    //!< The module has finished and tells the master about it
        FINISH_ORDER,       //!< The master tells the module he can finish

        NETWORK_REQUEST,    //!< The module requests the network to the master
        NETWORK,            //!< The master sends the Network to the module

        DYNAMIC_REQUEST,    //!< The module sends a dynamic request to the master
        DYNAMIC_ORDER,      //!< The master send a dynamic order to the module
        ORDER_DONE,         //!< The module finished its sequence of orders
        REQUEST_RESULT      //!< The master tells the module if the request was successful or not
    };
}
}

#endif // TAG_HPP
