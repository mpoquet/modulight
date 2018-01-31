#ifndef MODULESTATE_HPP
#define MODULESTATE_HPP

namespace modulight
{
    namespace ModuleState
    {
        /**
         * @brief Represents the state of a module
         */
        enum ModuleState
        {
            UNINITIALIZED, //!< The module is not initialized yet
            RUNNING, //!< The module is running, which means it has been initialized and is not finished yet
            RUNNING_WITHOUT_ENVIRONMENT, //!< The module is running but has no environment (it was launched outside a Modulight application)
            LAUNCH_ABORTED, //!< The module launch has been aborted
            FINALIZED //!< The module is not running anymore
        };
    }
}

#endif // MODULESTATE_HPP
