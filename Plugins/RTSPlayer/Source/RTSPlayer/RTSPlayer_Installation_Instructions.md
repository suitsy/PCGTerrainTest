# RTS Player Plugin
### Installation Instructions

### Dependent Plugins
- RTS Core

### Installation Steps C++
##### 1) Build.cs
- Add "RTSPlayer" entry to _ProjectName_.Build.cs PrivateDependencyModuleNames

### Installation Steps Editor
##### 2) Player Pawn
- If using the blueprint RTS Player Pawn (Framework/Player/B_RTSPlayer_Pawn) 
ensure it is set as the default pawn in your GameMode.
- If inheriting from the C++ RTSPlayer_Pawn class ensure the data input is assigned, the
input data setting is under "RTSPlayer Settings" in the blueprint of your Player Pawn. <br>
  (Input Data File: Framework/Input/Data/DA_RTSPlayer_InputData)

### Installation Steps Editor
##### 3) Custom Trace Channels
In Project Settings\Collision
- Add Custom Trace Channel "Terrain", Default: Ignore
- Add Custom Trace Channel "Selectable", Default: Ignore <br>
**or** <br>
Trace Channels can also be added directly to your /Config/DefaultEngine.ini 
- `+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Terrain")`
- `+DefaultChannelResponses=(Channel=ECC_GameTraceChannel2,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="Selectable")`

##### 4) Set Trace Channels
- Any terrain actors in your world (Landscape or LandscapeStreamingProxy) will need to have collision changed to "Custom" and the Terrain Channel set to Block
- Any selectable characters or objects will need to have their collision set to "Custom" and the Selectable channel set to block