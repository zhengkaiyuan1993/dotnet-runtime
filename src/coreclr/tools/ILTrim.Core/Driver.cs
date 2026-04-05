// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

using ILCompiler;
using ILCompiler.Dataflow;
using ILCompiler.DependencyAnalysis;
using ILCompiler.DependencyAnalysisFramework;

using Internal.TypeSystem;
using Internal.TypeSystem.Ecma;

namespace Mono.Linker
{
    public partial class Driver
    {
        public int Run(ILogWriter logWriter = null)
        {
            int setupStatus = SetupContext(logWriter);
            if (setupStatus > 0)
                return 0;
            if (setupStatus < 0)
                return 1;

            var tsContext = new ILTrimTypeSystemContext();
            tsContext.ReferenceFilePaths = context.Resolver.ToReferenceFilePaths();

            EcmaModule corelib = tsContext.GetModuleForSimpleName("System.Private.CoreLib");
            tsContext.SetSystemModule(corelib);

            var ilProvider = new ILTrimILProvider();

            Logger logger = new Logger(
                context.LogWriter,
                ilProvider,
                isVerbose: context.LogMessages,
                suppressedWarnings: context.NoWarn,
                singleWarn: false,
                singleWarnEnabledModules: [],
                singleWarnDisabledModules: [],
                suppressedCategories: ["AOT analysis"],
                treatWarningsAsErrors: false,
                warningsAsErrors: new Dictionary<int, bool>(),
                disableGeneratedCodeHeuristics: false);

            var factory = new NodeFactory(context, logger, ilProvider, tsContext);

            // TODO: Should configure based on LinkContext
            //DependencyAnalyzerBase<NodeFactory> analyzer = settings.LogStrategy switch
            //{
            //    LogStrategy.None => new DependencyAnalyzer<NoLogStrategy<NodeFactory>, NodeFactory>(factory, resultSorter: null),
            //    LogStrategy.FirstMark => new DependencyAnalyzer<FirstMarkLogStrategy<NodeFactory>, NodeFactory>(factory, resultSorter: null),
            //    LogStrategy.FullGraph => new DependencyAnalyzer<FullGraphLogStrategy<NodeFactory>, NodeFactory>(factory, resultSorter: null),
            //    LogStrategy.EventSource => new DependencyAnalyzer<EventSourceLogStrategy<NodeFactory>, NodeFactory>(factory, resultSorter: null),
            //    _ => throw new ArgumentException("Invalid log strategy")
            //};
            var analyzer = new DependencyAnalyzer<NoLogStrategy<NodeFactory>, NodeFactory>(factory, resultSorter: null);

            analyzer.ComputeDependencyRoutine += ComputeDependencyNodeDependencies;

            foreach (var input in context.Inputs)
                analyzer.AddRoot(input, "Command line root");

            analyzer.AddRoot(factory.VirtualMethodUse(
                (EcmaMethod)tsContext.GetWellKnownType(WellKnownType.Object).GetMethod("Finalize"u8, null)),
                "Finalizer");

            analyzer.ComputeMarkedNodes();

            var writers = ModuleWriter.CreateWriters(factory, analyzer.MarkedNodeList);
            if (!File.Exists(context.OutputDirectory))
                Directory.CreateDirectory(context.OutputDirectory);
            RunForEach(writers, writer =>
            {
                var ext = writer.AssemblyName == "test" ? ".exe" : ".dll";
                string outputPath = Path.Combine(context.OutputDirectory, writer.AssemblyName + ext);
                using var outputStream = File.OpenWrite(outputPath);
                writer.Save(outputStream);
            });

            // TODO: emit based on LinkContext
            //if (settings.LogFile != null)
            //{
            //    using var logStream = File.OpenWrite(settings.LogFile);
            //    DgmlWriter.WriteDependencyGraphToStream<NodeFactory>(logStream, analyzer, factory);
            //}

            return logger.HasLoggedErrors ? 1 : 0;

            void ComputeDependencyNodeDependencies(List<DependencyNodeCore<NodeFactory>> nodesWithPendingDependencyCalculation) =>
                RunForEach(
                    nodesWithPendingDependencyCalculation.Cast<INodeWithDeferredDependencies>(),
                    node => node.ComputeDependencies(factory));

            void RunForEach<T>(IEnumerable<T> inputs, Action<T> action)
            {
#if !SINGLE_THREADED
                if (context.MaxDegreeOfParallelism == 1)
#endif
                {
                    foreach (var input in inputs)
                        action(input);
                }
#if !SINGLE_THREADED
                else
                {
                    Parallel.ForEach(
                        inputs,
                        new() { MaxDegreeOfParallelism = context.EffectiveDegreeOfParallelism },
                        action);
                }
#endif
            }
        }

        protected virtual LinkContext GetDefaultContext(IReadOnlyList<DependencyNodeCore<NodeFactory>> inputs, ILogWriter logger)
        {
            return new LinkContext(inputs, logger ?? new TextLogWriter(Console.Out), "output")
            {
                TrimAction = AssemblyAction.Link,
                DefaultAction = AssemblyAction.Link,
                KeepComInterfaces = true,
            };
        }

        static List<DependencyNodeCore<NodeFactory>> GetStandardPipeline()
        {
            return new List<DependencyNodeCore<NodeFactory>>();
        }

        public void Dispose() { }
    }
}
