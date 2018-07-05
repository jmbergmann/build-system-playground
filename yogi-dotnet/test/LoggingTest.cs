using System;
using System.Reflection;
using System.IO;
using System.Runtime.CompilerServices;
using Xunit;

namespace test
{
    public class LoggingTest : IDisposable
    {
        public LoggingTest()
        {
            tempDir = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            Directory.CreateDirectory(tempDir);
        }

        public void Dispose()
        {
            Yogi.LogToConsole();
            Yogi.LogToHook();
            Yogi.LogToFile();

            Directory.Delete(tempDir, true);
        }

        string GetMyFilename([CallerFilePath] string filename = null)
        {
            return Path.GetFileName(filename);
        }

        string tempDir;

        [Fact]
        public void VerbosityEnum()
        {
            Helpers.AssertEnumMatches<Yogi.Verbosity>("YOGI_VB_");
        }

        [Fact]
        public void StreamEnum()
        {
            Helpers.AssertEnumMatches<Yogi.Stream>("YOGI_ST_");
        }

        [Fact]
        public void LogToConsole()
        {
            Yogi.LogToConsole(Yogi.Verbosity.Info, Yogi.Stream.Stdout, true);
            Yogi.AppLogger.Log(Yogi.Verbosity.Warning, "Warning message");
            Yogi.LogToConsole(Yogi.Verbosity.Debug, Yogi.Stream.Stdout, false, "%S.%3", "$t - $m");
            Yogi.AppLogger.Log(Yogi.Verbosity.Error, "Error message");
        }

        [Fact]
        public void LogToHook()
        {
            bool called = false;
            Yogi.LogToHook(Yogi.Verbosity.Debug,
                (severity, timestamp, tid, file, line, comp, msg) => {
                    Assert.IsType<Yogi.Verbosity>(severity);
                    Assert.Equal(Yogi.Verbosity.Warning, severity);
                    Assert.IsType<DateTime>(timestamp);
                    Assert.True(timestamp <= DateTime.Now);
                    Assert.IsType<int>(tid);
                    Assert.True(tid > 0);
                    Assert.IsType<string>(file);
                    Assert.NotEmpty(file);
                    Assert.IsType<int>(line);
                    Assert.True(line > 0);
                    Assert.IsType<string>(comp);
                    Assert.NotEmpty(comp);
                    Assert.IsType<string>(msg);
                    Assert.NotEmpty(msg);
                    called = true;
                }
            );

            Yogi.AppLogger.Log(Yogi.Verbosity.Warning, "A warning");
            Assert.True(called);
        }

        [Fact]
        public void LogToFile()
        {
            var filePrefix = Path.Combine(tempDir, "logfile_%Y_");

            var filename = Yogi.LogToFile(Yogi.Verbosity.Info, filePrefix + "1");
            Assert.DoesNotContain("%Y", filename);
            Assert.True(File.Exists(filename));

            filename = Yogi.LogToFile(Yogi.Verbosity.Info, filePrefix + "2", "%S.%3", "$t - $m");
            Assert.DoesNotContain("%Y", filename);
            Assert.True(File.Exists(filename));
        }

        [Fact]
        public void SetComponentsVerbosity()
        {
            Yogi.Logger.SetComponentsVerbosity("App", Yogi.Verbosity.Debug);
            Assert.Equal(Yogi.Verbosity.Debug, Yogi.AppLogger.Verbosity);
            Yogi.Logger.SetComponentsVerbosity("App", Yogi.Verbosity.Info);
            Assert.Equal(Yogi.Verbosity.Info, Yogi.AppLogger.Verbosity);
        }

        [Fact]
        public void LoggerVerbosity()
        {
            var logger = new Yogi.Logger("My logger");
            Assert.Equal(Yogi.Verbosity.Info, logger.Verbosity);
            logger.Verbosity = Yogi.Verbosity.Fatal;
            Assert.Equal(Yogi.Verbosity.Fatal, logger.Verbosity);
        }

        [Fact]
        public void Log()
        {
            var logger = new Yogi.Logger("My logger");

            bool called = false;
            Yogi.LogToHook(Yogi.Verbosity.Debug,
                (severity, timestamp, tid, file, line, comp, msg) => {
                    Assert.Equal(Yogi.Verbosity.Warning, severity);
                    Assert.Equal("My logger", comp);
                    Assert.Equal("Hey dude", msg);
                    Assert.Equal(GetMyFilename(), file);
                    Assert.True(line > 0);
                    called = true;
                }
            );

            logger.Log(Yogi.Verbosity.Warning, "Hey dude");
            Assert.True(called);

            called = false;
            Yogi.LogToHook(Yogi.Verbosity.Debug,
                (severity, timestamp, tid, file, line, comp, msg) => {
                    Assert.Equal("my file", file);
                    Assert.Equal(123, line);
                    called = true;
                }
            );

            logger.Log(Yogi.Verbosity.Warning, "Hey dude", file: "my file", line: 123);
            Assert.True(called);
        }

        [Fact]
        public void AppLogger()
        {
            Assert.IsType<Yogi.AppLoggerType>(Yogi.AppLogger);

            var logger = new Yogi.AppLoggerType();
            logger.Verbosity = Yogi.Verbosity.Warning;
            Assert.Equal(logger.Verbosity, Yogi.AppLogger.Verbosity);

            logger.Verbosity = Yogi.Verbosity.Trace;
            Assert.Equal(logger.Verbosity, Yogi.AppLogger.Verbosity);
        }
    }
}
