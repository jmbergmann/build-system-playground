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

//! Default multicast addresses to use for advertising (string)
#define YOGI_CONST_DEFAULT_ADV_ADDRESS 5

//! Default port to use for advertising via UDP IPv6 multicasts (integer)
#define YOGI_CONST_DEFAULT_ADV_PORT 6

//! Default advertising interval in milliseconds (integer)
#define YOGI_CONST_DEFAULT_ADV_INTERVAL 7

//! Default logging verbosity
#define YOGI_CONST_DEFAULT_LOG_VERBOSITY 8

//! Default textual format for LOG entries
#define YOGI_CONST_DEFAULT_LOG_FORMAT 9

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

//! Could not open a socket
#define YOGI_ERR_OPEN_SOCKET_FAILED -12

//! Could not bind a socket
#define YOGI_ERR_BIND_SOCKET_FAILED -13

//! Could not listen on socket
#define YOGI_ERR_LISTEN_SOCKET_FAILED -14

//! Could not set a socket option
#define YOGI_ERR_SET_SOCKET_OPTION_FAILED -15

//! Invalid regular expression
#define YOGI_ERR_INVALID_REGEX -16

//! @}
//!
//! @defgroup VB Log verbosity
//!
//! Verbosity levels for logging
//!
//! @{

//! Used to disable logging
#define YOGI_VB_NONE -1

//! Fatal errors are error that requires a process restart
#define YOGI_VB_FATAL 0

//! Errors that the system can recover from
#define YOGI_VB_ERROR 1

//! Warnings
#define YOGI_VB_WARNING 2

//! Useful general information about the system state
#define YOGI_VB_INFO 3

//! Information for debugging
#define YOGI_VB_DEBUG 4

//! Detailed debugging
#define YOGI_VB_TRACE 5

//! @}
//!
//! @defgroup BC Boolean Constants
//!
//! Definitions for true and false
//!
//! @{

//! Represents a logical false
#define YOGI_FALSE 0

//! Represents a logical true
#define YOGI_TRUE 1

//! @}
//!
//! @defgroup ST Stream Constants
//!
//! Constants for differentiating between stdout and stderr
//!
//! @{

//! Standard output
#define YOGI_STDOUT 0

