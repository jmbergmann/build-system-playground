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
YOGI_API int YOGI_CreateContext(void** context);

/***************************************************************************//**
 * Creates a new branch.
 *
 * \param[out] branch    Pointer to the branch handle
 * \param[in]  context   The context to use
 * \param[in]  netname   Name of network to join (set to NULL to use the
 *                       machine's hostname)
 * \param[in]  interface Network interface to use (set to NULL for default)
 * \param[in]  advport   Advertising port (set to 0 for default)
 * \param[in]  advint    Advertising interval in ms (set to 0 for default)
 *
 * \returns [=0] #YOGI_OK if successful
 * \returns [<0] An error code in case of a failure (see \ref EC)
 ******************************************************************************/
YOGI_API int YOGI_CreateBranch(void** branch, void* context,
                               const char* netname, const char* interface,
                               int advport, int advint);

//! @}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // YOGI_CORE_H
