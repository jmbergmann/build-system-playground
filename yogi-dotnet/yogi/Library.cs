/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.Runtime.InteropServices;

public static partial class Yogi
{
    internal class Library
    {
        public interface LibUtils
        {
            IntPtr LoadLibrary(string filename);
            void FreeLibrary(IntPtr libHandle);
            IntPtr GetProcAddress(IntPtr libHandle, string fnName);
        }

        public class WindowsLibUtils : LibUtils
        {
            void LibUtils.FreeLibrary(IntPtr handle)
            {
                FreeLibrary(handle);
            }

            IntPtr LibUtils.GetProcAddress(IntPtr dllHandle, string name)
            {
                return GetProcAddress(dllHandle, name);
            }

            IntPtr LibUtils.LoadLibrary(string fileName)
            {
                return LoadLibrary(fileName);
            }

            [DllImport("kernel32")]
            static extern IntPtr LoadLibrary(string fileName);

            [DllImport("kernel32.dll")]
            static extern int FreeLibrary(IntPtr handle);

            [DllImport("kernel32.dll")]
            static extern IntPtr GetProcAddress(IntPtr handle, string procedureName);
        }

        public class UnixLibUtils : LibUtils
        {
            public IntPtr LoadLibrary(string fileName)
            {
                return dlopen(fileName, RTLD_NOW);
            }

            public void FreeLibrary(IntPtr handle)
            {
                dlclose(handle);
            }

            public IntPtr GetProcAddress(IntPtr dllHandle, string name)
            {
                // clear previous errors if any
                dlerror();
                var res = dlsym(dllHandle, name);
                var errPtr = dlerror();
                if (errPtr != IntPtr.Zero)
                {
                    throw new System.Exception("dlsym: " + Marshal.PtrToStringAnsi(errPtr));
                }
                return res;
            }

            const int RTLD_NOW = 2;

            [DllImport("libdl.so")]
            static extern IntPtr dlopen(String fileName, int flags);

            [DllImport("libdl.so")]
            static extern IntPtr dlsym(IntPtr handle, String symbol);

            [DllImport("libdl.so")]
            static extern int dlclose(IntPtr handle);

            [DllImport("libdl.so")]
            static extern IntPtr dlerror();
        }

        static LibUtils utils;
        static string filename;
        static IntPtr dll;

        static Library()
        {
            utils = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
                           ? (LibUtils)new WindowsLibUtils()
                           : (LibUtils)new UnixLibUtils();

            filename = Environment.GetEnvironmentVariable("YOGI_CORE_LIBRARY");
            if (filename == null)
            {
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                {
                    string architecture = System.Environment.Is64BitProcess ? "x64" : "x86";
                    filename = $"yogi-core-{architecture}.dll";
                }
                else
                {
                    filename = "libyogi-core.so";
                }
            }

            dll = utils.LoadLibrary(filename);
            if (dll == IntPtr.Zero)
            {
                throw new DllNotFoundException($"Could not load library {filename}");
            }
        }

        internal static T GetDelegateForFunction<T>(string functionName) where T : class
        {
            IntPtr fn = utils.GetProcAddress(dll, functionName);
            if (fn == IntPtr.Zero)
            {
                throw new MissingMethodException($"Function {functionName} is missing in {filename}");
            }

            return Marshal.GetDelegateForFunctionPointer(fn, typeof(T)) as T;
        }
    }
}