//! Standard error output
#define YOGI_STDERR 1

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
 * Allows the YOGI to write library-internal and user logging to stdout or
 * stderr.
 *
 * This function supports colourizing the output if the terminal that the
 * process is running in supports it. The colour used for a log entry depends on
 * the entry's severity. For example, errors will be printed in red and warnings
 * in yellow.
 *
 * Writing to the console can be disabled by setting \p verbosity to
 * #YOGI_VB_NONE.
 *
 * Each log entry contains the *component* tag which describes which part of a
 * program issued the log entry. For entries created by the library itself, this
 * parameter is prefixed with the string "Yogi.", followed by the internal
 * component name. For example, the component tag for a branch would be
 * "Yogi.Branch".
 *
 * The \p fmt parameter describes the textual format of a log entry. The
 * following placeholders can be used:
 *  - *%Y*: Four digit year
 *  - *%m*: Month name as a decimal 01 to 12
 *  - *%d*: Day of the month as decimal 01 to 31
 *  - *%H*: The hour as a decimal number using a 24-hour clock (range 00 to 23)
 *  - *%M*: The minute as a decimal 00 to 59
 *  - *%S*: Seconds as a decimal 00 to 59
 *  - *%T*: The time in 24-hour notation (%H:%M:%S)
 *  - *%3*: Milliseconds as decimal number 000 to 999
 *  - *%6*: Microseconds as decimal number 000 to 999
 *  - *%9*: Nanoseconds as decimal number 000 to 999
 *  - *%V*: Verbosity as three letter string (e.g. WRN or ERR)
 *  - *%X*: Log message
 *  - *%P*: Process ID (PID)
 *  - *%t*: Thread ID
 *  - *%%*: A % sign
 *  - *%f*: Source filename
 *  - *%l*: Source line number
 *  - *%c*: Component that created the entry
 *
 * \param[in] stream    The stream to use (#YOGI_STDOUT or #YOGI_STDERR)
 * \param[in] colour    Use colours in output (#YOGI_TRUE or #YOGI_FALSE)
 * \param[in] fmt       Format of a log entry (set to NULL for default)
 * \param[in] verbosity Maximum verbosity of messages to log to stderr
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_LogToConsole(int stream, int colour, const char* fmt,
                               int verbosity);

/***************************************************************************//**
 * Installs a callback function for receiving log entries.
 *
 * This function can be used to get notified whenever the YOGI library itself or
 * the user produces log messages. These messages can then be processed further
 * in user code.
 *
 * Only one callback function can be registered. Calling LogToHook()
 * again will override the previous function. Setting \p fn to NULL or
 * \p verbosity to #YOGI_VB_NONE will disable the hook.
 *
 * Note: The library will call \p fn from only one thread at a time, i.e. \p fn
 *       does not have to be thread-safe.
 *
 * The parameters passed to \p fn are:
 *  -# *severity*: Severity (verbosity) of the message (see \ref VB)
 *  -# *timestamp*: Timestamp of the message in nanoseconds since 01/01/1970 UTC
 *  -# *tid*: Thread ID
 *  -# *file*: Source file name
 *  -# *line*: Source file line number
 *  -# *comp*: Component that created the entry
 *  -# *msg*: Log message
 *
 * Note: The two string arguments *comp* and *msg* of \p fn are valid only while
 *       \p fn is being executed. Do not access those variables at a later time!
 *
 * \param[in] fn        Callback function
 * \param[in] verbosity Maximum verbosity of messages to call \p fn for
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_LogToHook(void (*fn)(int, long long, int, const char*, int,
                                       const char*),
                            int verbosity);

/***************************************************************************//**
 * Creates log file.
 *
 * This function opens a file to write library-internal and user logging
 * information to. If the file with the given filename already exists then it
 * will be overwritten.
 *
 * Writing to a log file can be disabled by setting \p filename to NULL or
 * \p verbosity to #YOGI_VB_NONE.
 *
 * The \p filename parameter supports the following placeholders:
 *  - *%Y*: Four digit year
 *  - *%m*: Month name as a decimal 01 to 12
 *  - *%d*: Day of the month as decimal 01 to 31
 *  - *%H*: The hour as a decimal number using a 24-hour clock (range 00 to 23)
 *  - *%M*: The minute as a decimal 00 to 59
 *  - *%S*: Seconds as a decimal 00 to 59
 *
 * The \p fmt parameter describes the textual format of a log entry. See the
 * YOGI_LogToConsole() function for supported placeholders.
 *
 * \param[in] filename  Path to the log file (see description for placeholders)
 * \param[in] fmt       Format of a trace entry (set to NULL for default)
 * \param[in] verbosity Maximum verbosity of messages to log to stderr
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_LogToFile(const char* filename, const char* fmt,
                            int verbosity);

/***************************************************************************//**
 * Creates a logger.
 *
 * A logger is an object used for generating log entries that are tagged with
 * the logger's component tag. A logger's component tag does not have to be
 * unique, i.e. multiple loggers can be created using the same \p component
 * parameter.
 *
 * The verbosity of new loggers is #YOGI_VB_INFO by default.
 *
 * \param[out] logger    Pointer to the logger handle
 * \param[in]  component The component tag to use
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_LoggerCreate(void** logger, const char* component);

/***************************************************************************//**
 * Sets the verbosity of a particular logger.
 *
 * The verbosity of a logger acts as a filter. Only messages with a verbosity
 * less than or equal to the given value are being logged.
 *
 * Note: The verbosity of a logger affects only messages logged through that
 *       particular logger, i.e. if two loggers have identical component tags
 *       their verbosity settings are still independent from each other.
 *
 * \param[in] logger    Logger handle (set to NULL for the App logger)
 * \param[in] verbosity Maximum verbosity entries to be logged (see \ref VB)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_LoggerSetVerbosity(void* logger, int verbosity);

/***************************************************************************//**
 * Sets the verbosity of all loggers matching a given component tag.
 *
 * This function finds all loggers whose component tag matches the regular
 * expression given in the \p components parameter and sets their verbosity
 * to \p verbosity.
 *
 * \param[in]  components Regex (ECMAScript) for the component tags to match
 * \param[in]  verbosity  Maximum verbosity entries to be logged (see \ref VB)
 * \param[out] count      Number of matching loggers (can be set to NULL)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_LoggerSetComponentsVerbosity(const char* components,
                                               int verbosity, int* count);

/***************************************************************************//**
 * Creates a log entry.
 *
 * The entry can be generated using a specific logger or, by setting \p logger
 * to NULL, the App logger will be used. The App logger is always present and
 * uses the string "App" as the component tag.
 *
 * \param[in] logger   The logger to use (set to NULL for the App logger)
 * \param[in] severity Severity (verbosity) of the entry (see \ref VB)
 * \param[in] file     Source file name (can be set to NULL)
 * \param[in] line     Source file line number (can be set to 0)
 * \param[in] msg      Log message
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_LoggerLog(void* logger, int severity, const char* file,
                            int line, const char* msg);

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
 * This function must be called outside of any handler function that is executed
 * through a context's event loop.
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
 * \param[in]  context  The context to use
 * \param[out] count    Number of executed handlers (may be set to NULL)
 * \param[in]  duration Duration in nanoseconds (-1 for infinity)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextRun(void* context, int* count, long long duration);

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
 * \param[in]  context  The context to use
 * \param[out] count    Number of executed handlers (may be set to NULL)
 * \param[in]  duration Duration in nanoseconds (-1 for infinity)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextRunOne(void* context, int* count, long long duration);

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
 * If the \p duration parameter is set to 0 then the function works in polling
 * mode.
 *
 * If the event processing loop has not been running after the specified
 * timeout, then the YOGI_ERR_TIMEOUT error is returned. This also applies when
 * this function is used in polling mode as described above.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in] context  The context to use
 * \param[in] duration Duration in nanoseconds (-1 for infinity)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextWaitForRunning(void* context, long long duration);

/***************************************************************************//**
 * Blocks until no thread is running the context's event processing loop any
 * or until the specified timeout is reached.
 *
 * If the \p duration parameter is set to 0 then the function works in polling
 * mode.
 *
 * If a thread is still running the event processing loop after the specified
 * timeout, then the YOGI_ERR_TIMEOUT error is returned. This also applies when
 * this function is used in polling mode as described above.
 *
 * This function must be called from outside any handler functions that are being
 * executed through the context.
 *
 * \param[in] context  The context to use
 * \param[in] duration Duration in nanoseconds (-1 for infinity)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_ContextWaitForStopped(void* context, long long duration);

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
 * \param[in] timer    The timer to start
 * \param[in] duration Duration in nanoseconds (-1 for infinity)
 * \param[in] fn       The function to call after the given time passed
 * \param[in] userarg  User-specified argument to be passed to \p fn
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_TimerStart(void* timer, long long duration,
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
 * via IP broadcasts/multicasts with its unique ID and information required for
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
 * \param[in]  advaddr     Multicast address to use; e.g. 239.255.0.1 for IPv4
 *                         or ff31::8000:1234 for IPv6 (set to NULL for default)
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
                               const char* advaddr, int advport, int advint);

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
 *      "advertising_address":  "ff31::8000:2439",
 *      "advertising_port":     13531,
 *      "advertising_interval": 1.0,
 *      "tcp_server_port":      53332,
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
 *      "tcp_server_port":      43384,
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
