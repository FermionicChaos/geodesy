sudo apt install git
sudo apt install cmake
sudo apt install gcc g++
# Install X11 Dev Kit
sudo apt install xorg-dev
# Install OpenXR XCB support
sudo apt install libxcb-glx0-dev libxcb-xfixes0-dev
# Install Vulkan
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-noble.list https://packages.lunarg.com/vulkan/lunarg-vulkan-noble.list
sudo apt update
sudo apt install vulkan-sdk