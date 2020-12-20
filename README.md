# ClassTest
This is a proof of concept demonstrating how to add custom classes to Blockland. This can be extended to create custom NetEvents, the like.

**PLEASE NOTE: As this hooks into Blockland's internals heavily, IT WILL be needed to load before the game fully initializes.**

**To ensure that the hook works properly, you will need to add the DLL to the imports table so that it is loaded as soon as the executable starts.**

## Building
Open `CustomClasses.sln`, and build. 
