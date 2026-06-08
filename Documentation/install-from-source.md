# Install RMF2 For Unreal from source

### Step 1: Add Plugin to Your Project

1. Navigate to your Unreal Engine project directory

1. Create a `Plugins` folder in your project root if it doesn't exist

1. Clone or download this repository inside the `Plugins` folder

   ```sh
   git clone https://github.com/ros-industrial/rmf2-unreal
   ```

   Your project structure should look like this:

   ```
   <YourProjectName>/
   ├── Content/
   ├── Source/
   ├── Plugins/
   │   └── RMF2ForUnreal/
   │       ├── RMF2ForUnreal.uplugin
   │       ├── Documentation/
   │       ├── Resources/
   │       ├── Source/
   │       └── ThirdParty/
   ├── Config/
   ├── <YourProjectName>.uproject
   └── ...
   ```

### Step 2: Enable the Plugin

> [!NOTE]
> Usually don't need to, but if somehow it is not enabled after cloning the repository, Do this.

1. Open your project in Unreal Engine Editor

1. Go to (Top Left) **Edit → Plugins**

1. Search for "RMF2"

  ![RMF2 For Unreal Plugin in UE5 Editor](Images/RMF2ForUnreal_Plugin.png)

1. Check the box next to **RMF2ForUnreal** to enable it

1. Click **Restart Now** when prompted (or just restart after enabling it)
