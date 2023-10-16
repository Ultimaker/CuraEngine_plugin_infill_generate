# CuraEngine_plugin_infill_generate
This Engine plugin extends the current infill patterns in CURA with:

- Continuous Honeycomb
- Normal Honey Comb
- Cura
- Fill

NOTE: Please note that this plugin is Experimental and adding custom infills is not possible at the moment.

To build the target in DEBUG mode:

1. Configure Conan
   Before you start, if you use conan for other (big) projects as well, it's a good idea to either switch conan-home and/or backup your existing conan configuration(s).

That said, installing our config goes as follows:
'''
pip install conan==1.56
conan config install https://github.com/ultimaker/conan-config.git
conan profile new default --detect --force
'''


2. Clone CuraEngine
   git clone https://github.com/Ultimaker/CuraEngine.git
   cd CuraEngine
3. Install & Build CuraEngine (Release OR Debug)
   Release
   conan install . --build=missing --update
# optional for a specific version: conan install . curaengine/<version>@<user>/<channel> --build=missing --update
Skip these next two lines when on Windows, if you want to work in Visual Studio (as opposed to from the command line):

cmake --preset release
cmake --build --preset release