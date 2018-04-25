#ifndef YOGI_CORE_H
#define YOGI_CORE_H

//! @defgroup VS Version Macros
//!
//! Various macros describing the version of the header file. Note that those
//! macros only denote the version of the header file which does not necessarily
//! have to match the version of the actual library that is loaded at runtime.
//!
//! @{

#define YOGI_HDR_VERSION "0.0.3" ///< Whole version number
#define YOGI_HDR_VERSION_MAJOR 0 ///< Major version number
#define YOGI_HDR_VERSION_MINOR 0 ///< Minor version number
#define YOGI_HDR_VERSION_PATCH 3 ///< Patch version number

//! @}
//!
//! @defgroup CV Constants
//!
//! Immutable values used in the library such as version numbers and default
//! values for function calls. The values of the constants can be obtained via
//! the YOGI_GetConstant() function.
//!
//! The type of each constant is denoted in brackets after its description.
//!
//! @{

//! Whole version number of the library (string)
#define YOGI_CONST_VERSION_NUMBER 1

//! Major version number of the library (integer)
#define YOGI_CONST_VERSION_MAJOR 2

//! Major version number of the library (integer)
#define YOGI_CONST_VERSION_MINOR 3

//! Major version number of the library (integer)
#define YOGI_CONST_VERSION_PATCH 4

//! Default addresses/interfaces to use for connections and advertising (string)
#define YOGI_CONST_DEFAULT_INTERFACE 5

//! Default port to use for advertising via UDP IPv6 multicasts (integer)
#define YOGI_CONST_DEFAULT_ADV_PORT 6

//! Default advertising interval in milliseconds (integer)
#define YOGI_CONST_DEFAULT_ADV_INTERVAL 7

//! @}
//!
//! @defgroup EC Error Codes
//!
//! YOGI error codes are always < 0 and a human-readable description of an
//! error code can be obtained by calling YOGI_GetErrorString().
//!
//! @{

//! Operation completed successfully
#define YOGI_OK 0

//! Unknown internal error occured
#define YOGI_ERR_UNKNOWN -1

//! The object is still being used by another object
#define YOGI_ERR_OBJECT_STILL_USED -2

//! Insufficient memory to complete the operation
#define YOGI_ERR_BAD_ALLOC -3

//! Invalid parameter
#define YOGI_ERR_INVALID_PARAM -4

//! Invalid Handle
#define YOGI_ERR_INVALID_HANDLE -5

//! Object is of the wrong type
#define YOGI_ERR_WRONG_OBJECT_TYPE -6

//! The operation has been canceled
#define YOGI_ERR_CANCELED -7

//! Operation failed because the object is busy
#define YOGI_ERR_BUSY -8

//! The operation timed out
#define YOGI_ERR_TIMEOUT -9

//! The timer has not been started or already expired
#define YOGI_ERR_TIMER_EXPIRED -10

//! The supplied buffer is too small
#define YOGI_ERR_BUFFER_TOO_SMALL -11

//! @}

#ifndef YOGI_API
# ifdef _MSC_VER
#   define YOGI_API __declspec(dllimport)
# else
#   define YOGI_API
# endif
#endif

//! @defgroup FN Library Functions
//!
//! Description of the various library functions
//!
//! @{

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Get the version of the loaded library.
 *
 * The returned string is human-reable and in the same format as the
 * #YOGI_HDR_VERSION macro. Note that those two strings do not necessarily have
 * to match since the final executable can load a library with a version number
 * different from the version number in the used header file.
 *
 * \returns Library version
 ******************************************************************************/
YOGI_API const char* YOGI_GetVersion();

/***************************************************************************//**
 * Get a description of an error code.
 *
 * Returns a human-readable string describing the given error code. The returned
 * string is always a valid, null-terminated and human-readable string, even if
 * the supplied error code is not valid.
 *
 * \param[in] err Error code (see \ref EC)
 *
 * \returns Description of the error code
 ******************************************************************************/
YOGI_API const char* YOGI_GetErrorString(int err);

