// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.IO;
using UnrealBuildTool;
using System.Diagnostics;
using System.Text;
using System.Linq;
using System.Collections;
using System.Collections.Generic;

public class VTSLibrary : ModuleRules
{
	public VTSLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		BuildStuff(Target);
	}

	private bool BuildStuff(ReadOnlyTargetRules Target)
	{
		var isBuilt = BuildLibrary(Target);
		if (!isBuilt)
		{
			return false;
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "vts-browser.lib"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("vts-browser.dll");

			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/VTSLibrary/bin/vts-browser.dll");
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
		}
		else
		{
			return false;
		}
		return true;
	}

	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string ThirdPartyPath
	{
		get
		{
			return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/"));
		}
	}

	private bool BuildLibrary(ReadOnlyTargetRules Target)
	{
		const string buildType = "Release";
		
		var buildDirectory = "model-build-" + buildType;
		var buildPath = Path.Combine(ThirdPartyPath, "generated", buildDirectory);
		Directory.CreateDirectory(buildPath);
		var configureCommand = CreateCMakeBuildCommand(buildPath, buildType);
		Console.WriteLine("> configure >" + configureCommand);
		var configureCode = 0;//ExecuteCommandSync(configureCommand);
		if (configureCode != 0)
		{
			Console.WriteLine("Cannot configure CMake project. Exited with code: "
							  + configureCode);
			return false;
		}
		
		var installCommand = CreateCMakeInstallCommand(buildPath, buildType);
		Console.WriteLine("> install >" + installCommand);
		var buildCode = 0;//ExecuteCommandSync(installCommand);
		if (buildCode != 0)
		{
			Console.WriteLine("Cannot build project. Exited with code: " + buildCode);
			return false;
		}
		
		var installPath = Path.Combine(ThirdPartyPath, "generated");

		var dirs = new List<String> { "lib", "bin", "include" };

		foreach (var d in dirs)
		{
			Directory.CreateDirectory(Path.Combine(ModulePath, d));
		}

		foreach (var item in Directory.EnumerateDirectories(installPath)
			.Where((d) => dirs.Contains(Path.GetFileName(d)))
			.SelectMany((d, i) => Directory.EnumerateFileSystemEntries(d))
			.Where((e) => e.Contains("browser") && !e.Contains("exe"))
		)
		{
			Console.WriteLine(item);
			var rel = item.Substring(installPath.Length + 1);
			var target = Path.Combine(ModulePath, rel);
			if (Directory.Exists(item))
			{
				DirectoryCopy(item, target, true, true);
			}
			else
			{
				File.Copy(item, target, true);
			}
		}

		//var dllTarget = Path.GetFullPath(Path.Combine(ModulePath, "../../..", "Binaries", "Win64"));
		//Directory.CreateDirectory(dllTarget);
		//File.Copy(Path.Combine(installPath, "bin", "vts-browser.dll"), Path.Combine(dllTarget, "vts-browser.dll"), true);
		//File.Copy(Path.Combine(installPath, "lib", "vts-browser.lib"), Path.Combine(dllTarget, "vts-browser.lib"), true);

		return true;
	}

	private string CreateCMakeBuildCommand(string buildDirectory,
										   string buildType)
	{
		const string program = "cmake.exe";
		var currentDir = ModulePath;
		var rootDirectory = Path.Combine(currentDir, "..", "..", "..");
		var installPath = Path.Combine(ThirdPartyPath, "generated");
		var sourceDir = Path.Combine(rootDirectory, "vts-browser-cpp-build-wrapper");

		var arguments = " -S " + sourceDir +
						" -B " + buildDirectory +
						" -A x64 " +
						" -G\"Visual Studio 16 2019\" " +
						" -T host=x64" +
						" -DCMAKE_BUILD_TYPE=" + buildType +
						" -DCMAKE_INSTALL_PREFIX=" + installPath;

		return program + arguments;
	}

	private string CreateCMakeInstallCommand(string buildDirectory,
											 string buildType)
	{
		return "cmake.exe --build " + buildDirectory +
			   " --target install --config relwithdebinfo -j 20";
	}
	private int ExecuteCommandSync(string command)
	{
		Console.WriteLine("Running: " + command);
		var processInfo = new ProcessStartInfo("cmd.exe", "/c " + command)
		{
			CreateNoWindow = true,
			UseShellExecute = false,
			RedirectStandardError = true,
			RedirectStandardOutput = true,
			WorkingDirectory = ModulePath
		};

		StringBuilder sb = new StringBuilder();
		Process p = Process.Start(processInfo);
		p.OutputDataReceived += (sender, args) => Console.WriteLine(args.Data);
		p.ErrorDataReceived += (sender, args) => Console.WriteLine(args.Data);
		p.BeginOutputReadLine();
		p.BeginErrorReadLine();
		p.WaitForExit();

		return p.ExitCode;
	}


	private static void DirectoryCopy(string sourceDirName, string destDirName, bool copySubDirs, bool overwrite)
	{
		// Get the subdirectories for the specified directory.
		DirectoryInfo dir = new DirectoryInfo(sourceDirName);

		if (!dir.Exists)
		{
			throw new DirectoryNotFoundException(
				"Source directory does not exist or could not be found: "
				+ sourceDirName);
		}

		DirectoryInfo[] dirs = dir.GetDirectories();

		// If the destination directory doesn't exist, create it.       
		Directory.CreateDirectory(destDirName);

		// Get the files in the directory and copy them to the new location.
		FileInfo[] files = dir.GetFiles();
		foreach (FileInfo file in files)
		{
			string tempPath = Path.Combine(destDirName, file.Name);
			file.CopyTo(tempPath, overwrite);
		}

		// If copying subdirectories, copy them and their contents to new location.
		if (copySubDirs)
		{
			foreach (DirectoryInfo subdir in dirs)
			{
				string tempPath = Path.Combine(destDirName, subdir.Name);
				DirectoryCopy(subdir.FullName, tempPath, copySubDirs, overwrite);
			}
		}
	}
}