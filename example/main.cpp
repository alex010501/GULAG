#include <GUI_Core.h>
#include <iostream>

int main(int argc, char** argv)
{
    CoreWindow testWindow("Computer Aided Robotics", "resources/Icons/RA_Icon.png");

    switch (testWindow.run())
    {
    case 0:
        std::cout << "Program finished correctly"<< std::endl;
        return 0;
        break;
    case 1:
        std::cout << "Error GLFW init"<< std::endl;
        return 1;
        break;
    case 2:
        std::cout << "Error GLFW window"<< std::endl;
        return 2;
        break;
    default:
        std::cout << "Error unknown"<< std::endl;
        return -1;
        break;
    }
}