/***************************************************************************//**
 * Get the value of a constant (see \ref CV)
 *
 * Depending on the type of constant, which can either be an integer number or a
 * null-terminated string (see \ref CV), the target of the \p dest parameter
 * will be set accordingly: For integer-type constants, \p dest will be treated
 * as a pointer to an int and its target will be set to the value of the
 * constant; for string-type constants, \p dest will be treated as a pointer to
 * a char* string and its target will be set to the address of the string
 * constant.
 *
 * \param[out] dest     Pointer to where the value will be written to
 * \param[in]  constant The constant to retrieve (see \ref CV)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_GetConstant(void* dest, int constant);

/***************************************************************************//**
 * Destroys an object.
 *
 * Tries to destroy the object belonging to the given handle. The call fails and
 * returns #YOGI_ERR_OBJECT_STILL_USED if the object is still being used by other
 * objects that have been created via other library calls.
 *
 * Never destroy a context object from within a handler function that is executed
 * through the same context.
 *
 * Destroying an object will cause any active asynchronous operations to get
 * canceled and the corresponding completion handlers will be invoked with an
 * error code of #YOGI_ERR_CANCELED.
 ******************************************************************************/
YOGI_API int YOGI_Destroy(void* object);

/***************************************************************************//**
 * Destroys all objects.
 *
 * Destroys all previously created objects. All handles will be invalidated and
 * must not be used any more. This effectively resets the library.
 *
 * Destroying objects will cause any active asynchronous operations to get
 * canceled and the corresponding completion handlers will be invoked with an
 * error code of #YOGI_ERR_CANCELED.
 ******************************************************************************/
YOGI_API int YOGI_DestroyAll();

