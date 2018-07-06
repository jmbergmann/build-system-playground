using System;
using System.IO;
using System.Text.RegularExpressions;
using Xunit;

namespace test
{
    public class TestCase : IDisposable
    {
        static string yogiCoreH;

        static TestCase()
        {
            string filename = new System.Diagnostics.StackTrace(true).GetFrame(0).GetFileName();
            string path = Path.Combine(new FileInfo(filename).Directory.ToString(),
                "../../yogi-core/include/yogi_core.h");
            yogiCoreH = File.ReadAllText(path);
        }

        public static string GetCoreMacroString(string macroName)
        {
            var regex = new Regex($"#define {macroName} ([^ \\s]+).*");
            Match m = regex.Match(yogiCoreH);
            if (!m.Success)
            {
                throw new IOException($"Macro {macroName} not found in yogi_core.h");
            }

            var str = m.Groups[1].Value;
            str = str.Trim();
            if (str.StartsWith('"'))
            {
                str = str.Substring(1, str.Length - 2);
            }

            return str;
        }

        public static int GetCoreMacroInt(string macroName)
        {
            var str = GetCoreMacroString(macroName);
            var regex = new Regex("\\( *1 *<< *(\\d+) *\\)");
            Match m = regex.Match(str);
            if (m.Success)
            {
                var n = int.Parse(m.Groups[1].Value);
                return 1 << n;
            }

            return int.Parse(str);
        }

        public static void AssertEnumElementMatches<T>(string macroPrefix, int enumElement)
        {
            var enumElementName = Enum.GetName(typeof(T), enumElement);
            var macroName = macroPrefix + Regex.Replace(enumElementName,
                "(?<=.)([A-Z])", "_$0").ToUpper();
            var macroVal = GetCoreMacroInt(macroName);
            Assert.Equal(enumElement, macroVal);
        }

        public static void AssertEnumMatches<T>(string macroPrefix)
        {
            foreach (int val in Enum.GetValues(typeof(T)))
            {
                AssertEnumElementMatches<T>(macroPrefix, val);
            }
        }

        public static void AssertFlagMatches<T>(string macroPrefix, int enumElement)
        {
            AssertEnumElementMatches<T>(macroPrefix, enumElement);
        }

        public void Dispose()
        {
            Yogi.LogToConsole();
            Yogi.LogToHook();
            Yogi.LogToFile();
        }
    }
}
