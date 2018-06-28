// using System;
// using System.Runtime.InteropServices;
// using System.Runtime.CompilerServices;

// static public partial class Yogi
// {
//     internal partial class LibraryFunctions2
//     {
//         [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//         public delegate int GetConstantDelegate(IntPtr dest, int constant);
//         public static GetConstantDelegate YOGI_GetConstant;

//         static LibraryFunctions2()
//         {
//             RuntimeHelpers.RunClassConstructor(typeof(Library).TypeHandle);

//             IntPtr fn = Library.utils.GetProcAddress(Library.dll, "YOGI_GetConstant");
//             if (fn == IntPtr.Zero)
//             {
//                 throw new MissingMethodException($"Function YOGI_GetConstant is missing in {Library.filename}");
//             }

//             YOGI_GetConstant = Marshal.GetDelegateForFunctionPointer(fn, typeof(GetConstantDelegate)) as GetConstantDelegate;
//         }
//     }

//     public static int GetConstant()
//     {
//         int val = -1;
//         int res = LibraryFunctions2.YOGI_GetConstant(new IntPtr(val), 4);
//         Console.WriteLine("Val: {0} Res: {1}", val, res);

//         return res;
//     }
// }
