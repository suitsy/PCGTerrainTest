# RTS Teams Plugin

### Installation Instructions

### Dependent Plugins
- RTS Core

### Installation Steps C++

##### 1) Build.cs
- Add "RTSTeams" entry to _ProjectName_.Build.cs PrivateDependencyModuleNames

##### 2) Game Mode
- Override Function GenericPlayerInitialization
- Add OnGameModePlayerInitialized Delegate
(Example: Framework\Managers\RTSTeams_GameMode)

##### 3) Game State
- Add URTSTeams_ManagerComponent to GameState
- Update references to PlayerState in URTSTeams_ManagerComponent (if not using RTSTeams_PlayerState)
(Example: Framework\Managers\RTSTeams_GameState)

##### 4) Player State
- Implement IRTSTeams_Agent in the PlayerState
- Implement Interface Functions
  (Example: Framework\Player\RTSTeams_PlayerState)

### Installation Steps Editor
- Nil