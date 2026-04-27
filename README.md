### INSTALLING:
```bush
    git clone https://github.com/Valdymarych/RelativisticElectrodynamics0.1.git
    cd RelativisticElectrodynamics0.1
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    cmake --build build
    !!!Replace $VCPKG_ROOT with your vcpkg path!!!
``` 

After building, the app (MyApp or MyApp.exe) will appear in the build folder.

Press K to open the Control Panel
Press P to pause


About Relativistic Electrodynamics.
This application visualizes the electric field, magnetic field, and Poynting vector for a system of relativistic charges. Each charge can either follow a predefined (hard-coded) trajectory or move freely under the local electromagnetic field. All heavy computations are performed on the GPU using compute shaders and SSBOs (Shader Storage Buffer Objects).