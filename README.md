# ImpostorBaker

Updated to UE 5.4, fixed port bugs (now works like original version).

## Changes
 - Simplified workflow, no separate level, pressing right click on static mesh and selecting `Create Impostor Data` will create new `ImpostorData` asset, which will hold all the impostor data and through which impostor/traditional billboards creation workflow goes. ImpostorData asset is not necessary later, can be removed (useful to rebake the impostor after updating mesh);
 - Removed necessity of ImpostorBakerSwitch material function for mesh materials;
 - Created separate action with its own editor for impostor baking, making it more straightforward;
 - Updated traditional billboards functionality;

Video (not actual, some bugs are fixed)

https://github.com/Erlandys/ImpostorBaker/assets/6483175/07991163-5938-453d-99e5-23408317c540


## Forked from
https://github.com/Erlandys/ImpostorBaker
https://github.com/ictusbrucks/ImpostorBaker (Original version)