/***************************************************************************//**
 * Creates a context for the execution of asynchronous operations.
 *
 * \param[out] context Pointer to the context handle
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextCreate(void** context);

/***************************************************************************//**
 * Runs the context's event processing loop to execute ready handlers.
 *
 * This function runs handlers (internal and user-supplied such as functions
 * registered through YOGI_ContextPost() or YOGI_TimerStartSingleShot()) that
 * are ready to run, without blocking, until the YOGI_ContextStop() function has
 * been called or there are no more ready handlers.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in]  context The context to use
 * \param[out] count   Number of executed handlers (may be set to NULL)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextPoll(void* context, int* count);

/***************************************************************************//**
 * Runs the context's event processing loop to execute at most one ready
 * handler.
 *
 * This function runs at most one handler (internal and user-supplied such as
 * functions registered through YOGI_ContextPost() or
 * YOGI_TimerStartSingleShot()) that is ready to run, without blocking.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in]  context The context to use
 * \param[out] count   Number of executed handlers (may be set to NULL)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextPollOne(void* context, int* count);

/***************************************************************************//**
 * Runs the context's event processing loop until the context has been stopped.
 *
 * This function blocks while running the context's event processing loop and
 * calling dispatched handlers (internal and user-supplied such as functions
 * registered through RunInContext() or YOGI_TimerStartSingleShot()) until the
 * YOGI_ContextStop() function has been called.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in]  context The context to use
 * \param[out] count   Number of executed handlers (may be set to NULL)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextRun(void* context, int* count);

/***************************************************************************//**
 * Runs the context's event processing loop to execute a single handler has or
 * until the context has been stopped.
 *
 * This function blocks while running the context's event processing loop and
 * calling dispatched handlers (internal and user-supplied such as functions
 * registered through RunInContext() or YOGI_TimerStartSingleShot()) until a
 * single handler function has been executed or until the YOGI_ContextStop()
 * function has been called.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in]  context The context to use
 * \param[out] count   Number of executed handlers (may be set to NULL)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextRunOne(void* context, int* count);

/***************************************************************************//**
 * Runs the context's event processing loop for the specified duration.
 *
 * This function blocks while running the context's event processing loop and
 * calling dispatched handlers (internal and user-supplied such as functions
 * registered through RunInContext() or YOGI_TimerStartSingleShot()) for the
 * specified duration unless YOGI_ContextStop() is called within that time.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in]  context     The context to use
 * \param[out] count       Number of executed handlers (may be set to NULL)
 * \param[in]  seconds     Duration in seconds
 * \param[in]  nanoseconds Sub-second part of the duration
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextRunFor(void* context, int* count, int seconds,
                                int nanoseconds);

/***************************************************************************//**
 * Runs the context's event processing loop for the specified duration to
 * execute at most one handler.
 *
 * This function blocks while running the context's event processing loop and
 * calling dispatched handlers (internal and user-supplied such as functions
 * registered through RunInContext() or YOGI_TimerStartSingleShot()) for the
 * specified duration until a single handler function has been executed,
 * unless YOGI_ContextStop() is called within that time.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in]  context     The context to use
 * \param[out] count       Number of executed handlers (may be set to NULL)
 * \param[in]  seconds     Duration in seconds
 * \param[in]  nanoseconds Sub-second part of the duration
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextRunOneFor(void* context, int* count, int seconds,
                                   int nanoseconds);

/***************************************************************************//**
 * Starts an internal thread for running the context's event processing loop.
 *
 * This function starts a threads that runs the context's event processing loop
 * in the background. It relieves the user from having to start a thread and
 * calling the appropriate YOGI_ContextRun... or YOGI_ContextPoll... functions
 * themself. The thread can be stopped using YOGI_ContextStop().
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in] context The context to use
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextRunInBackground(void* context);

/***************************************************************************//**
 * Stops the context's event processing loop.
 *
 * This function signals the context to stop running its event processing loop.
 * This causes YOGI_ContextRun... functions to return as soon as possible and it
 * terminates the thread started via YOGI_ContextRunInBackground().
 *
 * \param[in] context The context to use
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextStop(void* context);

/***************************************************************************//**
 * Blocks until the context's event processing loop is being run or until the
 * specified timeout is reached.
 *
 * If the \p seconds and the \p nanoseconds parameter are both set to 0, the
 * function works in polling mode.
 *
 * If the event processing loop has not been running after the specified
 * timeout, then the YOGI_ERR_TIMEOUT error is returned. This also applies when
 * this function is used in polling mode as described above.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in] context The context to use
 * \param[in] seconds     Timeout in seconds (set to -1 for infinity)
 * \param[in] nanoseconds Sub-second part of the timeout
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextWaitForRunning(void* context, int seconds,
                                        int nanoseconds);

/***************************************************************************//**
 * Blocks until no thread is running the context's event processing loop any
 * or until the specified timeout is reached.
 *
 * If the \p seconds and the \p nanoseconds parameter are both set to 0, the
 * function works in polling mode.
 *
 * If a thread is still running the event processing loop after the specified
 * timeout, then the YOGI_ERR_TIMEOUT error is returned. This also applies when
 * this function is used in polling mode as described above.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in] context The context to use
 * \param[in] seconds     Timeout in seconds (set to -1 for infinity)
 * \param[in] nanoseconds Sub-second part of the timeout
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextWaitForStopped(void* context, int seconds,
                                        int nanoseconds);

/***************************************************************************//**
 * Adds the given functions to the context's event processing queue to be
 * executed and returns immediately.
 *
 * The handler \p fn will will only be executed after this function returns and
 * only by a thread running the context's event processing loop. The only
 * parameter for \p fn will be set to the value of the \p userarg parameter.
 *
 * \param[in] context The context to use
 * \param[in] fn      The function to call from within the given context
 * \param[in] userarg User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextPost(void* context, void (*fn)(void* userarg),
                              void* userarg);

/***************************************************************************//**
 * Creates a new timer.
 *
 * \param[out] timer   Pointer to the timer handle
 * \param[in]  context The context to use
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_TimerCreate(void** timer, void* context);

/***************************************************************************//**
 * Starts the given timer in single shot mode.
 *
 * If the timer is already running, the timer will be canceled first, as if
 * YOGI_TimerCancel() were called explicitly.
 *
 * The parameters of the handler function \p fn are:
 *  -# *res*: YOGI_OK or error code in case of a failure (see \ref EC)
 *  -# *userarg*: Value of the user-specified \p userarg parameter
 *
 * \param[in] timer       The timer to start
 * \param[in] seconds     Timeout in seconds (-1 for infinity)
 * \param[in] nanoseconds Sub-second part of the timeout
 * \param[in] fn          The function to call after the given time passed
 * \param[in] userarg     User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_TimerStart(void* timer, int seconds, int nanoseconds,
                             void (*fn)(int res, void* userarg), void* userarg);

/***************************************************************************//**
 * Cancels the given timer.
 *
 * Canceling a timer will result in the handler function registered via
 * YOGI_TimerStart() to be called with the YOGI_ERR_CANCELED error as first
 * parameter. Note that if the handler is already scheduled for executing, it
 * will be called with YOGI_OK instead.
 *
 * If the timer has not been started or it already expired, this function will
 * return YOGI_ERR_TIMER_EXPIRED.
 *
 * \param[in] timer The timer to cancel
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_TimerCancel(void* timer);

/***************************************************************************//**
 * Creates a new branch.
 *
 * A branch represents an entry point into a YOGI network. It advertises itself
 * via IPv6 multicasts with its unique ID and information required for
 * establishing a connection. If a branch detects other branches on the network,
 * it connects to them via TCP to retrieve further information such as their
 * name, description and network name. If the network names match, two branches
 * attempt to authenticate with each other by securely comparing passwords.
 * Once authentication succeeds and there is no other known branch with the same
 * path then the branches can actively communicate as part of the YOGI network.
 *
 * Advertising and establishing connections can be limited to certain network
 * interfaces via the \p interface parameter. The default is to use all
 * available interfaces.
 *
 * The \p advint parameter can be set to -1 which prevents the branch from
 * actively participating in the YOGI network, i.e. the branch will not
 * advertise itself and it will not authenticate in order to join a network.
 * However, the branch will temporarily connect to other branches in order to
 * obtain more detailed information such as name, description, network name
 * and so on. This is useful for obtaining information about active branches
 * without actually becoming part of the YOGI network.
 *
 * Note: Even though the authentication process via passwords is done in a
 *       secure manor, any further communication is done in plain text.
 *
 * \param[out] branch      Pointer to the branch handle
 * \param[in]  context     The context to use
 * \param[in]  name        Name of the branch (set to NULL to use the format
 *                         PID@hostname with PID being the process ID)
 * \param[in]  description Description of the branch (set to NULL for none)
 * \param[in]  netname     Name of network to join (set to NULL to use the
 *                         machine's hostname)
 * \param[in]  password    Password for the network (set to NULL for none)
 * \param[in]  path        Path of the branch in the network (set to NULL to use
 *                         the format /name where name is the branch's name)
 * \param[in]  interface   Network interface to use (set to NULL for default)
 * \param[in]  advport     Advertising port (set to 0 for default)
 * \param[in]  advint      Advertising interval in ms (set to 0 for default;
 *                         set to -1 for no advertising and no joining networks)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_BranchCreate(void** branch, void* context, const char* name,
                               const char* description, const char* netname,
                               const char* password, const char* path,
                               const char* interface, int advport, int advint);

/***************************************************************************//**
 * Retrieves information about a local branch.
 *
 * This function writes the branch's UUID (16 bytes) in binary form to \p uuid.
 * Any further information is written to \p json in JSON format. The function
 * call fails with the YOGI_ERR_BUFFER_TOO_SMALL error if the produced JSON
 * string does not fit into \p json, i.e. if \p jsonsize is too small. However,
 * in that case, the first \p jsonsize - 1 characters and a trailing zero will
 * be written to \p json.
 *
 * The produced JSON string is as follows, without any unnecessary whitespace:
 *
 *    {
 *      "uuid":                 "6ba7b810-9dad-11d1-80b4-00c04fd430c8",
 *      "name":                 "Fan Controller",
 *      "description":          "Controls a fan via PWM",
 *      "net_name":             "Hardware Control",
 *      "path":                 "/Cooling System/Fan Controller",
 *      "hostname":             "beaglebone",
 *      "pid":                  4124,
 *      "interface":            "TODO",
 *      "advertising_port":     13531,
 *      "advertising_interval": 1.0,
 *      "start_time":           "2018-04-23T18:25:43.511Z",
 *      "active_connections":   3
 *    }
 *
 * \param[in]  branch   The branch handle
 * \param[out] uuid     Pointer to 16 byte array for storing the UUID (can be
 *                      set to NULL)
 * \param[out] json     Pointer to a char array for storing the information (can
 *                      be set to NULL)
 * \param[in]  jsonsize Maximum number of bytes to write to \p json
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_BranchGetInfo(void* branch, void* uuid, char* json,
                                int jsonsize);

/***************************************************************************//**
 * Retrieves information about all remote branches that were discovered.
 *
 * For each of the discovered remote branches, this function will:
 * -# Write the branch's UUID (16 bytes) in binary form to \p uuid.
 * -# Generate a JSON string containing further information to \p json.
 * -# Execute the handler \p fn with YOGI_OK as first argument if \p jsonsize
 *    is as least as large as the length of the generated JSON string
 *
 * If the produced JSON string for the branch does not fit into \p json, i.e. if
 * \p jsonsize is too small, then \p json will be filled with the first
 * \p jsonsize - 1 characters and a trailing zero and \p fn will be called with
 * the YOGI_ERR_BUFFER_TOO_SMALL error for that particular branch.
 *
 * This function will return YOGI_ERR_BUFFER_TOO_SMALL if \p json is not large
 * enough to hold each one of the JSON strings. However, \p fn will still be
 * called for each discovered branch.
 *
 * The produced JSON string is as follows, without any unnecessary whitespace:
 *
 *    {
 *      "uuid":                 "123e4567-e89b-12d3-a456-426655440000",
 *      "name":                 "Pump Safety Logic",
 *      "description":          "Monitors the pump for safety",
 *      "net_name":             "Hardware Control",
 *      "path":                 "/Cooling System/Pump/Safety",
 *      "hostname":             "beaglebone",
 *      "pid":                  3321,
 *      "advertising_interval": 1.0,
 *      "start_time":           "2018-04-23T18:25:43.511Z",
 *      "connected":            true,
 *      "last_connected":       "2018-04-23T18:28:12.333Z",
 *      "last_disconnected":    "2018-04-23T18:27:12.333Z",
 *      "last_error":           ""
 *    }
 *
 * \param[in]  branch   The branch handle
 * \param[out] uuid     Pointer to 16 byte array for storing the UUID (can be
 *                      set to NULL)
 * \param[out] json     Pointer to a char array for storing the information (can
 *                      be set to NULL)
 * \param[in]  jsonsize Maximum number of bytes to write to \p json
 * \param[in]  fn       Handler to call for each discovered branch
 * \param[in]  userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_BranchGetDiscoveredBranches(void* branch, void* uuid,
                                              char* json, int jsonsize,
                                              void (*fn)(int, void* userarg),
                                              void* userarg);

/***************************************************************************//**
 * Asynchronously waits for the list of discovered branches to change.
 *
 * This function will register \p fn to be called once the list of discovered
 * branches changes. The first parameter passed to \p fn is set to YOGI_OK on
 * success and an error code (see \ref EC) in case of a failure.
 *
 * Note: Make sure that the two supplied buffers \p uuid and \p json remain
 *       valid until \p fn has been executed.
 *
 * If the produced JSON string for the branch does not fit into \p json, i.e. if
 * \p jsonsize is too small, then \p json will be filled with the first
 * \p jsonsize - 1 characters and a trailing zero and \p fn will be called with
 * the YOGI_ERR_BUFFER_TOO_SMALL error for that particular branch.
 *
 * The produced JSON string is as described in the
 * YOGI_BranchGetDiscoveredBranches() function.
 *
 * \param[in]  branch   The branch handle
 * \param[out] uuid     Pointer to 16 byte array for storing the UUID (can be
 *                      set to NULL)
 * \param[out] json     Pointer to a char array for storing the information (can
 *                      be set to NULL)
 * \param[in]  jsonsize Maximum number of bytes to write to \p json
 * \param[in]  fn       Handler to call for each discovered branch
 * \param[in]  userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_BranchAwaitDiscoveredBranchesChange(
    void* branch, void* uuid, char* json, int jsonsize,
    void (*fn)(int res, void* userarg), void* userarg);

/***************************************************************************//**
 * Cancels waiting for the list of discovered branches to change.
 *
 * Calling this function will cause the handler registered via
 * YOGI_BranchAwaitDiscoveredBranchesChange() to be called with the
 * YOGI_ERR_CANCELED error.
 *
 * \param[in] branch The branch handle
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_BranchCancelAwaitDiscoveredBranchesChange(void* branch);

//! @}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // YOGI_CORE_H
