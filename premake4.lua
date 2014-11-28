solution "OpenGL"

    project "Castle"
        configurations { "debug", "release" }
            language "C++"
            kind "ConsoleApp"
            files "src/*.cpp"
            buildoptions { "-w -Wall -std=c++11" }
            linkoptions { "-fopenmp -lGL -lsfml-window -lsfml-system -lsfml-graphics -lGLEW `pkg-config --libs opencv`" }
            objdir "obj"

        configuration { "debug" }
            targetdir "debug"


        configuration { "release" }
            targetdir "release"
            flags "OptimizeSpeed"

        if _ACTION == "clean" then
            os.rmdir("debug")
            os.rmdir("release")
            os.rmdir("obj")
        end
