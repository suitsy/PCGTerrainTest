# RTS Core Plugin

### Installation Instructions

### Dependent Plugins
- Nil

### Installation Steps C++

##### 1) Build.cs
- Add "RTSCore" entry to _ProjectName_.Build.cs PrivateDependencyModuleNames

##### 2) Install Plugins
1) CommonGame
2) CommonUI
3) CommonUser
4) ModularGameplayActors

##### 2) Create Framework Classes in Project
Either inherit from or use the RTS Core Framework classes
Dependent on what plugins from the RTS Framework you intend to use, either create these classes in your project 
or update your existing classes.
- GameInstance (Inherit from Framework\Managers\RTSCore_GameInstance)
- GameMode (Inherit from Framework\Managers\RTSCore_GameMode)
- GameState (Inherit from Framework\Managers\RTSCore_GameState) </br></br>
If using RTSEntities inherit from RTSEntities Player Controller, otherwise the RTS Core Player Controller
- PlayerController (Inherit from Framework\Player\RTSCore_PlayerController) </br></br>
If using RTSTeams inherit from the RTSTeams Player State, otherwise the RTS Core Player State
- PlayerState (Inherit from Framework\Player\RTSCore_PlayerState) </br></br>
If you using the RTSPlayer you will need to either inherit from the RTSPlayer_Pawn for your player pawn 
or use the B_RTSPlayer_Pawn blueprint class as your player pawn. 
- See respective plugins installation instructions for changes to framework classes.


### Installation Steps Editor
##### 3) Set Developer Setting - BoxSelectionWidget (Framework\UI\W_RTSC_BoxSelectWidget)
- Assign Box Selection Widget Class in Project Settings\RTS Core Settings


##### 4) AssetManager
- Add Asset Manager entries for data types
- Either manually in the Project Settings\AssetManager as below
  or
- Directly in the DefaultGame.ini under [/Script/Engine.AssetManagerSettings] <br>
- `+PrimaryAssetTypesToScan=(PrimaryAssetType="GameData",AssetBaseClass="/Script/RTSCore.RTSCore_GameDataAsset",bHasBlueprintClasses=False,bIsEditorOnly=False,Directories=((Path="/RTSCore/Data/Game")),SpecificAssets=,Rules=(Priority=-1,ChunkId=-1,bApplyRecursively=True,CookRule=AlwaysCook))`
- If [/Script/Engine.AssetManagerSettings] does not show in the DefaultGame.ini you can add it or edit the project settings to add it
