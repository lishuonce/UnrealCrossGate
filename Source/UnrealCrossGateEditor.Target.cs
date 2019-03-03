

using UnrealBuildTool;
using System.Collections.Generic;

public class UnrealCrossGateEditorTarget : TargetRules
{
	public UnrealCrossGateEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        bUseUnityBuild = false;
        bUsePCHFiles = false;

		ExtraModuleNames.AddRange( new string[] { "UnrealCrossGate" } );
	}
}
