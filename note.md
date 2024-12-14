--
> Achieve Prosperity,
> Prosperity through Power,
> Power through Mastery
---
### Todo
---

### Grahics_D3D
- [x] Seperate viewport 
- [x] Seperate rasterizer
- [x] remove ndc
- [x] make sprite creation function that accepts shader/texture file path
- [x] make sprite default behavior (default texture if not found)
- [x] Support new shader resource
- [x] material that support generic properties assignment
- [x] Global constanst update each frame
- [x] memory management, like deleting unused materials and so on
- [x] BIG Refactor ownership of graphics data to GameObject/Resource
- [x] Font rendering for debugging
- [] shader multiple texture support
- [] (HARD DIFFICULTY) multiple render target support
- [] (HARD DIFFICULTY) multi-pass rendering


### Engine Core
- [x] CoreRenderer sends drawing data to Platform
- [x] Need to refactor and reorganize Graphics_D3D such that it supports instancing
- [x] Optimize MVP matrix such that we only need to change World matrix instead calculating all in CPU
- [x] Input keyboard print from engine core
- [x] GameObjects (sprite, materials, etc)
- [x] Optimize renderer to reuse buffers
- [x] Deprecate renderer draw/create resource function, and move it to GameResource/GameObject
- [x] GameObjects & GameResource functions implementation
- [x] Sprite naming
- [x] make automatic id generation, and how to draw it might related to scenegraph or use vector first
- [x] callback function for sprite update 
- [x] Renderer Scenegraph using vector
- [x] Proper UID for game resources
- [x] Vector math
- [x] Migrate object ids to std::string
- [x] Change Debugging log to a new Logger()
- [x] renaming renderer.h/cpp to scenegraph
- [x] Empty Game Object for grouping
- [x] Renderer Scenegraph using real graph
- [x] Matrix CoreMath 
- [x] Scenegraph transformations pass
- [x] Local Bounding Volume & Draw
- [x] Local Bounding Volume & Draw Tidyup/organize
- [x] Camera Object
- [x] Scenegraph update pass
- [x] Scenegraph draw pass Z index ordering
- [x] Scenegraph draw pass
- [x] World matrix owned by sprite object itself
- [x] Orthographic Camera Movement
- [x] seperate update batch from drawing batch
- [x] Level Serializations
- [x] Font SDF usage with maximum 72pt
- [x] Font Object in Scene Graph : the kind of text that is part of the game world
- [x] Font Object in GameLoader (Save and Load)
- [x] Font Resource save file
- [x] Animation preliminary (Animated Sprite)
- [x] Physics Preliminary (very rough setup structure)
- [] UI Billboard / UI Containers : a renderable container that is independent of scene graphs hierachy
- [] UI Containers specific draw pass
- [] Sound preliminary
- [] CoreInput remake
- [] Change all vbuffer to index buffer 
- [] Game World Space Metrics, Grids, Coordinates


### Engine Core New
- [x] Refactor Level save and load
- [x] Game code Parser 
- [x] Shader Reflection
- [] Optimize z-index sorting
- [] game object communication


### Engine Platform
- [x] Game Input
- [x] DLL loading
- [x] File IO
- [x] QueryPerformanceCounter
- [] Resize Window and viewport management
- [?] Memory Allocation Wrapper


### Resource Manager
- [] Open and read file function


### Code Organization
- [] utils folder should be part of core or platform (Core->Utils, Platform->Utils)
- [] Naming conventions for variables, files etc is a mess
- [x] better DebugLog()
- vim highlight like TOOD etc


## Engine Production Ideas, Improvement and Fixes
- Should we use IO using win32 or standard c lib ?
- Should we seperate struct declaration and impl seperation ?
- GameObject extension attribute 'state' at level file rename ?
- GameObject extension attribute 'parent' at gamescript rename ?
- CoreGlobals naming convention of 'collection' and '_collection' alternatives ?
- Improve free resources mechanism (should it delete the self or simply its atttribute only);
- Should we create custom data type ? (string, vectors, etc)
- We need to redraw our engine structure and file placements later
- Decide between struct inheritance or struct composition
- When dealing with relative path should we include '/' or not ?
- Should we still use XMFLOAT or simply CoreMath ?
- Remove all postfix '2D' such as 'Node2D' to 'Node'
- Use QuadTree instead of camera frustum based scene pruning
- Do no hesitate to use member function (instead of function pointer)
- Should we use iterator or access by index?

