# TestRenderer
Just helping a fellow on Stack Overflow with his Z axis question on the persepctive matrix in Vulkan. Turns out the camera was just going through the image while pointing at 0.0f, 0.0f, 0.0f where the image is and continuing to fly backwards along the positive z axis.

## Adding and Compiling the 3rdParty Libraries
- GLFW, GLM, stb, and Vulkan SDK will be needed.
- I have included the include files already in $(SolutionDir)TestRender\\3rdParty\\\<library>\\
- Each library can be found online either on GitHub or from the vendor's website. (VulkanSDK is an installer from their website.)
- In the `csproj` properties, configure the library directories to where ever your compiled binaries exist for each library.
- Download either the T-Shirt texture as a `.png` from the stack overflow post mentioned below in the [credits](#credits) or download/create/use your own `.png` texture and place it in the `csproj` directory.

## The Fix
I simply started the eyeZPos at -10.0f before increasing it's time value in the eyeZPos calculation like so:
```c++
float eyeZPos = (time * 1.0f) - 10.0f;
```

## Credits
- [John P Maus](https://stackoverflow.com/users/15268012/john-p-maus): For the Fix
- [Dess](https://stackoverflow.com/users/1229966/dess): For posting most of the code needed to reproduce this.