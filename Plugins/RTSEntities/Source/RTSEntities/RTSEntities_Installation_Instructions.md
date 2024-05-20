# RTS Entities Plugin

### Installation Instructions

### Dependent Plugins
- RTS Core

### Installation Steps C++
##### 1) Build.cs
- Add "RTSEntities" entry to _ProjectName_.Build.cs PrivateDependencyModuleNames

##### 2) Game State
- Add URTSE_LoadoutManager Component to GameState <br>
(Example: Framework\Managers\RTSEntities_GameState)

##### 3) AssetManager
- Add Asset Manager entries for data types
- Either manually in the Project Settings\AssetManager as below
or
- Directly in the DefaultGame.ini under [/Script/Engine.AssetManagerSettings] <br>
- `+PrimaryAssetTypesToScan=(PrimaryAssetType="EntityData",AssetBaseClass="/Script/RTSEntities.RTSEntities_EntityDataAsset",bHasBlueprintClasses=False,bIsEditorOnly=False,Directories=((Path="/RTSEntities/Data")),SpecificAssets=,Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))`
- `+PrimaryAssetTypesToScan=(PrimaryAssetType="GroupData",AssetBaseClass="/Script/RTSEntities.RTSEntities_GroupDataAsset",bHasBlueprintClasses=False,bIsEditorOnly=False,Directories=((Path="/RTSEntities/Data/Groups")),SpecificAssets=,Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))`
- `+PrimaryAssetTypesToScan=(PrimaryAssetType="LoadoutData",AssetBaseClass="/Script/RTSEntities.RTSEntities_PlayerLoadoutDataAsset",bHasBlueprintClasses=False,bIsEditorOnly=False,Directories=((Path="/RTSEntities/Data/Loadouts")),SpecificAssets=,Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))`
- `+PrimaryAssetTypesToScan=(PrimaryAssetType="FormationData",AssetBaseClass="/Script/RTSEntities.RTSEntities_FormationDataAsset",bHasBlueprintClasses=False,bIsEditorOnly=False,Directories=((Path="/RTSEntities/Data/Formations")),SpecificAssets=,Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))` 
- If [/Script/Engine.AssetManagerSettings] does not show in the DefaultGame.ini you can add it or edit the project settings to add it


### Installation Steps Editor
##### 4) PlayerController
- Ensure your games player controller is using RTSEntities_PlayerController as its base class <br> 
(Framework\Player\RTSEntities_PlayerController)