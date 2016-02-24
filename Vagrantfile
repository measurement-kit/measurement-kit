Vagrant.configure(2) do |config|
  config.vm.define "vivid64" do |vivid64|
    vivid64.vm.box = "ubuntu/vivid64"
    vivid64.vm.provider "virtualbox" do |v|
      v.memory = 2048
    end
    vivid64.vm.provision "shell", inline: <<-SHELL
      sudo apt-get update
      sudo apt-get dist-upgrade -y
      sudo apt-get install -y g++ autoconf automake libtool valgrind git
      sudo apt-get install -y libevent-dev libssl-dev
      sudo apt-get install -y libboost-dev libyaml-cpp-dev
    SHELL
  end
end
