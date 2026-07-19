import os
import glob

files = glob.glob("Source/Core/Plugins/Stock/*.cpp") + glob.glob("Source/Core/Plugins/Stock/*.h")
for f in files:
    with open(f, 'r') as file:
        content = file.read()
    if "NimbusRotaryDial" in content:
        content = content.replace("NimbusRotaryDial", "PluginDial")
        with open(f, 'w') as file:
            file.write(content)
        print("Updated", f)
