# ImpostorBaker

Master branch tested on 5.4

## Changes
 - Simplified workflow, no separate level, pressing right click on static mesh and selecting `Create Impostor Data` will create new `ImpostorData` asset, which will hold all the impostor data and through which impostor/traditional billboards creation workflow goes. ImpostorData asset is not necessary later, can be removed (useful to rebake the impostor after updating mesh);
 - Removed necessity of ImpostorBakerSwitch material function for mesh materials;
 - Created separate action with its own editor for impostor baking, making it more straightforward;
 - Updated traditional billboards functionality;

https://github.com/Erlandys/ImpostorBaker/assets/6483175/07991163-5938-453d-99e5-23408317c540


## Forked from
https://github.com/ictusbrucks/ImpostorBaker
