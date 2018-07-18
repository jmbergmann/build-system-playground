using System;
using System.IO;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Xunit;

namespace test
{
    public class ConfigurationTest : TestCase
    {
        public ConfigurationTest()
        {
            tempDir = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            Directory.CreateDirectory(tempDir);
        }

        public new void Dispose()
        {
            Directory.Delete(tempDir, true);
            base.Dispose();
        }

        string tempDir;

        [Fact]
        public void ConfigurationFlagsEnum()
        {
            AssertEnumMatches<Yogi.ConfigurationFlags>("YOGI_CFG_");
        }

        [Fact]
        public void CommandLineOptionsEnum()
        {
            foreach (var elem in GetEnumElements<Yogi.CommandLineOptions>())
            {
                if (elem == Yogi.CommandLineOptions.BranchAll)
                {
                    var exceptions = new List<Yogi.CommandLineOptions>();
                    foreach (var x in GetEnumElements<Yogi.CommandLineOptions>())
                    {
                        if (!x.ToString().StartsWith("Branch"))
                        {
                            exceptions.Add(x);
                        }
                    }

                    AssertFlagCombinationMatches("YOGI_CLO_", elem, exceptions);
                }
                else if (elem == Yogi.CommandLineOptions.All)
                {
                    AssertFlagCombinationMatches("YOGI_CLO_", elem);
                }
                else
                {
                    AssertFlagMatches("YOGI_CLO_", elem);
                }
            }
        }

        [Fact]
        public void FlagsProperty()
        {
            var cfg = new Yogi.Configuration(Yogi.ConfigurationFlags.MutableCmdLine);
            Assert.Equal(Yogi.ConfigurationFlags.MutableCmdLine, cfg.Flags);
        }

        [Fact]
        public void UpdateFromCommandLine()
        {
            var cfg = new Yogi.Configuration();

            cfg.UpdateFromCommandLine(new[] {"exe", "-o", "{\"age\": 25}"},
                Yogi.CommandLineOptions.Overrides);
            Assert.Equal(25, (int)cfg.ToJson()["age"]);

            cfg.UpdateFromCommandLine(new List<string>{"exe", "-o", "{\"age\": 18}"},
                Yogi.CommandLineOptions.Overrides);
            Assert.Equal(18, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void UpdateFromJson()
        {
            var cfg = new Yogi.Configuration();
            cfg.UpdateFromJson("{\"age\": 42}");
            Assert.Equal(42, (int)cfg.ToJson()["age"]);
            cfg.UpdateFromJson(JObject.Parse("{\"age\": 88}"));
            Assert.Equal(88, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void UpdateFromFile()
        {
            var filename = Path.Combine(tempDir, "cfg.json");
            File.WriteAllText(filename, "{\"age\": 66}");

            var cfg = new Yogi.Configuration();
            cfg.UpdateFromFile(filename);

            Assert.Equal(66, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void Dump()
        {
            var cfg = new Yogi.Configuration(Yogi.ConfigurationFlags.DisableVariables);
            cfg.UpdateFromJson("{\"age\": 42}");

            Assert.Throws<Yogi.Exception>(() => cfg.Dump(true));

            Assert.DoesNotContain(" ", cfg.Dump());
            Assert.DoesNotContain("\n", cfg.Dump());
            Assert.Contains(" ", cfg.Dump(indentation: 2));
            Assert.Contains("\n", cfg.Dump(indentation: 2));
        }

        [Fact]
        public void ToJson()
        {
            var cfg = new Yogi.Configuration(Yogi.ConfigurationFlags.DisableVariables);
            cfg.UpdateFromJson("{\"age\": 42}");

            Assert.Throws<Yogi.Exception>(() => cfg.ToJson(true));

            Assert.Equal(42, (int)cfg.ToJson()["age"]);
        }

        [Fact]
        public void WriteToFile()
        {
            var filename = Path.Combine(tempDir, "dump.json");

            var cfg = new Yogi.Configuration(Yogi.ConfigurationFlags.DisableVariables);
            cfg.UpdateFromJson("{\"age\": 11}");

            Assert.Throws<Yogi.Exception>(() => cfg.WriteToFile(filename, true));

            cfg.WriteToFile(filename);
            var content = File.ReadAllText(filename);
            Assert.DoesNotContain(" ", content);
            Assert.DoesNotContain("\n", content);
            Assert.Equal(11, (int)JObject.Parse(content)["age"]);

            cfg.WriteToFile(filename, indentation: 2);
            content = File.ReadAllText(filename);
            Assert.Contains(" ", content);
            Assert.Contains("\n", content);
            Assert.Equal(11, (int)JObject.Parse(content)["age"]);
        }
    }
}
