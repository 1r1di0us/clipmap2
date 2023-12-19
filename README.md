To Run:
1. Open the solution file in Visual Studio 2022.
2. Make sure you are in Win32 (not x64). You can be in Debug or Release.
3. Click either green arrow, and the code should run.

The current map and texture files were downloaded from https://www.motionforgepictures.com/height-maps/

To run the code using your own grayscale heightmap:
1. Download a greyscale heightmap in a format that you can open in an app such as photos, including .jpg, .png... etc
2. Open the image in photo app, and save a new copy of that file into the repository's Images folder. Make sure to select .bmp for the file type.
3. In Main.cpp, edit the strings in line 194 and 195 to be "../Images/FullNameOfYourFile.bmp". If you downloaded a texture as well as a heightmap,
   make sure that texture is also in bmp format and edit the string in line 195 to match the texture file.
4. Edit the global define VERTICALEXAGGERATION (defined at line 30) as you please to set the vertical scaling of your height map.
