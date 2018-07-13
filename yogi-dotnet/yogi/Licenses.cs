using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    partial class Api
    {
        // === YOGI_GetLicense ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr GetLicenseDelegate();

        public static GetLicenseDelegate YOGI_GetLicense
            = Library.GetDelegateForFunction<GetLicenseDelegate>(
                "YOGI_GetLicense");

        // === YOGI_Get3rdPartyLicenses ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr Get3rdPartyLicensesDelegate();

        public static Get3rdPartyLicensesDelegate YOGI_Get3rdPartyLicenses
            = Library.GetDelegateForFunction<Get3rdPartyLicensesDelegate>(
                "YOGI_Get3rdPartyLicenses");
    }

    /// <summary>
    /// String containing the Yogi license.
    /// </summary>
    public static string License
    {
        get
        {
            return Marshal.PtrToStringAnsi(Api.YOGI_GetLicense());
        }
    }

    /// <summary>
    /// String containing licenses of the 3rd party libraries compiled into Yogi Core.
    /// </summary>
    public static string ThirdPartyLicenses
    {
        get
        {
            return Marshal.PtrToStringAnsi(Api.YOGI_Get3rdPartyLicenses());
        }
    }
}
