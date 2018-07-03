using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

static public partial class Yogi
{
    internal partial class Api
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DestroyDelegate(IntPtr handle);
        public static DestroyDelegate YOGI_Destroy
            = Library.GetDelegateForFunction<DestroyDelegate>("YOGI_Destroy");
    }

    /// <summary>
    /// Base class for all "creatable" objects.
    /// </summary>
    public class Object : IDisposable
    {
        /// <summary>
        /// Constructs the object.
        ///
        /// The objects in the depencies list are stored in order to prevent the
        /// garbage collector to destroy them before this objects gets destroyed.
        /// For example, it would prevent the destruction of the context object in
        /// the following case:
        ///    var timer = new Yogi.Timer(new Yogi.Context())
        /// </summary>
        /// <param name="handle">Native handle representing the object.</param>
        /// <param name="dependencies">Other objects that this object depends on.</param>
        public Object(IntPtr handle, [Optional] List<Object> dependencies)
        {
            Handle = handle;
            this.dependencies = dependencies;
        }

        ~Object()
        {
            Destroy();
        }

        public void Dispose()
        {
            Destroy();
            GC.SuppressFinalize(this);
        }

        private void Destroy()
        {
            if (Handle == IntPtr.Zero) return;

            try
            {
                CheckErrorCode(Api.YOGI_Destroy(Handle));
            }
            catch (Exception e)
            {
                string info = "";
                if (e.Failure.ErrorCode == ErrorCode.ObjectStillUsed)
                {
                    info = " Check that you don't have circular dependencies on Yogi objects.";
                }

                throw new System.Exception($"Could not destroy {ToString()}: {e.Message}.{info}");
            }

            Handle = IntPtr.Zero;
        }

        /// <summary>
        /// Returns a string representation of the object.
        /// </summary>
        /// <returns>String representation of the object.</returns>
        public override string ToString()
        {
            string s = GetType().Name;
            s += Handle == IntPtr.Zero ? " [INVALID]" : $" {Handle.ToInt64(),10:0x}";
            return s;
        }

        protected IntPtr Handle { get; private set; }

        private List<Object> dependencies;
    }
}
