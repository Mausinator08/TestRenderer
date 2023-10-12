# TestRenderer
Just helping a fellow on Stack Overflow with his Z axis question on the persepctive matrix in Vulkan. Turns out the camera was just going through the image while pointing at 0.0f, 0.0f, 0.0f where the image is and continuing to fly backwards along the positive z axis.

## The Fix
I simply started the eyeZPos at -10.0f before increasing it's time value in the eyeZPos calculation like so:
```c++
float eyeZPos = (time * 1.0f) - 10.0f;
```

## Credits
- [John P Maus](https://stackoverflow.com/users/15268012/john-p-maus): For the Fix
- [Dess](https://stackoverflow.com/users/1229966/dess): For posting most of the code needed to reproduce this.